/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "core/chat_client.h"
#include "string.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Program entrypoint
int main(int argc, char **argv) {
  std::cout << "jChatSystem - Client" << std::endl;

  jchat::ChatClient chat_client("127.0.0.1", 9998);
  chat_client.OnDisconnected.Add([]() {
    std::cout << "Disconnected from server" << std::endl;
    exit(0);
    return true;
  });
  if (chat_client.Connect()) {
    std::cout << "Connected to "
              << chat_client.GetRemoteEndpoint().ToString()
              << std::endl;
    while (true) {
      std::string input;
      std::getline(std::cin, input);

      // Check if the input is valid
      if (input.empty()) {
        continue;
      }

      if (input[0] != '/') {
        std::cout << "Invalid command" << std::endl;
        continue;
      }

      // Split the input
      std::vector<std::string> input_split
        = jchat::String::Split(input.substr(1), " ");

      // Read the command
      if (input_split.size() == 0) {
        std::cout << "Invalid command" << std::endl;
        continue;
      }

      std::string &command = input_split[0];
      std::vector<std::string> arguments(input_split.begin() + 1,
        input_split.end());

      if (command == "identify" && arguments.size() == 1) {
        std::string &username = arguments[0];
        // TODO: Implement
      } else if (command == "join" && arguments.size() == 1) {
        std::string &target = arguments[0];
        // TODO: Implement
      } else if (command == "msg" && arguments.size() >= 2) {
        std::string &target = arguments[0];
        std::string message = jchat::String::Join(
          std::vector<std::string>(arguments.begin() + 1, arguments.end()),
          " ");
        // TODO: Implement
      } else {
        std::cout << "Invalid command" << std::endl;
        continue;
      }

      std::cout << ">> " << input << std::endl;
    }
  } else {
    std::cout << "Failed to connect to "
              << chat_client.GetRemoteEndpoint().ToString()
              << std::endl;
  }

  return 0;
}
