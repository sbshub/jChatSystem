/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_remote_chat_client_h_
#define jchat_common_remote_chat_client_h_

#include "ip_endpoint.hpp"
#include <string>
#include <vector>
#include <mutex>

namespace jchat {
struct RemoteChatClient {
  IPEndpoint Endpoint;
  std::string Username;
  std::string Hostname;
  bool Identified;
  std::vector<std::string> Channels;
  std::mutex ChannelsMutex;
};
}

#endif // jchat_common_remote_chat_client_h_
