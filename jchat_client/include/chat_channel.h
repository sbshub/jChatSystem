/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_client_chat_channel_h_
#define jchat_client_chat_channel_h_

#include "chat_user.h"
#include <vector>
#include <memory>

namespace jchat {
struct ChatChannel {
  bool Enabled;
  std::string Name;
  std::vector<std::shared_ptr<ChatUser>> Operators;
  std::mutex OperatorsMutex;
  std::vector<std::shared_ptr<ChatUser>> Clients;
  std::mutex ClientsMutex;
  std::vector<std::string> BannedUsers; // Format: username@hostname
  std::mutex BannedUsersMutex;
};
}

#endif // jchat_client_chat_channel_h_
