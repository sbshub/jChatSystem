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

  jchat::TcpClient tcp_client("127.0.0.1", 9998, false);
  if (tcp_client.Connect()) {
    std::cout << "Connected to 127.0.0.1:9998" << std::endl;
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  } else {
    std::cout << "Failed to connect to 127.0.0.1:9998" << std::endl;
  }

  return 0;
}
