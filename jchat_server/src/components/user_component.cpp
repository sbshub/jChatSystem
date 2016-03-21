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

namespace jchat {
UserComponent::UserComponent() {
}

UserComponent::~UserComponent() {
  if (!users_.empty()) {
    for (auto pair : users_) {
      delete pair.second;
    }
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
    for (auto pair : users_) {
      delete pair.second;
    }
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
    for (auto pair : users_) {
      delete pair.second;
    }
    users_.clear();
  }
  users_mutex_.unlock();

  return true;
}

void UserComponent::OnClientConnected(RemoteChatClient &client) {
  // Create a chat user class instance that we can use to store information
  // about the client
  ChatUser *chat_user = new ChatUser();

  // Store the user
  users_mutex_.lock();
  users_[&client] = chat_user;
  users_mutex_.unlock();

  // Give the client a guest username (which will prevent it from accessing
  // anything until it has authenticated)
  chat_user->Username = "guest";

  // Set the IP address as the endpoint until the client authenticates
  chat_user->Hostname = client.Endpoint.GetAddressString();
}

void UserComponent::OnClientDisconnected(RemoteChatClient &client) {
  users_mutex_.lock();

  // NOTE (CRITICAL): Depending on which component handler is called first
  // this will delete the ChatUser object and then the ChannelComponent
  // will not be able to use this object in order to notify other clients that
  // the user has disconnected, unless we store a copy of ChatUser in
  // ChannelComponent somehow and not use the same ChatUser object we use here.
  ChatUser *user = users_[&client];

  // Delete user
  delete user;

  users_.erase(&client);
  users_mutex_.unlock();
}

ComponentType UserComponent::GetType() {
  return kComponentType_User;
}

bool UserComponent::Handle(RemoteChatClient &client, uint16_t message_type,
  TypedBuffer &buffer) {

  return false;
}

bool UserComponent::GetChatUser(RemoteChatClient &client, ChatUser *out_user) {
  users_mutex_.lock();
  if (users_.find(&client) == users_.end()) {
    users_mutex_.unlock();
    return false;
  } else {
    user = users_[&client];
    users_mutex_.unlock();
    return true;
  }
}
}
