/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_client_chat_handler_h_
#define jchat_client_chat_handler_h_

#include "remote_chat_client.h"
#include "typed_buffer.hpp"

namespace jchat {
class ChatClient;
class ChatHandler {
public:
  virtual uint8_t GetType() = 0;
  virtual bool Handle(ChatClient &client, TypedBuffer &buffer) = 0;
};
}

#endif // jchat_client_chat_handler_h_
