/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/system_component.h"
#include "chat_server.h"
#include "protocol/version.h"
#include "protocol/components/system_message_type.h"

namespace jchat {
SystemComponent::SystemComponent() {
}

SystemComponent::~SystemComponent() {
}

bool SystemComponent::Initialize(ChatServer &server) {
  server_ = &server;
  return true;
}

bool SystemComponent::Shutdown() {
  server_ = 0;
  return true;
}

bool SystemComponent::OnStart() {
  return true;
}

bool SystemComponent::OnStop() {
  return true;
}

void SystemComponent::OnClientConnected(RemoteChatClient &client) {
  
}

void SystemComponent::OnClientDisconnected(RemoteChatClient &client) {

}

ComponentType SystemComponent::GetType() {
  return kComponentType_System;
}

bool SystemComponent::Handle(RemoteChatClient &client, uint16_t message_type,
  TypedBuffer &buffer) {
  if (message_type == kSystemMessageType_Hello) {
    std::string protocol_version;
    if (!buffer.ReadString(protocol_version)
      || protocol_version != JCHAT_CHAT_PROTOCOL_VERSION) {
      return false;
    }
    if (!OnHelloCompleted(client)) {
      return false;
    }
    TypedBuffer send_buffer = server_->CreateBuffer();
    send_buffer.WriteUInt16(kSystemMessageResult_Ok);
    return server_->SendUnicast(client, kComponentType_System,
      kSystemMessageType_Complete_Hello, send_buffer);
  }

  return false;
}
}
