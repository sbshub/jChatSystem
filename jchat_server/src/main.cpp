/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "tcp_server.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Program entrypoint
int main(int argc, char **argv) {
  std::cout << "jChatSystem - Server" << std::endl;

  jchat::TcpServer tcp_server("0.0.0.0", 9998, false);
  tcp_server.OnClientConnected.Add([](jchat::TcpClient &tcp_client) {
    std::cout << "Client connected!" << std::endl;
    return true;
  });
  if (tcp_server.Start()) {
    std::cout << "Started listening on 0.0.0.0:9998" << std::endl;
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  } else {
    std::cout << "Failed to listen on 0.0.0.0:9998" << std::endl;
  }

  return 0;
}
