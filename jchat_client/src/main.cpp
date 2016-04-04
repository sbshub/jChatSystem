/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "command_line.hpp"
#include "chat_client.h"
#include "components/system_component.h"
#include "components/user_component.h"
#include "components/channel_component.h"
#include "string.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Program entrypoint
int main(int argc, char **argv) {
  std::cout << "jChatSystem - Client" << std::endl;

  jchat::CommandLine command_line(argc, argv);
  if (argc > 1) {
	  std::cout << "Starting with arguments..." << std::endl;
	  std::cout << command_line << std::endl;
  }

  // Create the chat client
  jchat::ChatClient chat_client(
    command_line.GetString("ipaddress", "127.0.0.1").c_str(),
    command_line.GetInt32("port", 9998));

  // Handle client events
  chat_client.OnDisconnected.Add([]() {
    std::cout << "Disconnected from server" << std::endl;
    exit(0);
    return true;
  });

  // Create the chat components
  auto system_component = std::make_shared<jchat::SystemComponent>();
  auto user_component = std::make_shared<jchat::UserComponent>();
  auto channel_component = std::make_shared<jchat::ChannelComponent>();

  // Handle any API events
  system_component->OnHelloCompleted.Add([](jchat::SystemMessageResult result) {
    if (result == jchat::kSystemMessageResult_Ok) {
      std::cout << "System: Hello succeeded" << std::endl;
    } else if (result == jchat::kSystemMessageResult_InvalidProtocolVersion) {
      std::cout << "System: Invalid protocol version!" << std::endl;
      exit(0);
    }
    return true;
  });
  user_component->OnIdentifyCompleted.Add([](jchat::UserMessageResult result,
	  std::string &username) {
    if (result == jchat::kUserMessageResult_Ok) {
      std::cout << "User: Successfully identified! (" << username << ")"
        << std::endl;
    } else if (result == jchat::kUserMessageResult_InvalidUsername) {
      std::cout << "User: Invalid username! (" << username << ")" << std::endl;
    } else if (result == jchat::kUserMessageResult_UsernameInUse) {
      std::cout << "User: Username in use! (" << username << ")" << std::endl;
    } else if (result == jchat::kUserMessageResult_UsernameTooLong) {
      std::cout << "User: Username too long! (" << username << ")" << std::endl;
    } else if (result == jchat::kUserMessageResult_AlreadyIdentified) {
      std::cout << "User: Already identified! (" << username << ")"
        << std::endl;
    }
    return true;
  });
  user_component->OnSendMessageCompleted.Add([](jchat::UserMessageResult result,
	  std::string &username, std::string &message) {
    if (result == jchat::kUserMessageResult_InvalidUsername) {
      std::cout << "User: Invalid username! (" << username  << ", \""
        << message << "\")" << std::endl;
    } else if (result == jchat::kUserMessageResult_NotIdentified) {
      std::cout << "User: Not identified! (" << username  << ", \""
        << message << "\")" << std::endl;
    } else if (result == jchat::kUserMessageResult_UserNotIdentified) {
      std::cout << "User: User not identified! (" << username  << ", \""
        << message << "\")" << std::endl;
    } else if (result == jchat::kUserMessageResult_InvalidMessage) {
      std::cout << "User: Invalid message! (" << username  << ", \""
        << message << "\")" << std::endl;
    } else if (result == jchat::kUserMessageResult_MessageTooLong) {
      std::cout << "User: Message too long! (" << username  << ", \""
        << message << "\")" << std::endl;
    } else if (result == jchat::kUserMessageResult_CannotMessageSelf) {
      std::cout << "User: Cannot message self! (" << username  << ", \""
        << message << "\")" << std::endl;
    }
    return true;
  });
  user_component->OnMessage.Add([=](std::string &source_username,
    std::string &source_hostname, std::string &target, std::string &message) {
    std::shared_ptr<jchat::ChatUser> user;
    if (!user_component->GetChatUser(user)) {
      return false;
    }

    if (user->Username != source_username) {
      std::cout << "User: " << source_username << " => " << target << ": "
        << message << std::endl;
    }

    return true;
  });
  channel_component->OnJoinCompleted.Add([](jchat::ChannelMessageResult result,
    std::string &channel_name) {
    if (result == jchat::kChannelMessageResult_Ok) {
      std::cout << "Channel: Successfully joined channel! (" << channel_name
        << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_ChannelCreated) {
      std::cout << "Channel: Successfully created channel! (" << channel_name
        << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_AlreadyInChannel) {
      std::cout << "Channel: Already in channel! (" << channel_name << ")"
        << std::endl;
    } else if (result == jchat::kChannelMessageResult_BannedFromChannel) {
      std::cout << "Channel: Banned from channel! (" << channel_name << ")"
        << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotIdentified) {
      std::cout << "Channel: Not identified! (" << channel_name << ")"
        << std::endl;
    } else if (result == jchat::kChannelMessageResult_InvalidChannelName) {
      std::cout << "Channel: Invalid channel name! (" << channel_name << ")"
        << std::endl;
    }
    return true;
  });
  channel_component->OnLeaveCompleted.Add([](jchat::ChannelMessageResult result,
    std::string &channel_name) {
    if (result == jchat::kChannelMessageResult_Ok) {
      std::cout << "Channel: Successfully left channel! (" << channel_name
        << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_ChannelDestroyed) {
      std::cout << "Channel: Successfully destroyed channel! (" << channel_name
        << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotInChannel) {
      std::cout << "Channel: Not in channel! (" << channel_name << ")"
        << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotIdentified) {
      std::cout << "Channel: Not identified! (" << channel_name << ")"
        << std::endl;
    } else if (result == jchat::kChannelMessageResult_InvalidChannelName) {
      std::cout << "Channel: Invalid channel name! (" << channel_name << ")"
        << std::endl;
    }
    return true;
  });
  channel_component->OnSendMessageCompleted.Add([](
    jchat::ChannelMessageResult result, std::string &channel_name,
    std::string &message) {
    if (result == jchat::kChannelMessageResult_InvalidMessage) {
      std::cout << "Channel: Invalid message! (" << channel_name << ", \""
      << message << "\")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_MessageTooLong) {
      std::cout << "Channel: Message too long! (" << channel_name << ", \""
      << message << "\")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotInChannel) {
      std::cout << "Channel: Not in channel! (" << channel_name << ", \""
      << message << "\")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotIdentified) {
      std::cout << "Channel: Not identified! (" << channel_name << ", \""
      << message << "\")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_InvalidChannelName) {
      std::cout << "Channel: Invalid channel name! (" << channel_name << ", \""
      << message << "\")" << std::endl;
    }
    return true;
  });
  channel_component->OnOpUserCompleted.Add([](
    jchat::ChannelMessageResult result, std::string &channel_name,
    std::string &username) {
    if (result == jchat::kChannelMessageResult_Ok) {
      std::cout << "Channel: Successfully opped user! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotPermitted) {
      std::cout << "Channel: Not permitted! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_AlreadyOperator) {
      std::cout << "Channel: Already operator! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_CannotOpSelf) {
      std::cout << "Channel: Cannot op self! (" << channel_name
        << ", " << username << ")" << std::endl;
    }else if (result == jchat::kChannelMessageResult_NotInChannel) {
      std::cout << "Channel: Not in channel! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotIdentified) {
      std::cout << "Channel: Not identified! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_InvalidChannelName) {
      std::cout << "Channel: Invalid channel name! (" << channel_name
        << ", " << username << ")" << std::endl;
    }
    return true;
  });
  channel_component->OnDeopUserCompleted.Add([](
    jchat::ChannelMessageResult result, std::string &channel_name,
    std::string &username) {
    if (result == jchat::kChannelMessageResult_Ok) {
      std::cout << "Channel: Successfully deopped user! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotPermitted) {
      std::cout << "Channel: Not permitted! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_AlreadyNotOperator) {
      std::cout << "Channel: Already not operator! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotInChannel) {
      std::cout << "Channel: Not in channel! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotIdentified) {
      std::cout << "Channel: Not identified! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_InvalidChannelName) {
      std::cout << "Channel: Invalid channel name! (" << channel_name
        << ", " << username << ")" << std::endl;
    }
    return true;
  });
  channel_component->OnKickUserCompleted.Add([](
    jchat::ChannelMessageResult result, std::string &channel_name,
    std::string &username) {
    if (result == jchat::kChannelMessageResult_Ok) {
      std::cout << "Channel: Successfully kicked user! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotPermitted) {
      std::cout << "Channel: Not permitted! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_UserNotInChannel) {
      std::cout << "Channel: User not in channel! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_CannotKickSelf) {
      std::cout << "Channel: Cannot kick self! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotInChannel) {
      std::cout << "Channel: Not in channel! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotIdentified) {
      std::cout << "Channel: Not identified! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_InvalidChannelName) {
      std::cout << "Channel: Invalid channel name! (" << channel_name
        << ", " << username << ")" << std::endl;
    }
    return true;
  });
  channel_component->OnBanUserCompleted.Add([](
    jchat::ChannelMessageResult result, std::string &channel_name,
    std::string &username) {
    if (result == jchat::kChannelMessageResult_Ok) {
      std::cout << "Channel: Successfully banned user! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotPermitted) {
      std::cout << "Channel: Not permitted! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_UserNotInChannel) {
      std::cout << "Channel: User not in channel! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_AlreadyBanned) {
      std::cout << "Channel: User already banned! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_CannotBanSelf) {
      std::cout << "Channel: Cannot ban self! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotInChannel) {
      std::cout << "Channel: Not in channel! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotIdentified) {
      std::cout << "Channel: Not identified! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_InvalidChannelName) {
      std::cout << "Channel: Invalid channel name! (" << channel_name
        << ", " << username << ")" << std::endl;
    }
    return true;
  });
  channel_component->OnUnbanUserCompleted.Add([](
    jchat::ChannelMessageResult result, std::string &channel_name,
    std::string &username) {
    if (result == jchat::kChannelMessageResult_Ok) {
      std::cout << "Channel: Successfully banned user! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotPermitted) {
      std::cout << "Channel: Not permitted! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotBanned) {
      std::cout << "Channel: User not bannned! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_AlreadyBanned) {
      std::cout << "Channel: User already banned! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_CannotUnbanSelf) {
      std::cout << "Channel: Cannot unban self! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotInChannel) {
      std::cout << "Channel: Not in channel! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_NotIdentified) {
      std::cout << "Channel: Not identified! (" << channel_name
        << ", " << username << ")" << std::endl;
    } else if (result == jchat::kChannelMessageResult_InvalidChannelName) {
      std::cout << "Channel: Invalid channel name! (" << channel_name
        << ", " << username << ")" << std::endl;
    }
    return true;
  });
  channel_component->OnChannelJoined.Add([=](jchat::ChatChannel &channel,
    jchat::ChatUser &user) {
    std::shared_ptr<jchat::ChatUser> local_user;
    if (!user_component->GetChatUser(local_user)) {
      return false;
    }

    if (&user != local_user.get()) {
      std::cout << "Channel: " << user.Username << " joined " << channel.Name
        << std::endl;
    }

    return true;
  });
  channel_component->OnChannelLeft.Add([=](jchat::ChatChannel &channel,
    jchat::ChatUser &user) {
    std::shared_ptr<jchat::ChatUser> local_user;
    if (!user_component->GetChatUser(local_user)) {
      return false;
    }

    if (&user != local_user.get()) {
      std::cout << "Channel: " << user.Username << " left " << channel.Name
        << std::endl;
    }

    return true;
  });
  channel_component->OnChannelMessage.Add([=](jchat::ChatChannel &channel,
    jchat::ChatUser &user, std::string &message) {
    std::shared_ptr<jchat::ChatUser> local_user;
    if (!user_component->GetChatUser(local_user)) {
      return false;
    }

    if (&user != local_user.get()) {
      std::cout << "Channel: " << user.Username << " => " << channel.Name
        << ": " << message << std::endl;
    }

    return true;
  });
  channel_component->OnChannelUserOpped.Add([=](jchat::ChatChannel &channel,
    jchat::ChatUser &user) {
    std::shared_ptr<jchat::ChatUser> local_user;
    if (!user_component->GetChatUser(local_user)) {
      return false;
    }

    if (&user != local_user.get()) {
      std::cout << "Channel: " << user.Username << " was opped"
        << " (" << channel.Name << ")" << std::endl;
    } else {
      std::cout << "Channel: You were opped"
        << " (" << channel.Name << ")" << std::endl;
    }

    return true;
  });
  channel_component->OnChannelUserDeopped.Add([=](jchat::ChatChannel &channel,
    jchat::ChatUser &user) {
    std::shared_ptr<jchat::ChatUser> local_user;
    if (!user_component->GetChatUser(local_user)) {
      return false;
    }

    if (&user != local_user.get()) {
      std::cout << "Channel: " << user.Username << " was deopped"
        << " (" << channel.Name << ")" << std::endl;
    } else {
      std::cout << "Channel: You were deopped"
        << " (" << channel.Name << ")" << std::endl;
    }

    return true;
  });
  channel_component->OnChannelUserKicked.Add([=](jchat::ChatChannel &channel,
    jchat::ChatUser &user) {
    std::shared_ptr<jchat::ChatUser> local_user;
    if (!user_component->GetChatUser(local_user)) {
      return false;
    }

    if (&user != local_user.get()) {
      std::cout << "Channel: " << user.Username << " was kicked"
        << " (" << channel.Name << ")" << std::endl;
    } else {
      std::cout << "Channel: You were kicked"
        << " (" << channel.Name << ")" << std::endl;
    }

    return true;
  });
  channel_component->OnChannelUserBanned.Add([=](jchat::ChatChannel &channel,
    jchat::ChatUser &user) {
    std::shared_ptr<jchat::ChatUser> local_user;
    if (!user_component->GetChatUser(local_user)) {
      return false;
    }

    if (&user != local_user.get()) {
      std::cout << "Channel: " << user.Username << " was banned"
        << " (" << channel.Name << ")" << std::endl;
    } else {
      std::cout << "Channel: You were banned"
        << " (" << channel.Name << ")" << std::endl;
    }

    return true;
  });
  channel_component->OnChannelUserUnbanned.Add([=](jchat::ChatChannel &channel,
    std::string &username, std::string &hostname) {
    std::shared_ptr<jchat::ChatUser> local_user;
    if (!user_component->GetChatUser(local_user)) {
      return false;
    }

    if (local_user->Username != username && local_user->Hostname != hostname) {
      std::cout << "Channel: " << username << " was unbanned"
        << " (" << channel.Name << ")" << std::endl;
    }

    return true;
  });

  // Add the components to the client instance
  chat_client.AddComponent(system_component);
  chat_client.AddComponent(user_component);
  chat_client.AddComponent(channel_component);

  // Connect to the server and read input
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
        user_component->Identify(username);
      } else if (command == "join" && arguments.size() == 1) {
        std::string &target = arguments[0];
        channel_component->JoinChannel(target);
      } else if (command == "leave" && arguments.size() == 1) {
        std::string &target = arguments[0];
        channel_component->LeaveChannel(target);
      } else if (command == "msg" && arguments.size() >= 2) {
        std::string &target = arguments[0];
        std::string message = jchat::String::Join(
          std::vector<std::string>(arguments.begin() + 1, arguments.end()),
          " ");
        if (!target.empty() && target[0] == '#') {
          channel_component->SendMessage(target, message);
        } else {
          user_component->SendMessage(target, message);
        }
      } /*else if (command == "op" && arguments.size() == 2) {
        std::string &channel = arguments[0];
        std::string &target = arguments[1];
        channel_component->OpUser(channel, target);
      } else if (command == "deop" && arguments.size() == 2) {
        std::string &channel = arguments[0];
        std::string &target = arguments[1];
        channel_component->DeopUser(channel, target);
      } */else if (command == "kick" && arguments.size() == 2) {
        std::string &channel = arguments[0];
        std::string &target = arguments[1];
        channel_component->KickUser(channel, target);
      } else if (command == "ban" && arguments.size() == 2) {
        std::string &channel = arguments[0];
        std::string &target = arguments[1];
        channel_component->BanUser(channel, target);
      } /*else if (command == "unban" && arguments.size() == 2) {
        std::string &channel = arguments[0];
        std::string &target = arguments[1];
        channel_component->UnbanUser(channel, target);
      } */else if (command == "quit" && arguments.size() == 0) {
        exit(0);
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
