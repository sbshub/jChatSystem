/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*
*/

// Required libraries
#include "tcp_server.hpp"
#include <iostream>

// Program entrypoint
int main(int argc, char **argv) {
  std::cout << "jChatSystem - Server" << std::endl;

  jchat::TcpServer tcp_server(9998);

  return 0;
}
