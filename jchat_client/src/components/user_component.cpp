/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/user_component.h"
#include "chat_client.h"
#include "protocol/version.h"
#include "protocol/components/user_message_type.h"

namespace jchat {
UserComponent::UserComponent() {
}

UserComponent::~UserComponent() {
  if (user_ != 0) {
    delete user_;
    user_ = 0;
  }
}

bool UserComponent::Initialize(ChatClient &client) {
  client_ = &client;
  user_ = new ChatUser();

  return true;
}

bool UserComponent::Shutdown() {
  client_ = 0;
  if (user_ != 0) {
    delete user_;
    user_ = 0;
  }

  return true;
}

void UserComponent::OnConnected() {

}

void UserComponent::OnDisconnected() {

}

ComponentType UserComponent::GetType() {
  return kComponentType_User;
}

bool UserComponent::Handle(uint16_t message_type, TypedBuffer &buffer) {


  return false;
}
}
