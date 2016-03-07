/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_server_chat_handler_h_
#define jchat_server_chat_handler_h_

#include "remote_chat_client.h"
#include "typed_buffer.hpp"

namespace jchat {
class ChatServer;
class ChatHandler {
public:
  virtual uint8_t GetType() = 0;
  virtual bool Handle(ChatServer &server, RemoteChatClient &client,
    TypedBuffer &buffer) = 0;
};
}

#endif // jchat_server_chat_handler_h_
