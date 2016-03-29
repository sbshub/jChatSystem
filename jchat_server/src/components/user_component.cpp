/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/user_component.h"
#include "chat_server.h"
#include "protocol/version.h"
#include "protocol/components/user_message_type.h"
#include "utility.hpp"
#include "string.hpp"

namespace jchat {
UserComponent::UserComponent() {
}

UserComponent::~UserComponent() {
  if (!users_.empty()) {
    users_.clear();
  }
}

bool UserComponent::Initialize(ChatServer &server) {
  server_ = &server;
  return true;
}

bool UserComponent::Shutdown() {
  server_ = 0;

  // Remove users
  users_mutex_.lock();
  if (!users_.empty()) {
    users_.clear();
  }
  users_mutex_.unlock();

  return true;
}

bool UserComponent::OnStart() {
  return true;
}

bool UserComponent::OnStop() {
  // Remove users
  users_mutex_.lock();
  if (!users_.empty()) {
    users_.clear();
  }
  users_mutex_.unlock();

  return true;
}

void UserComponent::OnClientConnected(RemoteChatClient &client) {
  // Create a chat user class instance that we can use to store information
  // about the client
  auto chat_user = std::make_shared<ChatUser>();

  // Store the user
  users_mutex_.lock();
  users_[&client] = chat_user;
  users_mutex_.unlock();

  // Give the client a guest username (which will prevent it from accessing
  // anything until it has authenticated)
  chat_user->Username = "guest-" + std::to_string(Utility::Random(100000,
    999999));

  // Set the IP address as the endpoint until the client authenticates
  chat_user->Hostname = client.Endpoint.GetAddressString();
}

void UserComponent::OnClientDisconnected(RemoteChatClient &client) {
  users_mutex_.lock();

  std::shared_ptr<ChatUser> &user = users_[&client];

  // TODO: Do anything with the user that we need to

  // Delete user
  users_.erase(&client);
  users_mutex_.unlock();
}

ComponentType UserComponent::GetType() {
  return kComponentType_User;
}

bool UserComponent::Handle(RemoteChatClient &client, uint16_t message_type,
  TypedBuffer &buffer) {
   // Reads in the username 
  if (message_type == kUserMessageType_Identify) {
    std::string user_name;
    if (!buffer.ReadString(user_name)) {
      return true;
    }
   
    if (!String::Contains(user_name, "#")) {
      TypedBuffer send_buffer = server_->CreateBuffer(); // Not sure if to user server here or not, let me know 
      buffer.WriteUInt16(kUserMessageResult_InvalidUsername);
      server_->SendUnicast(client, kComponentType_User,
      kUserMessageType_Complete_Identify, send_buffer);
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
  } 
  return false;
}

bool UserComponent::GetChatUser(RemoteChatClient &client,
  std::shared_ptr<ChatUser> &out_user) {
  users_mutex_.lock();
  if (users_.find(&client) == users_.end()) {
    users_mutex_.unlock();
    return false;
  } else {
    out_user = users_[&client];
    users_mutex_.unlock();
    return true;
  }
}
}
