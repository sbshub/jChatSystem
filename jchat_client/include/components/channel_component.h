/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_client_channel_component_h_
#define jchat_client_channel_component_h_

#include "chat_component.h"
#include "chat_channel.h"
#include "protocol/components/channel_message_result.h"
#include "event.hpp"

namespace jchat {
class ChannelComponent : public ChatComponent {
private:
  ChatClient *client_;
  std::vector<ChatChannel *> channels_;
  std::mutex channels_mutex_;

public:
  ChannelComponent();
  ~ChannelComponent();

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
