/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_lib_tcp_server_hpp_
#define jchat_lib_tcp_server_hpp_

// Required libraries
#include "platform.h"
#include "event.hpp"
#include "tcp_client.hpp"
#include "buffer.hpp"
#include <mutex>
#include <thread>
#if defined(OS_LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

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
#include <Windows.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
#endif

#ifndef JCHAT_TCP_SERVER_BACKLOG
#define JCHAT_TCP_SERVER_BACKLOG 50
#endif // JCHAT_TCP_SERVER_BACKLOG

namespace jchat {
class TcpServer {
  const char *hostname_;
  uint16_t port_;
  bool is_blocking_;
  bool is_listening_;
  SOCKET listen_socket_;
#if defined(OS_WIN)
  WSADATA wsa_data_;
#endif
  sockaddr_in listen_endpoint_;
  std::vector<TcpClient *> accepted_clients_;
  std::mutex accepted_clients_mutex_;
  std::thread worker_thread_;

  void worker_loop() {
    while (is_listening_) {
      sockaddr_in client_endpoint;
      uint32_t client_endpoint_size = sizeof(client_endpoint);
      // TODO/NOTE/FIXME: This is actually blocking, which needs fixing...
      SOCKET client_socket = accept(listen_socket_,
        (sockaddr *)&client_endpoint, &client_endpoint_size);
      if (client_socket != SOCKET_ERROR) {
        TcpClient tcp_client(client_socket, client_endpoint);
        OnClientConnected(tcp_client);
      }
    }
  }

public:
  TcpServer(const char *hostname, uint16_t port, bool is_blocking = true)
    : hostname_(hostname), port_(port), is_blocking_(is_blocking),
    is_listening_(false), listen_socket_(0) {
    listen_endpoint_.sin_family = AF_INET;
    listen_endpoint_.sin_port = htons(port);
#if defined(OS_LINUX)
    listen_endpoint_.sin_addr.s_addr = inet_addr("0.0.0.0");
#elif defined(OS_WIN) // Currently untested
    listen_endpoint_.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
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
#if defined(OS_LINUX)
					listen_endpoint_.sin_addr.s_addr = endpoint_info->sin_addr.s_addr;
#elif defined(OS_WIN) // Currently untested
					listen_endpoint_.sin_addr.S_un.S_addr = endpoint_info->sin_addr.S_un.S_addr;
#endif
        }
      }
    }
  }

  ~TcpServer() {
    if (is_listening_) {
      closesocket(listen_socket_);
#if defined(OS_WIN)
      WSACleanup();
#endif
    }
  }

  bool Start() {
    if (is_listening_) {
      return false;
    }

#if defined(OS_WIN)
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
      return false;
    }
#endif

    if ((listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
      == SOCKET_ERROR) {
      return false;
    }

    if (bind(listen_socket_, (const sockaddr *)&listen_endpoint_,
      sizeof(listen_endpoint_)) == SOCKET_ERROR) {
      closesocket(listen_socket_);
      return false;
    }

    if (listen(listen_socket_, JCHAT_TCP_SERVER_BACKLOG) == SOCKET_ERROR) {
      closesocket(listen_socket_);
      return false;
    }

    if (!is_blocking_) {
#if defined(OS_LINUX)
      uint32_t flags = fcntl(listen_socket_, F_GETFL, 0);
      if (flags != SOCKET_ERROR) {
        flags &= ~O_NONBLOCK;
        if (fcntl(listen_socket_, F_SETFL, flags) == SOCKET_ERROR) {
          closesocket(listen_socket_);
          return false;
        }
      } else {
        closesocket(listen_socket_);
        return false;
      }
#elif defined(OS_WIN)
      uint32_t blocking = is_blocking ? 0 : 1;
      if (ioctlsocket(listen_socket_, FIONBIO, &blocking) == SOCKET_ERROR) {
        closesocket(listen_socket_);
        return false;
      }
#endif
    }

    is_listening_ = true;

    worker_thread_ = std::thread(&TcpServer::worker_loop, this);

    return true;
  }

  bool Stop() {
    if (!is_listening_) {
      return false;
    }

    is_listening_ = false;
    worker_thread_.join();
    closesocket(listen_socket_);

#if defined(OS_WIN)
    WSACleanup();
#endif

    return true;
  }

  Event<TcpClient &> OnClientConnected;
  Event<TcpClient &> OnClientDisconnected;
  Event<TcpClient &, Buffer &> OnDataReceived;
};
}

#endif // jchat_lib_tcp_server_hpp_
