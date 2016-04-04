/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_server_chat_component_h_
#define jchat_server_chat_component_h_

#include "protocol/component_type.h"
#include "remote_chat_client.h"
#include "typed_buffer.hpp"

namespace jchat {
class ChatServer;
class ChatComponent {
public:  
  // Internal functions
  virtual bool Initialize(ChatServer &server) = 0;
  virtual bool Shutdown() = 0;

  virtual bool OnStart() = 0;
  virtual bool OnStop() = 0;

  // Internal events
  virtual void OnClientConnected(RemoteChatClient &client) = 0;
  virtual void OnClientDisconnected(RemoteChatClient &client) = 0;

  // Handler functions
  virtual ComponentType GetType() = 0;
  virtual bool Handle(RemoteChatClient &client, uint16_t message_type,
    TypedBuffer &buffer) = 0;
};
}

#endif // jchat_server_chat_component_h_
