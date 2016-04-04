/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/system_component.h"
#include "components/user_component.h"
#include "chat_server.h"
#include "protocol/protocol.h"
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

    // Get user component
    std::shared_ptr<UserComponent> user_component;
    if (!server_->GetComponent(kComponentType_User, user_component)) {
      // Internal error, disconnect client
      return false;
    }

    // Get the chat client
    std::shared_ptr<ChatUser> chat_user;
    if (!user_component->GetChatUser(client, chat_user)) {
      // Internal error, disconnect client
      return false;
    }

    // Set as enabled
    chat_user->Enabled = true;

    TypedBuffer send_buffer = server_->CreateBuffer();
    send_buffer.WriteUInt16(kSystemMessageResult_Ok);
    server_->Send(client, kComponentType_System,
      kSystemMessageType_Hello_Complete, send_buffer);

	return true;
  }

  return false;
}
}
