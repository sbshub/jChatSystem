/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_server_chat_handler_h_
#define jchat_server_chat_handler_h_

#include "protocol/component_type.h"
#include "remote_chat_client.h"
#include "typed_buffer.hpp"

namespace jchat {
class ChatServer;
class ChatHandler {
public:
  virtual bool Initialize(ChatServer &server) = 0;
  virtual bool Shutdown(ChatServer &server) = 0;

  virtual ComponentType GetType() = 0;
  virtual bool Handle(ChatServer &server, RemoteChatClient &client,
    uint8_t message_type, TypedBuffer &buffer) = 0;
};
}

#endif // jchat_server_chat_handler_h_
