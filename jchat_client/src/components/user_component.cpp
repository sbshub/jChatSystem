/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/user_component.h"
#include "chat_client.h"
#include "protocol/components/user_message_type.h"

namespace jchat {
UserComponent::UserComponent() {
}

UserComponent::~UserComponent() {
}

bool UserComponent::Initialize(ChatClient &client) {
  client_ = &client;
  user_ = std::make_shared<ChatUser>();

  return true;
}

bool UserComponent::Shutdown() {
  client_ = 0;
  user_.reset();

  return true;
}

void UserComponent::OnConnected() {
  // NOTE: This should happen on protocol verification (SystemComponent::Hello)
  user_->Enabled = true;
}

void UserComponent::OnDisconnected() {
  user_->Enabled = false;
}

ComponentType UserComponent::GetType() {
  return kComponentType_User;
}

bool UserComponent::Handle(uint16_t message_type, TypedBuffer &buffer) {
  if (message_type == kUserMessageType_Identify_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string username;
    if (!buffer.ReadString(username)) {
      return false;
    }
    OnIdentifyCompleted(static_cast<UserMessageResult>(message_result),
      username);
    if (message_result == kUserMessageResult_Ok) {
      std::string hostname;
      if (!buffer.ReadString(hostname)) {
        return false;
      }
      user_->Username = username;
      user_->Hostname = hostname;
      user_->Identified = true;

      OnIdentified();
    }

    return true;
  } else if (message_type == kUserMessageType_SendMessage_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string username;
    if (!buffer.ReadString(username)) {
      return false;
    }
    std::string message;
    if (!buffer.ReadString(message)) {
      return false;
    }
    OnSendMessageCompleted(static_cast<UserMessageResult>(message_result),
      username, message);
    if (message_result == kUserMessageResult_Ok) {
      OnMessage(user_->Username, user_->Hostname, username, message);
    }

    return true;
  } else if (message_type == kUserMessageType_SendMessage) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string username;
    if (!buffer.ReadString(username)) {
      return false;
    }
    std::string hostname;
    if (!buffer.ReadString(hostname)) {
      return false;
    }
    std::string message;
    if (!buffer.ReadString(message)) {
      return false;
    }
    if (message_result != kUserMessageResult_MessageSent) {
      return false;
    }
    OnMessage(username, hostname, user_->Username, message);
    return true;
  }

  return false;
}

bool UserComponent::GetChatUser(std::shared_ptr<ChatUser> &out_user) {
  if (user_) {
    out_user = user_;
    return true;
  }
  return false;
}

bool UserComponent::Identify(std::string username) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(username);
  return client_->Send(kComponentType_User, kUserMessageType_Identify, buffer);
}

bool UserComponent::SendMessage(std::string username, std::string message) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(username);
  buffer.WriteString(message);
  return client_->Send(kComponentType_User, kUserMessageType_SendMessage,
    buffer);
}
}
