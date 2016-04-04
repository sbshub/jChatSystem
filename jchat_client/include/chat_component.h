/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_client_chat_component_h_
#define jchat_client_chat_component_h_

#include "protocol/component_type.h"
#include "remote_chat_client.h"
#include "typed_buffer.hpp"

namespace jchat {
class ChatClient;
class ChatComponent {
public:
  // Internal functions
  virtual bool Initialize(ChatClient &client) = 0;
  virtual bool Shutdown() = 0;

  // Internal events
  virtual void OnConnected() = 0;
  virtual void OnDisconnected() = 0;

  // Handler functions
  virtual ComponentType GetType() = 0;
  virtual bool Handle(uint16_t message_type, TypedBuffer &buffer) = 0;
};
}

#endif // jchat_client_chat_handler_h_
