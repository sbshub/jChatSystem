/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*
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
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

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
#else
#error "Unsupported platform!"
#endif

#ifndef JCHAT_TCP_SERVER_BACKLOG
#define JCHAT_TCP_SERVER_BACKLOG 50
#endif // JCHAT_TCP_SERVER_BACKLOG

namespace jchat {
class TcpServer {
  uint16_t port_;
  bool is_listening_;
  SOCKET listen_socket_;
#if defined(OS_WIN)
  WSADATA wsa_data_;
#endif
  sockaddr_in listen_endpoint_;
  std::vector<TcpClient> accepted_clients_;
  std::mutex accepted_clients_mutex_;
  std::thread worker_thread_;

  void worker_loop() {
    // TODO: Write
  }

public:
  TcpServer(uint16_t port) : port_(port), is_listening_(false),
    listen_socket_(0) {
    listen_endpoint_.sin_family = AF_INET;
    listen_endpoint_.sin_port = port;
#if defined(OS_LINUX)
    listen_endpoint_.sin_addr.s_addr = inet_aton("0.0.0.0");
#elif defined(OS_WIN) // Currently unsupported and untested
    listen_endpoint_.sin_addr.S_addr = inet_aton("0.0.0.0");
#endif
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

    if (bind(listen_socket_, &listen_endpoint_, sizeof(listen_endpoint_))
      == SOCKET_ERROR) {
      closesocket(listen_socket_);
      return false;
    }

    if (listen(listen_socket_, JCHAT_TCP_SERVER_BACKLOG) == SOCKET_ERROR) {
      closesocket(listen_socket_);
      return false;
    }

    is_listening_ = true;

    worker_thread_ = std::thread(worker_loop);

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

  Event<TcpClient> OnClientConnected;
  Event<TcpClient> OnClientDisconnected;
  Event<TcpClient, Buffer> OnDataReceived;
};
}

#endif // jchat_lib_tcp_server_hpp_
