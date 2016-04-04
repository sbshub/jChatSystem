/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/system_component.h"
#include "chat_client.h"
#include "protocol/protocol.h"
#include "protocol/components/system_message_type.h"

namespace jchat {
SystemComponent::SystemComponent() {
}

SystemComponent::~SystemComponent() {
}

bool SystemComponent::Initialize(ChatClient &client) {
  client_ = &client;
  return true;
}

bool SystemComponent::Shutdown() {
  client_ = 0;
  return true;
}

void SystemComponent::OnConnected() {
  // Send a hello to the server specifying the protocol version
  // this is used to see if this specific protocol is accepted by
  // the server
  SendHello();
}

void SystemComponent::OnDisconnected() {

}

ComponentType SystemComponent::GetType() {
  return kComponentType_System;
}

bool SystemComponent::Handle(uint16_t message_type, TypedBuffer &buffer) {
  if (message_type == kSystemMessageType_Hello_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    OnHelloCompleted(static_cast<SystemMessageResult>(message_result));
    if (message_result != kSystemMessageResult_Ok) {
      return false;
    }
    return true;
  }

  return false;
}

bool SystemComponent::SendHello() {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(JCHAT_CHAT_PROTOCOL_VERSION);
  return client_->Send(kComponentType_System, kSystemMessageType_Hello, buffer);
}
}
