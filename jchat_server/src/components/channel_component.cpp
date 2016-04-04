/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/channel_component.h"
#include "components/user_component.h"
#include "chat_server.h"
#include "protocol/protocol.h"
#include "protocol/components/channel_message_type.h"
#include "string.hpp"

namespace jchat {
ChannelComponent::ChannelComponent() {
}

ChannelComponent::~ChannelComponent() {
  if (!channels_.empty()) {
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
    channels_.clear();
  }
  channels_mutex_.unlock();

  return true;
}

void ChannelComponent::OnClientConnected(RemoteChatClient &client) {

}

void ChannelComponent::OnClientDisconnected(RemoteChatClient &client) {
  // Notify all clients in participating channels that the client has
  // disconnected
  channels_mutex_.lock();
  for (auto it = channels_.begin(); it != channels_.end(); ++it) {
    std::shared_ptr<ChatChannel> channel = *it;

    if (channel->Enabled) {
      channel->ClientsMutex.lock();
      if (channel->Clients.find(&client) != channel->Clients.end()) {
        // Get the chat user
        std::shared_ptr<ChatUser> chat_user = channel->Clients[&client];

        // Notify all clients in that channel that the client left
        TypedBuffer clients_buffer = server_->CreateBuffer();
        clients_buffer.WriteUInt16(kChannelMessageResult_UserLeft);
        clients_buffer.WriteString(channel->Name);
        clients_buffer.WriteString(chat_user->Username);
        clients_buffer.WriteString(chat_user->Hostname);

        for (auto &pair : channel->Clients) {
          if (pair.first != &client && pair.second->Enabled) {
            server_->Send(pair.first, kComponentType_Channel,
              kChannelMessageType_LeaveChannel, clients_buffer);
          }
        }

        // Trigger the events
        OnChannelLeft(*channel, *chat_user);

        // Remove the client from the clients list
        channel->Clients.erase(&client);

        // If there was nobody in the channel delete it
        if (channel->Clients.empty()) {
          channel->ClientsMutex.unlock();
          channel->OperatorsMutex.lock();
          channel->Operators.clear();
          channel->OperatorsMutex.unlock();
          channel->Enabled = false;
          channel.reset();
          continue;
        }
      }
      channel->ClientsMutex.unlock();

      // Remove the client from the operators list if they're an operator
      channel->OperatorsMutex.lock();
      if (channel->Operators.find(&client)
        != channel->Operators.end()) {
        channel->Operators.erase(&client);
      }
      channel->OperatorsMutex.unlock();
    }
  }
  channels_mutex_.unlock();
}

ComponentType ChannelComponent::GetType() {
  return kComponentType_Channel;
}

bool ChannelComponent::Handle(RemoteChatClient &client, uint16_t message_type,
  TypedBuffer &buffer) {
  if (message_type == kChannelMessageType_JoinChannel) {
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
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

    // Check if the user is logged in
    if (!chat_user->Identified) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotIdentified);
	    send_buffer.WriteString(channel_name);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_JoinChannel_Complete, send_buffer);

      // Trigger events
      OnJoinCompleted(kChannelMessageResult_NotIdentified, channel_name,
        *chat_user);

      return true;
    }

    // Check if the channel name is valid
    if (channel_name.empty() || channel_name[0] != '#') {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_JoinChannel_Complete, send_buffer);

      // Trigger events
      OnJoinCompleted(kChannelMessageResult_InvalidChannelName, channel_name,
        *chat_user);

      return true;
    }

    // Check if the channel exists
    std::shared_ptr<ChatChannel> chat_channel;
    channels_mutex_.lock();
    for (auto &channel : channels_) {
      if (channel->Enabled && channel->Name == channel_name) {
        chat_channel = channel;
        break;
      }
    }
    channels_mutex_.unlock();

    if (!chat_channel) {
      // Check if the channel name is too long
      if (channel_name.size() - 1 > JCHAT_CHAT_CHANNEL_NAME_LENGTH) {
        TypedBuffer send_buffer = server_->CreateBuffer();
        send_buffer.WriteUInt16(kChannelMessageResult_ChannelNameTooLong);
        send_buffer.WriteString(channel_name);
        server_->Send(client, kComponentType_User,
          kChannelMessageType_JoinChannel_Complete, send_buffer);

        // Trigger events
        OnJoinCompleted(kChannelMessageResult_ChannelNameTooLong, channel_name,
          *chat_user);

        return true;
      }

      // Create the channel and add the user to it
      chat_channel = std::make_shared<ChatChannel>();
      chat_channel->Enabled = true;
      chat_channel->Name = channel_name;
      chat_channel->Operators[&client] = chat_user;
      chat_channel->Clients[&client] = chat_user;

      // Add the channel to the component
      channels_mutex_.lock();
      channels_.push_back(chat_channel);
      channels_mutex_.unlock();

      // Notify the client that the channel was created and that they are
      // the operator operator and member of it
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_ChannelCreated);
      send_buffer.WriteString(channel_name);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_JoinChannel_Complete, send_buffer);

      // Trigger the events
      OnJoinCompleted(kChannelMessageResult_ChannelCreated, channel_name,
        *chat_user);

      OnChannelCreated(*chat_channel);
      OnChannelJoined(*chat_channel, *chat_user);

      return true;
    }

    // Check if the user is already in the channel
    chat_channel->ClientsMutex.lock();
    if (chat_channel->Clients.find(&client) != chat_channel->Clients.end()) {
      chat_channel->ClientsMutex.unlock();

      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_AlreadyInChannel);
      send_buffer.WriteString(chat_channel->Name);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_JoinChannel_Complete, send_buffer);

      // Trigger events
      OnJoinCompleted(kChannelMessageResult_AlreadyInChannel,
        chat_channel->Name, *chat_user);

      return true;
    }
    chat_channel->ClientsMutex.unlock();

    // Check if the user is banned
    chat_channel->BannedUsersMutex.lock();
    std::string chat_user_hostinfo = chat_user->Username + "@"
      + chat_user->Hostname;
    for (auto &banned_user : chat_channel->BannedUsers) {
      if (banned_user == chat_user_hostinfo) {
        chat_channel->BannedUsersMutex.unlock();

        TypedBuffer send_buffer = server_->CreateBuffer();
        send_buffer.WriteUInt16(kChannelMessageResult_BannedFromChannel);
        send_buffer.WriteString(chat_channel->Name);
        server_->Send(client, kComponentType_Channel,
          kChannelMessageType_JoinChannel_Complete, send_buffer);

        // Trigger events
        OnJoinCompleted(kChannelMessageResult_BannedFromChannel,
          chat_channel->Name, *chat_user);

        return true;
      }
    }
    chat_channel->BannedUsersMutex.unlock();

    // Add the user to the channel
    chat_channel->ClientsMutex.lock();
    chat_channel->Clients[&client] = chat_user;
    chat_channel->ClientsMutex.unlock();

    // Notify the client that it joined the channel and give it a list of
    // current clients
    TypedBuffer client_buffer = server_->CreateBuffer();
    client_buffer.WriteUInt16(kChannelMessageResult_Ok); // Channel joined
    client_buffer.WriteString(chat_channel->Name);

    chat_channel->OperatorsMutex.lock();
    chat_channel->ClientsMutex.lock();
    size_t client_count = 0;
    for (auto &pair : chat_channel->Clients) {
      if (pair.first != &client && pair.second->Enabled) {
        client_count++;
      }
    }
    client_buffer.WriteUInt64(client_count);
    for (auto &pair : chat_channel->Clients) {
      if (pair.first != &client && pair.second->Enabled) {
        client_buffer.WriteString(pair.second->Username);
        client_buffer.WriteString(pair.second->Hostname);
        client_buffer.WriteBoolean(
          chat_channel->Operators.find(pair.first)
          != chat_channel->Operators.end());
      }
    }
    chat_channel->ClientsMutex.unlock();
    chat_channel->OperatorsMutex.unlock();

    chat_channel->BannedUsersMutex.lock();
    client_buffer.WriteUInt64(chat_channel->BannedUsers.size());
    for (auto &banned_user : chat_channel->BannedUsers) {
      client_buffer.WriteString(banned_user);
    }
    chat_channel->BannedUsersMutex.unlock();

    server_->Send(client, kComponentType_Channel,
      kChannelMessageType_JoinChannel_Complete, client_buffer);

    // Notify all clients in the channel that the user has joined
    TypedBuffer clients_buffer = server_->CreateBuffer();
    clients_buffer.WriteUInt16(kChannelMessageResult_UserJoined);
    clients_buffer.WriteString(chat_channel->Name);
    clients_buffer.WriteString(chat_user->Username);
    clients_buffer.WriteString(chat_user->Hostname);

    chat_channel->ClientsMutex.lock();
    for (auto &pair : chat_channel->Clients) {
      if (pair.first != &client && pair.second->Enabled) {
        server_->Send(pair.first, kComponentType_Channel,
          kChannelMessageType_JoinChannel, clients_buffer);
      }
    }
    chat_channel->ClientsMutex.unlock();

    // Trigger the events
    OnJoinCompleted(kChannelMessageResult_Ok, chat_channel->Name, *chat_user);
    OnChannelJoined(*chat_channel, *chat_user);

    return true;
  } else if (message_type == kChannelMessageType_LeaveChannel) {
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
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

    // Check if the user is logged in
    if (!chat_user->Identified) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotIdentified);
      send_buffer.WriteString(channel_name);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_LeaveChannel_Complete, send_buffer);

      // Trigger events
      OnLeaveCompleted(kChannelMessageResult_NotIdentified, channel_name,
        *chat_user);

      return true;
    }

    // Check if the channel name is valid
    if (channel_name.empty() || channel_name[0] != '#') {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_LeaveChannel_Complete, send_buffer);

      // Trigger events
      OnLeaveCompleted(kChannelMessageResult_InvalidChannelName, channel_name,
        *chat_user);

      return true;
    }

    // Check if the channel exists
    std::shared_ptr<ChatChannel> chat_channel;
    channels_mutex_.lock();
    for (auto &channel : channels_) {
      if (channel->Enabled && channel->Name == channel_name) {
        chat_channel = channel;
        break;
      }
    }
    channels_mutex_.unlock();

    if (!chat_channel) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_LeaveChannel_Complete, send_buffer);

      // Trigger events
      OnLeaveCompleted(kChannelMessageResult_InvalidChannelName, channel_name,
        *chat_user);

      return true;
    }

    // Check if the user is in the channel
    chat_channel->ClientsMutex.lock();
    if (chat_channel->Clients.find(&client) == chat_channel->Clients.end()) {
      chat_channel->ClientsMutex.unlock();

      // Notify the client that they are not in the channel
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotInChannel);
      send_buffer.WriteString(chat_channel->Name);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_LeaveChannel_Complete, send_buffer);

      // Trigger events
      OnLeaveCompleted(kChannelMessageResult_NotInChannel, chat_channel->Name,
        *chat_user);

      return true;
    }
    chat_channel->ClientsMutex.unlock();

    // Notify all clients in that channel that the client left
    TypedBuffer clients_buffer = server_->CreateBuffer();
    clients_buffer.WriteUInt16(kChannelMessageResult_UserLeft);
    clients_buffer.WriteString(chat_channel->Name);
    clients_buffer.WriteString(chat_user->Username);
    clients_buffer.WriteString(chat_user->Hostname);

    chat_channel->ClientsMutex.lock();
    for (auto &pair : chat_channel->Clients) {
      if (pair.first != &client && pair.second->Enabled) {
        server_->Send(pair.first, kComponentType_Channel,
          kChannelMessageType_LeaveChannel, clients_buffer);
      }
    }
    chat_channel->ClientsMutex.unlock();

    // Notify the client that they left the channel
    TypedBuffer send_buffer = server_->CreateBuffer();
    send_buffer.WriteUInt16(kChannelMessageResult_Ok);
    send_buffer.WriteString(chat_channel->Name);
    server_->Send(client, kComponentType_Channel,
      kChannelMessageType_LeaveChannel_Complete, send_buffer);

    // Trigger events
    OnLeaveCompleted(kChannelMessageResult_Ok, chat_channel->Name, *chat_user);
    OnChannelLeft(*chat_channel, *chat_user);

    // Remove the client from the clients list
    chat_channel->Clients.erase(&client);

    // Remove the client from the operators list if they're an operator
    chat_channel->OperatorsMutex.lock();
    if (chat_channel->Operators.find(&client)
      != chat_channel->Operators.end()) {
      chat_channel->Operators.erase(&client);
    }
    chat_channel->OperatorsMutex.unlock();

    // If there was nobody in the channel delete it
    chat_channel->ClientsMutex.lock();
    if (chat_channel->Clients.empty()) {
      chat_channel->ClientsMutex.unlock();
      chat_channel->Enabled = false;
      chat_channel.reset();
    } else {
      chat_channel->ClientsMutex.unlock();
    }

    return true;
  } else if (message_type == kChannelMessageType_SendMessage) {
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }

    std::string message;
    if (!buffer.ReadString(message)) {
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

    // Check if the user is logged in
    if (!chat_user->Identified) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotIdentified);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(message);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_SendMessage_Complete, send_buffer);

      // Trigger events
      OnSendMessageCompleted(kChannelMessageResult_NotIdentified, channel_name,
        message, *chat_user);

      return true;
    }

    // Check if the channel name is valid
    if (channel_name.empty() || channel_name[0] != '#') {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(message);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_SendMessage_Complete, send_buffer);

      // Trigger events
      OnSendMessageCompleted(kChannelMessageResult_InvalidChannelName,
        channel_name, message, *chat_user);

      return true;
    }

    // Check if the message is valid
    if (message.empty()) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidMessage);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(message);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_SendMessage_Complete, send_buffer);

      // Trigger events
      OnSendMessageCompleted(kChannelMessageResult_InvalidMessage,
        channel_name, message, *chat_user);

      return true;
    }

    if (message.size() > JCHAT_CHAT_MESSAGE_LENGTH) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_MessageTooLong);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(message);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_SendMessage_Complete, send_buffer);

      // Trigger events
      OnSendMessageCompleted(kChannelMessageResult_MessageTooLong,
        channel_name, message, *chat_user);

      return true;
    }

    // Check if the channel exists
    std::shared_ptr<ChatChannel> chat_channel;
    channels_mutex_.lock();
    for (auto &channel : channels_) {
      if (channel->Enabled && channel->Name == channel_name) {
        chat_channel = channel;
        break;
      }
    }
    channels_mutex_.unlock();

    if (!chat_channel) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(message);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_SendMessage_Complete, send_buffer);

      // Trigger events
      OnSendMessageCompleted(kChannelMessageResult_InvalidChannelName,
        channel_name, message, *chat_user);

      return true;
    }

    // Check if the user is in the channel
    chat_channel->ClientsMutex.lock();
    if (chat_channel->Clients.find(&client) == chat_channel->Clients.end()) {
      chat_channel->ClientsMutex.unlock();

      // Notify the client that they are not in the channel
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotInChannel);
      send_buffer.WriteString(chat_channel->Name);
      send_buffer.WriteString(message);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_SendMessage_Complete, send_buffer);

      // Trigger events
      OnSendMessageCompleted(kChannelMessageResult_NotInChannel,
        chat_channel->Name, message, *chat_user);

      return true;
    }
    chat_channel->ClientsMutex.unlock();

    // Send the message to all the clients
    TypedBuffer clients_buffer = server_->CreateBuffer();
    clients_buffer.WriteUInt16(kChannelMessageResult_MessageSent);
    clients_buffer.WriteString(chat_channel->Name);
    clients_buffer.WriteString(chat_user->Username);
    clients_buffer.WriteString(chat_user->Hostname);
    clients_buffer.WriteString(message);

    chat_channel->ClientsMutex.lock();
    for (auto &pair : chat_channel->Clients) {
      if (pair.first != &client && pair.second->Enabled) {
        server_->Send(pair.first, kComponentType_Channel,
          kChannelMessageType_SendMessage, clients_buffer);
      }
    }
    chat_channel->ClientsMutex.unlock();

    // Tell the client that the message was sent
    TypedBuffer send_buffer = server_->CreateBuffer();
    send_buffer.WriteUInt16(kChannelMessageResult_Ok);
    send_buffer.WriteString(chat_channel->Name);
    send_buffer.WriteString(message);
    server_->Send(client, kComponentType_Channel,
      kChannelMessageType_SendMessage_Complete, send_buffer);

    // Trigger events
    OnSendMessageCompleted(kChannelMessageResult_Ok, chat_channel->Name,
      message, *chat_user);
    OnChannelMessage(*chat_channel, *chat_user, message);

    return true;
  } else if (message_type == kChannelMessageType_OpUser) {
    // TODO: Implement
    return false;
  } else if (message_type == kChannelMessageType_DeopUser) {
    // TODO: Implement
    return false;
  } else if (message_type == kChannelMessageType_KickUser) {
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }

    std::string target;
    if (!buffer.ReadString(target)) {
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

    // Check if the user is logged in
    if (!chat_user->Identified) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotIdentified);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_KickUser_Complete, send_buffer);

      // Trigger events
      OnKickUserCompleted(kChannelMessageResult_NotIdentified, channel_name,
        target, *chat_user);

      return true;
    }

    // Check if the channel name is valid
    if (channel_name.empty() || channel_name[0] != '#') {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_KickUser_Complete, send_buffer);

      // Trigger events
      OnKickUserCompleted(kChannelMessageResult_InvalidChannelName,
        channel_name, target, *chat_user);

      return true;
    }

    // Check if the target is valid
    if (target.empty() || String::Contains(target, "#")) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidUsername);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_KickUser_Complete, send_buffer);

      // Trigger events
      OnKickUserCompleted(kChannelMessageResult_InvalidUsername,
        channel_name, target, *chat_user);

      return true;
    }

    // Check if the channel exists
    std::shared_ptr<ChatChannel> chat_channel;
    channels_mutex_.lock();
    for (auto &channel : channels_) {
      if (channel->Enabled && channel->Name == channel_name) {
        chat_channel = channel;
        break;
      }
    }
    channels_mutex_.unlock();

    if (!chat_channel) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_KickUser_Complete, send_buffer);

      // Trigger events
      OnKickUserCompleted(kChannelMessageResult_InvalidChannelName,
        channel_name, target, *chat_user);

      return true;
    }

    // Check if the user is in the channel
    chat_channel->ClientsMutex.lock();
    if (chat_channel->Clients.find(&client) == chat_channel->Clients.end()) {
      chat_channel->ClientsMutex.unlock();

      // Notify the client that they are not in the channel
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotInChannel);
      send_buffer.WriteString(chat_channel->Name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_KickUser_Complete, send_buffer);

      // Trigger events
      OnKickUserCompleted(kChannelMessageResult_NotInChannel, chat_channel->Name,
        target, *chat_user);

      return true;
    }
    chat_channel->ClientsMutex.unlock();

    // Check if the user has permissions
    chat_channel->OperatorsMutex.lock();
    if (chat_channel->Operators.find(&client)
      == chat_channel->Operators.end()) {
      chat_channel->OperatorsMutex.unlock();

      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotPermitted);
      send_buffer.WriteString(chat_channel->Name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_KickUser_Complete, send_buffer);

      // Trigger events
      OnKickUserCompleted(kChannelMessageResult_NotPermitted,
        chat_channel->Name, target, *chat_user);

      return true;
    }
    chat_channel->OperatorsMutex.unlock();

    // Check if the user is trying to kick themself
    if (target == chat_user->Username) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_CannotKickSelf);
      send_buffer.WriteString(chat_channel->Name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_KickUser_Complete, send_buffer);

      // Trigger events
      OnKickUserCompleted(kChannelMessageResult_CannotKickSelf,
        chat_channel->Name, target, *chat_user);

      return true;
    }

    // Check if the target is in the channel
    RemoteChatClient *kick_user_key = 0;
    std::shared_ptr<ChatUser> kick_user;
    chat_channel->ClientsMutex.lock();
    for (auto pair : chat_channel->Clients) {
      if (pair.second->Username == target) {
        kick_user_key = pair.first;
        kick_user = pair.second;
        break;
      }
    }
    chat_channel->ClientsMutex.unlock();

    // Notify other clients
    TypedBuffer clients_buffer = server_->CreateBuffer();
    clients_buffer.WriteUInt16(kChannelMessageResult_UserKicked);
    clients_buffer.WriteString(chat_channel->Name);
    clients_buffer.WriteString(kick_user->Username);
    clients_buffer.WriteString(kick_user->Hostname);

    chat_channel->ClientsMutex.lock();
    for (auto &pair : chat_channel->Clients) {
      if (pair.first != &client && pair.second->Enabled) {
        server_->Send(pair.first, kComponentType_Channel,
          kChannelMessageType_KickUser, clients_buffer);
      }
    }
    chat_channel->ClientsMutex.unlock();

    // Tell the client that the user was banned
    TypedBuffer send_buffer = server_->CreateBuffer();
    send_buffer.WriteUInt16(kChannelMessageResult_Ok);
    send_buffer.WriteString(chat_channel->Name);
    send_buffer.WriteString(target);
    send_buffer.WriteString(kick_user->Username);
    send_buffer.WriteString(kick_user->Hostname);
    server_->Send(client, kComponentType_Channel,
      kChannelMessageType_KickUser_Complete, send_buffer);

    // Remove the client from channel client lists
    chat_channel->OperatorsMutex.lock();
    if (chat_channel->Operators.find(kick_user_key)
      != chat_channel->Operators.end()) {
      chat_channel->Operators.erase(kick_user_key);
    }
    chat_channel->OperatorsMutex.unlock();

    chat_channel->ClientsMutex.lock();
    if (chat_channel->Clients.find(kick_user_key)
      != chat_channel->Clients.end()) {
      chat_channel->Clients.erase(kick_user_key);
    }
    chat_channel->ClientsMutex.unlock();

    // Trigger events
    OnKickUserCompleted(kChannelMessageResult_Ok, chat_channel->Name,
      target, *chat_user);
    OnChannelUserKicked(*chat_channel, *kick_user);

    return true;
  } else if (message_type == kChannelMessageType_BanUser) {
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }

    std::string target;
    if (!buffer.ReadString(target)) {
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

    // Check if the user is logged in
    if (!chat_user->Identified) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotIdentified);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_BanUser_Complete, send_buffer);

      // Trigger events
      OnBanUserCompleted(kChannelMessageResult_NotIdentified, channel_name,
        target, *chat_user);

      return true;
    }

    // Check if the channel name is valid
    if (channel_name.empty() || channel_name[0] != '#') {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_BanUser_Complete, send_buffer);

      // Trigger events
      OnBanUserCompleted(kChannelMessageResult_InvalidChannelName,
        channel_name, target, *chat_user);

      return true;
    }

    // Check if the target is valid
    if (target.empty() || String::Contains(target, "#")) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidUsername);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_BanUser_Complete, send_buffer);

      // Trigger events
      OnBanUserCompleted(kChannelMessageResult_InvalidUsername,
        channel_name, target, *chat_user);

      return true;
    }

    // Check if the channel exists
    std::shared_ptr<ChatChannel> chat_channel;
    channels_mutex_.lock();
    for (auto &channel : channels_) {
      if (channel->Enabled && channel->Name == channel_name) {
        chat_channel = channel;
        break;
      }
    }
    channels_mutex_.unlock();

    if (!chat_channel) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_BanUser_Complete, send_buffer);

      // Trigger events
      OnBanUserCompleted(kChannelMessageResult_InvalidChannelName,
        channel_name, target, *chat_user);

      return true;
    }

    // Check if the user is in the channel
    chat_channel->ClientsMutex.lock();
    if (chat_channel->Clients.find(&client) == chat_channel->Clients.end()) {
      chat_channel->ClientsMutex.unlock();

      // Notify the client that they are not in the channel
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotInChannel);
      send_buffer.WriteString(chat_channel->Name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_BanUser_Complete, send_buffer);

      // Trigger events
      OnBanUserCompleted(kChannelMessageResult_NotInChannel, chat_channel->Name,
        target, *chat_user);

      return true;
    }
    chat_channel->ClientsMutex.unlock();

    // Check if the user has permissions
    chat_channel->OperatorsMutex.lock();
    if (chat_channel->Operators.find(&client)
      == chat_channel->Operators.end()) {
      chat_channel->OperatorsMutex.unlock();

      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotPermitted);
      send_buffer.WriteString(chat_channel->Name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_BanUser_Complete, send_buffer);

      // Trigger events
      OnBanUserCompleted(kChannelMessageResult_NotPermitted,
        chat_channel->Name, target, *chat_user);

      return true;
    }
    chat_channel->OperatorsMutex.unlock();

    // Check if the user is trying to ban themself
    if (target == chat_user->Username) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_CannotBanSelf);
      send_buffer.WriteString(chat_channel->Name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_BanUser_Complete, send_buffer);

      // Trigger events
      OnBanUserCompleted(kChannelMessageResult_CannotBanSelf,
        chat_channel->Name, target, *chat_user);

      return true;
    }

    // Check if the target is in the channel
    RemoteChatClient *ban_user_key = 0;
    std::shared_ptr<ChatUser> ban_user;
    std::string target_string;
    chat_channel->ClientsMutex.lock();
    for (auto pair : chat_channel->Clients) {
      if (pair.second->Username == target) {
        ban_user_key = pair.first;
        ban_user = pair.second;
        target_string = pair.second->Username + "@" + pair.second->Hostname;
        break;
      }
    }
    chat_channel->ClientsMutex.unlock();
    if (target_string.empty()) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidUsername);
      send_buffer.WriteString(channel_name);
      send_buffer.WriteString(target);
      server_->Send(client, kComponentType_Channel,
        kChannelMessageType_BanUser_Complete, send_buffer);

      // Trigger events
      OnBanUserCompleted(kChannelMessageResult_InvalidUsername,
        channel_name, target, *chat_user);

      return true;
    }

    // Check if the target is already banned
    chat_channel->BannedUsersMutex.lock();
    for (auto &banned_user : chat_channel->BannedUsers) {
      if (banned_user == target) {
        chat_channel->BannedUsersMutex.unlock();

        TypedBuffer send_buffer = server_->CreateBuffer();
        send_buffer.WriteUInt16(kChannelMessageResult_AlreadyBanned);
        send_buffer.WriteString(chat_channel->Name);
        send_buffer.WriteString(target);
        server_->Send(client, kComponentType_Channel,
          kChannelMessageType_BanUser_Complete, send_buffer);

        // Trigger events
        OnBanUserCompleted(kChannelMessageResult_AlreadyBanned,
          chat_channel->Name, target, *chat_user);

        return true;
      }
    }

    // Ban the user
    chat_channel->BannedUsers.push_back(target_string);
    chat_channel->BannedUsersMutex.unlock();

    // Notify other clients
    TypedBuffer clients_buffer = server_->CreateBuffer();
    clients_buffer.WriteUInt16(kChannelMessageResult_UserBanned);
    clients_buffer.WriteString(chat_channel->Name);
    clients_buffer.WriteString(ban_user->Username);
    clients_buffer.WriteString(ban_user->Hostname);

    chat_channel->ClientsMutex.lock();
    for (auto &pair : chat_channel->Clients) {
      if (pair.first != &client && pair.second->Enabled) {
        server_->Send(pair.first, kComponentType_Channel,
          kChannelMessageType_BanUser, clients_buffer);
      }
    }
    chat_channel->ClientsMutex.unlock();

    // Tell the client that the user was banned
    TypedBuffer send_buffer = server_->CreateBuffer();
    send_buffer.WriteUInt16(kChannelMessageResult_Ok);
    send_buffer.WriteString(chat_channel->Name);
    send_buffer.WriteString(target);
    send_buffer.WriteString(ban_user->Username);
    send_buffer.WriteString(ban_user->Hostname);
    server_->Send(client, kComponentType_Channel,
      kChannelMessageType_BanUser_Complete, send_buffer);

    // Remove the client from channel client lists
    chat_channel->OperatorsMutex.lock();
    if (chat_channel->Operators.find(ban_user_key)
      != chat_channel->Operators.end()) {
      chat_channel->Operators.erase(ban_user_key);
    }
    chat_channel->OperatorsMutex.unlock();

    chat_channel->ClientsMutex.lock();
    if (chat_channel->Clients.find(ban_user_key)
      != chat_channel->Clients.end()) {
      chat_channel->Clients.erase(ban_user_key);
    }
    chat_channel->ClientsMutex.unlock();

    // Trigger events
    OnBanUserCompleted(kChannelMessageResult_Ok, chat_channel->Name,
      target, *chat_user);
    OnChannelUserBanned(*chat_channel, *ban_user);

    return true;
  } else if (message_type == kChannelMessageType_UnbanUser) {
    // TODO: Implement
    return false;
  }

  return false;
}
}
