/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_server_system_component_h_
#define jchat_server_system_component_h_

#include "chat_component.h"
#include "protocol/components/system_message_result.h"
#include "event.hpp"

namespace jchat {
class SystemComponent : public ChatComponent {
private:
  ChatServer *server_;

public:
  SystemComponent();
  ~SystemComponent();

  // Internal functions
  virtual bool Initialize(ChatServer &server) override;
  virtual bool Shutdown() override;

  virtual bool OnStart() override;
  virtual bool OnStop() override;

  // Internal events
  virtual void OnClientConnected(RemoteChatClient &client) override;
  virtual void OnClientDisconnected(RemoteChatClient &client) override;

  // Handler functions
  virtual ComponentType GetType() override;
  virtual bool Handle(RemoteChatClient &client, uint16_t message_type,
    TypedBuffer &buffer) override;

  // API events
  Event<RemoteChatClient &> OnHelloCompleted;
};
}

#endif // jchat_server_system_component_h_
