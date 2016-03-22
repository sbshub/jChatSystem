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
#include "tcp_client.hpp"

#ifndef JCHAT_TCP_SERVER_BACKLOG
#define JCHAT_TCP_SERVER_BACKLOG 50
#endif // JCHAT_TCP_SERVER_BACKLOG

namespace jchat {
class TcpServer {
  const char *hostname_;
  uint16_t port_;
  bool is_listening_;
  SOCKET listen_socket_;
  IPEndpoint listen_endpoint_;
  std::vector<TcpClient *> accepted_clients_;
  std::mutex accepted_clients_mutex_;
  std::thread worker_thread_;

#if defined(OS_WIN)
	WSADATA wsa_data_;
#endif

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
#if defined(OS_LINUX)
        uint32_t client_endpoint_size = sizeof(client_endpoint);
#elif defined(OS_WIN)
        int32_t client_endpoint_size = sizeof(client_endpoint);
#endif
        SOCKET client_socket = accept(listen_socket_,
          (sockaddr *)&client_endpoint, &client_endpoint_size);
        if (client_socket != SOCKET_ERROR) {
          TcpClient *tcp_client = new TcpClient(client_socket, client_endpoint,
            listen_endpoint_.GetSocketEndpoint());
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
            (char *)(*tcp_client)->read_buffer_.data(),
            (*tcp_client)->read_buffer_.size(), 0);
          bool disconnect_client = false;
          if (read_bytes > 0) {
            Buffer buffer((*tcp_client)->read_buffer_.data(), read_bytes);
            if (!OnDataReceived(**tcp_client, buffer)) {
              disconnect_client = true;
            }
          } else {
            disconnect_client = true;
          }
          if (disconnect_client) {
            (*tcp_client)->is_connected_ = false;
            closesocket((*tcp_client)->client_socket_);
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
    listen_socket_(0), listen_endpoint_("0.0.0.0", port) {
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
          listen_endpoint_.SetAddress(ntohl(endpoint_info->sin_addr.s_addr));
          break;
        }
      }
    }

     freeaddrinfo(result);
  }

  ~TcpServer() {
    if (is_listening_) {
      is_listening_ = false;
      worker_thread_.join();
      closesocket(listen_socket_);

#if defined(OS_WIN)
			// Cleanup Winsock
			WSACleanup();
#endif
    }

    if (!accepted_clients_.empty()) {
      for (auto tcp_client : accepted_clients_) {
        delete tcp_client;
      }
      accepted_clients_.clear();
    }
  }

  bool Start() {
    if (is_listening_) {
      return false;
    }

    if ((listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
      == SOCKET_ERROR) {
      return false;
    }

    sockaddr_in listen_endpoint = listen_endpoint_.GetSocketEndpoint();
    if (bind(listen_socket_, (const sockaddr *)&listen_endpoint,
      sizeof(listen_endpoint)) == SOCKET_ERROR) {
      closesocket(listen_socket_);
      return false;
    }

    if (listen(listen_socket_, JCHAT_TCP_SERVER_BACKLOG) == SOCKET_ERROR) {
      closesocket(listen_socket_);
      return false;
    }

#if defined(OS_LINUX)
		uint32_t flags = fcntl(listen_socket_, F_GETFL, 0);
		if (flags != SOCKET_ERROR) {
			flags |= O_NONBLOCK;
			if (fcntl(listen_socket_, F_SETFL, flags) == SOCKET_ERROR) {
				closesocket(listen_socket_);
				return false;
			}
		} else {
#elif defined(OS_WIN)
		unsigned int blocking = TRUE;
		if (ioctlsocket(listen_socket_, FIONBIO, &blocking) == SOCKET_ERROR) {
#endif
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
    if (!accepted_clients_.empty()) {
      for (auto tcp_client : accepted_clients_) {
        delete tcp_client;
      }
      accepted_clients_.clear();
    }
    accepted_clients_mutex_.unlock();

    return true;
  }

  bool DisconnectClient(TcpClient &tcp_client) {
    accepted_clients_mutex_.lock();
    for (auto client = accepted_clients_.begin();
      client != accepted_clients_.end();) {
      if (*client == &tcp_client) {
        (*client)->is_connected_ = false;
        closesocket((*client)->client_socket_);
        OnClientDisconnected(**client);
        accepted_clients_.erase(client);
        accepted_clients_mutex_.unlock();
        return true;
      } else {
        ++client;
      }
    }
    accepted_clients_mutex_.unlock();
    return false;
  }

  bool Send(TcpClient &tcp_client, Buffer &buffer) {
    if (!tcp_client.is_internal_ || !tcp_client.is_connected_) {
      return false;
    }

    return send(tcp_client.client_socket_, (const char *)buffer.GetBuffer(),
      buffer.GetSize(), 0) != SOCKET_ERROR;
  }

  IPEndpoint GetListenEndpoint() {
    return listen_endpoint_;
  }

  Event<TcpClient &> OnClientConnected;
  Event<TcpClient &> OnClientDisconnected;
  Event<TcpClient &, Buffer &> OnDataReceived;
};
}

#endif // jchat_lib_tcp_server_hpp_
