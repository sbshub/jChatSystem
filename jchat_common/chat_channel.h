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

namespace jchat {
struct ChatChannel {
  std::string Name;
  // TODO/NOTE: This is subject to change
  std::map<RemoteChatClient *, ChatUser *> Operators;
  std::mutex OperatorsMutex;
  std::map<RemoteChatClient *, ChatUser *> Clients;
  std::mutex ClientsMutex;
};
}

#endif // jchat_common_chat_channel_h_
