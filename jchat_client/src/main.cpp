/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "tcp_client.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Program entrypoint
int main(int argc, char **argv) {
  std::cout << "jChatSystem - Client" << std::endl;

  jchat::TcpClient tcp_client("127.0.0.1", 9998);
  tcp_client.OnDisconnected.Add([]() {
    std::cout << "Disconnected from server" << std::endl;
    exit(0);
    return true;
  });
  if (tcp_client.Connect()) {
    std::cout << "Connected to "
              << tcp_client.GetRemoteEndpoint().ToString()
              << std::endl;
    while (true) {
      std::string message;
      std::getline(std::cin, message);

      jchat::Buffer buffer;
      buffer.WriteArray<char>(const_cast<char *>(message.c_str()), message.size());
      buffer.Write<char>(0);

      tcp_client.Send(buffer);

      std::cout << ">> " << message << std::endl;
    }
  } else {
    std::cout << "Failed to connect to "
              << tcp_client.GetRemoteEndpoint().ToString()
              << std::endl;
  }

  return 0;
}
