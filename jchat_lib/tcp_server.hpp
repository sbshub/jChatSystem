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
#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
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

#ifndef JCHAT_TCP_SERVER_BACKLOG
#define JCHAT_TCP_SERVER_BACKLOG 50
#endif // JCHAT_TCP_SERVER_BACKLOG

namespace jchat {
class TcpServer {
  const char *hostname_;
  uint16_t port_;
  bool is_listening_;
  SOCKET listen_socket_;
  sockaddr_in listen_endpoint_;
  std::vector<TcpClient *> accepted_clients_;
  std::mutex accepted_clients_mutex_;
  std::thread worker_thread_;

  void worker_loop() {
    fd_set socket_set;
    SOCKET max_socket = 0;
    while (is_listening_) {
      // Clear the socket set
      FD_ZERO(&socket_set);

      // Add the listener to the set
      FD_SET(listen_socket_, &socket_set);
      max_socket = listen_socket_;

      // Add all clients to the set
      accepted_clients_mutex_.lock();
      for (auto tcp_client : accepted_clients_) {
        if (tcp_client->is_connected_) {
          FD_SET(tcp_client->client_socket_, &socket_set);

          // If the client socket is the largest socket, set it so
          if (tcp_client->client_socket_ > max_socket) {
            max_socket = tcp_client->client_socket_;
          }
        }
      }
      accepted_clients_mutex_.unlock();

      // Check if an activity was completed on any of those sockets
      int32_t socket_activity = select(max_socket + 1, &socket_set, NULL, NULL,
        NULL);

      // Ensure select didn't fail
      if (socket_activity == SOCKET_ERROR && errno == EINTR) {
        continue;
      }

      // Check if a new connection is awaiting
      if (FD_ISSET(listen_socket_, &socket_set)) {
        sockaddr_in client_endpoint;
        uint32_t client_endpoint_size = sizeof(client_endpoint);
        SOCKET client_socket = accept(listen_socket_,
          (sockaddr *)&client_endpoint, &client_endpoint_size);
        if (client_socket != SOCKET_ERROR) {
          TcpClient *tcp_client = new TcpClient(client_socket, client_endpoint);
          accepted_clients_mutex_.lock();
          accepted_clients_.push_back(tcp_client);
          accepted_clients_mutex_.unlock();

          OnClientConnected(*tcp_client);
        }
      }

      // Check if there was some operation completed on another socket
      accepted_clients_mutex_.lock();
      for (auto tcp_client = accepted_clients_.begin();
        tcp_client != accepted_clients_.end();) {
        if (FD_ISSET((*tcp_client)->client_socket_, &socket_set)) {
          uint32_t read_bytes = recv((*tcp_client)->client_socket_,
            (*tcp_client)->read_buffer_.data(),
            (*tcp_client)->read_buffer_.size(), 0);
          if (read_bytes > 0) {
            Buffer buffer((*tcp_client)->read_buffer_.data(), read_bytes);
            OnDataReceived(**tcp_client, buffer);
          } else {
            (*tcp_client)->is_connected_ = false;
            OnClientDisconnected(**tcp_client);
            tcp_client = accepted_clients_.erase(tcp_client);
            continue;
          }
        }
        ++tcp_client;
      }
      accepted_clients_mutex_.unlock();

      // Sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

public:
  TcpServer(const char *hostname, uint16_t port)
    : hostname_(hostname), port_(port), is_listening_(false),
    listen_socket_(0) {
    listen_endpoint_.sin_family = AF_INET;
    listen_endpoint_.sin_port = htons(port);
    listen_endpoint_.sin_addr.s_addr = inet_addr("0.0.0.0");

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
          listen_endpoint_.sin_addr.s_addr = endpoint_info->sin_addr.s_addr;
        }
      }
    }

     freeaddrinfo(result);
  }

  ~TcpServer() {
    if (is_listening_) {
      closesocket(listen_socket_);
    }
    for (auto tcp_client : accepted_clients_) {
      delete tcp_client;
    }
    accepted_clients_.clear();
  }

  bool Start() {
    if (is_listening_) {
      return false;
    }

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

    uint32_t flags = fcntl(listen_socket_, F_GETFL, 0);
    if (flags != SOCKET_ERROR) {
      flags |= O_NONBLOCK;
      if (fcntl(listen_socket_, F_SETFL, flags) == SOCKET_ERROR) {
        closesocket(listen_socket_);
        return false;
      }
    } else {
      closesocket(listen_socket_);
      return false;
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

    accepted_clients_mutex_.lock();
    for (auto tcp_client : accepted_clients_) {
      delete tcp_client;
    }
    accepted_clients_.clear();
    accepted_clients_mutex_.unlock();

    return true;
  }

  bool Send(TcpClient &tcp_client, Buffer &buffer) {
    if (!tcp_client.is_internal_ || !tcp_client.is_connected_) {
      return false;
    }

    return send(tcp_client.client_socket_, buffer.GetBuffer(),
      buffer.GetSize(), 0) != SOCKET_ERROR;
  }

  Event<TcpClient &> OnClientConnected;
  Event<TcpClient &> OnClientDisconnected;
  Event<TcpClient &, Buffer &> OnDataReceived;
};
}

#endif // jchat_lib_tcp_server_hpp_
