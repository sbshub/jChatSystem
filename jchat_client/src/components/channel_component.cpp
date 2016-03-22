/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/channel_component.h"
#include "chat_client.h"
#include "protocol/version.h"
#include "protocol/components/channel_message_type.h"

namespace jchat {
ChannelComponent::ChannelComponent() {
}

ChannelComponent::~ChannelComponent() {
  // Remove channels
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
}

bool ChannelComponent::Initialize(ChatClient &client) {
  client_ = &client;
  return true;
}

bool ChannelComponent::Shutdown() {
  client_ = 0;

  // Remove channels
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }

  return true;
}

void ChannelComponent::OnConnected() {

}

void ChannelComponent::OnDisconnected() {
  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
  channels_mutex_.unlock();
}

ComponentType ChannelComponent::GetType() {
  return kComponentType_Channel;
}

bool ChannelComponent::Handle(uint16_t message_type, TypedBuffer &buffer) {
  if (message_type == kChannelMessageType_JoinChannel_Complete) {
    // TODO: 

    return true;
  }

  return false;
}
}
