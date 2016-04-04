/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "command_line.hpp"
#include "chat_server.h"
#include "components/system_component.h"
#include "components/user_component.h"
#include "components/channel_component.h"
#include <iostream>
#include <chrono>
#include <thread>

// Program entrypoint
int main(int argc, char **argv) {
  std::cout << "jChatSystem - Server" << std::endl;

  jchat::CommandLine command_line(argc, argv);
  if (argc > 1) {
	  std::cout << "Starting with arguments..." << std::endl;
	  std::cout << command_line << std::endl;
  }

  jchat::ChatServer chat_server(
    command_line.GetString("ipaddress", "0.0.0.0").c_str(),
    command_line.GetInt32("port", 9998));

  auto system_component = std::make_shared<jchat::SystemComponent>();
  auto user_component = std::make_shared<jchat::UserComponent>();
  auto channel_component = std::make_shared<jchat::ChannelComponent>();

  chat_server.AddComponent(system_component);
  chat_server.AddComponent(user_component);
  chat_server.AddComponent(channel_component);

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
