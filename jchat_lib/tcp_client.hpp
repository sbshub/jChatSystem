/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_lib_tcp_client_hpp_
#define jchat_lib_tcp_client_hpp_

// Required libraries
#include "platform.h"
#include "event.hpp"
#include "buffer.hpp"
#include "ip_endpoint.hpp"
#include <chrono>
#include <thread>
#if defined(OS_LINUX)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifndef __SOCKET__
#define __SOCKET__
typedef int SOCKET;
#endif // __SOCKET__

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif // SOCKET_ERROR

#ifndef __CLOSE_SOCKET__
#define __CLOSE_SOCKET__
#define closesocket(socket_fd) close(socket_fd)
#endif // __CLOSE_SOCKET__

#elif defined(OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>

// Platform/Compiler patches
#if defined(__CYGWIN__) || defined(__MINGW32__)
#if defined(FIONBIO)
#undef FIONBIO
#define FIONBIO 0x8004667E
#endif
#endif
#endif

#ifndef JCHAT_TCP_CLIENT_BUFFER_SIZE
#define JCHAT_TCP_CLIENT_BUFFER_SIZE 8192
#endif // JCHAT_TCP_CLIENT_BUFFER_SIZE

namespace jchat {
class TcpServer;
class TcpClient {
  friend class TcpServer;

  const char *hostname_;
  uint16_t port_;
  bool is_connected_;
  bool is_internal_;
  SOCKET client_socket_;
  IPEndpoint client_endpoint_;
  IPEndpoint remote_endpoint_;
  std::thread worker_thread_;
  std::vector<uint8_t> read_buffer_;

#if defined(OS_WIN)
	WSADATA wsa_data_;
#endif

  void worker_loop() {
    fd_set socket_set;
    while (is_connected_) {
      // Clear the socket set
      FD_ZERO(&socket_set);

      // Add the client to the set
      FD_SET(client_socket_, &socket_set);

      // Check if an activity was completed on any of those sockets
      int32_t socket_activity = select(client_socket_ + 1, &socket_set, NULL,
        NULL, NULL);

      // Ensure select didn't fail
      if (socket_activity == SOCKET_ERROR && errno == EINTR) {
        continue;
      }

      // Check if there was some operation completed on the client socket
      if (FD_ISSET(client_socket_, &socket_set)) {
        uint32_t read_bytes = recv(client_socket_, (char *)read_buffer_.data(),
          read_buffer_.size(), 0);
        bool disconnect_client = false;
        if (read_bytes > 0) {
          Buffer buffer(read_buffer_.data(), read_bytes);
          if (!OnDataReceived(buffer)) {
            disconnect_client = true;
          }
        } else {
          disconnect_client = true;
        }
        if (disconnect_client) {
          is_connected_ = false;
          closesocket(client_socket_);
          OnDisconnected();
        }
      }

      // Sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

public:
  TcpClient(const char *hostname, uint16_t port)
    : hostname_(hostname), port_(port), client_socket_(0),
    remote_endpoint_(hostname, port), is_connected_(false),
    is_internal_(false) {
    read_buffer_.resize(JCHAT_TCP_CLIENT_BUFFER_SIZE);

#if defined(OS_WIN)
		// Initialize Winsock
		WSAStartup(MAKEWORD(2, 2), &wsa_data_);
#endif

    // Get remote address info
    addrinfo *result = nullptr;
    addrinfo hints;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int return_value = getaddrinfo(hostname, std::to_string(port).c_str(),
      &hints, &result);
    if (return_value != SOCKET_ERROR) {
      for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET) {
          sockaddr_in *endpoint_info = (sockaddr_in *)ptr->ai_addr;
          remote_endpoint_.SetAddress(ntohl(endpoint_info->sin_addr.s_addr));
          break;
        }
      }
    }

     freeaddrinfo(result);
  }

  // NOTE: For internal usage only!
  TcpClient(SOCKET client_socket, sockaddr_in client_endpoint,
    sockaddr_in server_endpoint) : client_socket_(client_socket),
    client_endpoint_(client_endpoint), remote_endpoint_(server_endpoint),
    is_connected_(true), is_internal_(true) {
    read_buffer_.resize(JCHAT_TCP_CLIENT_BUFFER_SIZE);

#if defined(OS_LINUX)
		uint32_t flags = fcntl(client_socket, F_GETFL, 0);
		if (flags != SOCKET_ERROR) {
			flags |= O_NONBLOCK;
			fcntl(client_socket, F_SETFL, flags);
		}
#elif defined(OS_WIN)
		unsigned int blocking = 1;
		ioctlsocket(client_socket, FIONBIO, &blocking);
#endif
  }

  ~TcpClient() {
    if (is_connected_) {
      is_connected_ = false;
      if (!is_internal_) {
        worker_thread_.join();
      }
      closesocket(client_socket_);
#if defined(OS_WIN)
			// Cleanup Winsock
			WSACleanup();
#endif
    }
  }

  bool Connect() {
    if (is_connected_ || is_internal_) {
      return false;
    }

    if ((client_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
      == SOCKET_ERROR) {
      return false;
    }

    sockaddr_in remote_endpoint = remote_endpoint_.GetSocketEndpoint();
    if (connect(client_socket_, (const sockaddr *)&remote_endpoint,
      sizeof(remote_endpoint)) == SOCKET_ERROR) {
      closesocket(client_socket_);
      return false;
    }

    sockaddr_in client_endpoint;
    socklen_t client_endpoint_size = sizeof(client_endpoint);
    if (getsockname(client_socket_, (sockaddr *)&client_endpoint,
      &client_endpoint_size) == SOCKET_ERROR) {
      closesocket(client_socket_);
      return false;
    }
    client_endpoint_.SetSocketEndpoint(client_endpoint);

#if defined(OS_LINUX)
		uint32_t flags = fcntl(client_socket_, F_GETFL, 0);
		if (flags != SOCKET_ERROR) {
			flags |= O_NONBLOCK;
			if (fcntl(client_socket_, F_SETFL, flags) == SOCKET_ERROR) {
				closesocket(client_socket_);
				return false;
			}
		} else {
#elif defined(OS_WIN)
		unsigned int blocking = 1;
		if (ioctlsocket(client_socket_, FIONBIO, &blocking) == SOCKET_ERROR) {
#endif
      closesocket(client_socket_);
      return false;
    }

    worker_thread_ = std::thread(&TcpClient::worker_loop, this);

    is_connected_ = true;
    OnConnected();

    return true;
  }

  bool Disconnect() {
    if (!is_connected_ || is_internal_) {
      return false;
    }

    is_connected_ = false;
    worker_thread_.join();
    closesocket(client_socket_);

    OnDisconnected();

    return true;
  }

  bool Send(Buffer &buffer) {
    if (is_internal_ || !is_connected_) {
      return false;
    }

    return send(client_socket_, (const char *)buffer.GetBuffer(),
      buffer.GetSize(), 0) != SOCKET_ERROR;
  }

  IPEndpoint GetLocalEndpoint() {
    if (is_internal_) {
      return remote_endpoint_;
    } else {
      return client_endpoint_;
    }
  }

  IPEndpoint GetRemoteEndpoint() {
    if (is_internal_) {
      return client_endpoint_;
    } else {
      return remote_endpoint_;
    }
  }

  // NOTE: These are not intended to be used in combination with TcpServer
  Event<> OnConnected;
  Event<> OnDisconnected;
  Event<Buffer &> OnDataReceived;
};
}

#endif // jchat_lib_tcp_client_hpp_
