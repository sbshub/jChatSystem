/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_chat_channel_h_
#define jchat_common_chat_channel_h_

#include "remote_chat_client.h"
#include "chat_user.h"
#include <map>
#include <memory>

namespace jchat {
// TODO/NOTE: This is subject to change
struct ChatChannel {
  std::string Name;
  std::map<RemoteChatClient *, std::shared_ptr<ChatUser>> Operators;
  std::mutex OperatorsMutex;
  std::map<RemoteChatClient *, std::shared_ptr<ChatUser>> Clients;
  std::mutex ClientsMutex;
  std::vector<std::string> BannedUsers; // Format: username@hostname
  std::mutex BannedUsersMutex;
};
}

#endif // jchat_common_chat_channel_h_
