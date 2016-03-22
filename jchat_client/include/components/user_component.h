/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_client_user_component_h_
#define jchat_client_user_component_h_

#include "chat_component.h"
#include "chat_user.h"
#include "protocol/components/user_message_result.h"
#include "event.hpp"

namespace jchat {
class UserComponent : public ChatComponent {
private:
  ChatClient *client_;
  // NOTE: This is the local user
  ChatUser *user_;
  // TODO/NOTE: Create and store RemoteChatClient/ChatUser map for remote users
  // which will be used by the ChannelComponent

public:
  UserComponent();
  ~UserComponent();

  // Internal functions
  virtual bool Initialize(ChatClient &client) override;
  virtual bool Shutdown() override;

  // Internal events
  virtual void OnConnected() override;
  virtual void OnDisconnected() override;

  // Handler functions
  virtual ComponentType GetType() override;
  virtual bool Handle(uint16_t message_type, TypedBuffer &buffer) override;

  // API functions


  // API events

};
}

#endif // jchat_client_channel_component_h_
