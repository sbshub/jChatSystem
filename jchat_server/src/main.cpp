/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "chat_server.h"
#include "components/system_component.h"
#include <iostream>
#include <chrono>
#include <thread>

// Program entrypoint
int main(int argc, char **argv) {
  std::cout << "jChatSystem - Server" << std::endl;

  jchat::ChatServer chat_server("0.0.0.0", 9998);
  jchat::SystemComponent system_component;
  chat_server.AddComponent(&system_component);
  chat_server.OnClientConnected.Add([](jchat::RemoteChatClient &client) {
    std::cout << "Client from "
              << client.Endpoint.ToString()
              << " connected"
              << std::endl;
    return true;
  });
  chat_server.OnClientDisconnected.Add([](jchat::RemoteChatClient &client) {
    std::cout << "Client from "
              << client.Endpoint.ToString()
              << " disconnected"
              << std::endl;
    return true;
  });
  if (chat_server.Start()) {
    std::cout << "Started listening on "
              << chat_server.GetListenEndpoint().ToString()
              << std::endl;
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  } else {
    std::cout << "Failed to listen on "
              << chat_server.GetListenEndpoint().ToString()
              << std::endl;
    return -1;
  }

  return 0;
}
