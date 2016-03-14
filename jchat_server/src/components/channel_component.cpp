/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/channel_component.h"
#include "chat_server.h"
#include "protocol/version.h"
#include "protocol/components/channel_message_type.h"

namespace jchat {
ChannelComponent::ChannelComponent() {
}

ChannelComponent::~ChannelComponent() {
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
}

bool ChannelComponent::Initialize(ChatServer &server) {
  server_ = &server;
  return true;
}

bool ChannelComponent::Shutdown() {
  server_ = 0;

  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
  channels_mutex_.unlock();

  return true;
}

bool ChannelComponent::OnStart() {
  return true;
}

bool ChannelComponent::OnStop() {
  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
  channels_mutex_.unlock();

  return true;
}

void ChannelComponent::OnClientConnected(RemoteChatClient &client) {

}

void ChannelComponent::OnClientDisconnected(RemoteChatClient &client) {

}

ComponentType ChannelComponent::GetType() {
  return kComponentType_Channel;
}

bool ChannelComponent::Handle(RemoteChatClient &client, uint16_t message_type,
  TypedBuffer &buffer) {

  return false;
}
}
