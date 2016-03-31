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
#include "string.hpp"

namespace jchat {
UserComponent::UserComponent() {
}

UserComponent::~UserComponent() {
  user_.reset();
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

}

void UserComponent::OnDisconnected() {
	// Removes user
	users_mutex_.lock();
  if (!users_.empty()) {
    users_.clear();
  }
	users_mutex_.unlock();
}

ComponentType UserComponent::GetType() {
  return kComponentType_User;
}

bool UserComponent::Handle(uint16_t message_type, TypedBuffer &buffer) {
  if(message_type == kUserMessageType_Identify){
  	std::string user_name;
  	if (!buffer.ReadString(user_name)){
  		return false;
  	}

  	if (!String::Contains(user_name, "#")) {
      buffer.WriteUInt16(kUserMessageResult_InvalidUsername);
      return true;
    } else if (String::Contains(user_name, "#")) {
      return false;
    }

    if (message_type == kUserMessageType_Max){
      if(user_name.length() > 20){
        return false;
      }
      buffer.WriteUInt16(kUserMessageResult_Max);
      return true;
    }

  	if(message_type != kUserMessageResult_Ok){
  		return true;
  	}

  }
  return false;
}
}
