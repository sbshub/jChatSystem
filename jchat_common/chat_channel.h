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

namespace jchat {
struct ChatChannel {
  std::string Name;
  std::vector<RemoteChatClient *> Operators;
  std::mutex OperatorsMutex;
  std::vector<RemoteChatClient *> Clients;
  std::mutex ClientsMutex;
};
}

#endif // jchat_common_chat_channel_h_
