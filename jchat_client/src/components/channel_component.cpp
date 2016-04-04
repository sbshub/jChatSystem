/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/channel_component.h"
#include "components/user_component.h"
#include "chat_client.h"
#include "protocol/components/channel_message_type.h"

namespace jchat {
ChannelComponent::ChannelComponent() {
}

ChannelComponent::~ChannelComponent() {
  // Remove channels
  if (!channels_.empty()) {
    channels_.clear();
  }
}

bool ChannelComponent::Initialize(ChatClient &client) {
  client_ = &client;
  return true;
}

bool ChannelComponent::Shutdown() {
  client_ = 0;

  // Remove channels
  if (!channels_.empty()) {
    channels_.clear();
  }

  return true;
}

void ChannelComponent::OnConnected() {

}

void ChannelComponent::OnDisconnected() {
  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    channels_.clear();
  }
  channels_mutex_.unlock();
}

ComponentType ChannelComponent::GetType() {
  return kComponentType_Channel;
}

bool ChannelComponent::Handle(uint16_t message_type, TypedBuffer &buffer) {
  if (message_type == kChannelMessageType_JoinChannel_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }
    OnJoinCompleted(static_cast<ChannelMessageResult>(message_result),
      channel_name);
    if (message_result != kChannelMessageResult_Ok
      && message_result != kChannelMessageResult_ChannelCreated) {
      return true;
    }

    // Create the ChatChannel and do necessary actions
    auto chat_channel = std::make_shared<ChatChannel>();
    chat_channel->Name = channel_name;
    chat_channel->Enabled = true;

    // Add the channel to the channel list
    channels_mutex_.lock();
    channels_.push_back(chat_channel);
    channels_mutex_.unlock();

    // Get user component
    std::shared_ptr<UserComponent> user_component;
    if (!client_->GetComponent(kComponentType_User, user_component)) {
      // Internal error, disconnect client
      return false;
    }

    // Get the chat client
    std::shared_ptr<ChatUser> chat_user;
    if (!user_component->GetChatUser(chat_user)) {
      // Internal error, disconnect client
      return false;
    }

    // Add the local user
    chat_channel->ClientsMutex.lock();
    chat_channel->Clients.push_back(chat_user);
    chat_channel->ClientsMutex.unlock();

    if (message_result == kChannelMessageResult_ChannelCreated) {
      chat_channel->OperatorsMutex.lock();
      chat_channel->Operators.push_back(chat_user);
      chat_channel->OperatorsMutex.unlock();

      OnChannelCreated(*chat_channel, *chat_user);
      OnChannelJoined(*chat_channel, *chat_user);

      return true;
    }

    uint64_t users_count = 0;
    if (!buffer.ReadUInt64(users_count)) {
      return false;
    }

    for (size_t i = 0; i < users_count; i++) {
      auto user = std::make_shared<ChatUser>();
      user->Enabled = true;
      user->Identified = true;

      if (!buffer.ReadString(user->Username)) {
        return false;
      }
      if (!buffer.ReadString(user->Hostname)) {
        return false;
      }
      bool is_operator = false;
      if (!buffer.ReadBoolean(is_operator)) {
        return false;
      }

      chat_channel->ClientsMutex.lock();
      chat_channel->Clients.push_back(user);
      chat_channel->ClientsMutex.unlock();
      if (is_operator) {
        chat_channel->OperatorsMutex.lock();
        chat_channel->Operators.push_back(user);
        chat_channel->OperatorsMutex.unlock();
      }
    }

    // Read bans
    uint64_t bans_count = 0;
    if (!buffer.ReadUInt64(bans_count)) {
      return false;
    }

    chat_channel->BannedUsersMutex.lock();
    for (size_t i = 0; i < bans_count; i++) {
      std::string banned_user;
      if (!buffer.ReadString(banned_user)) {
        chat_channel->BannedUsersMutex.unlock();
        return false;
      }
      chat_channel->BannedUsers.push_back(banned_user);
    }
    chat_channel->BannedUsersMutex.unlock();

    // Trigger events
    OnChannelJoined(*chat_channel, *chat_user);

    return true;
  } else if (message_type == kChannelMessageType_LeaveChannel_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }
    OnLeaveCompleted(static_cast<ChannelMessageResult>(message_result),
      channel_name);
    if (message_result != kChannelMessageResult_Ok
      && message_result != kChannelMessageResult_ChannelDestroyed) {
      return true;
    }

    // Get user component
    std::shared_ptr<UserComponent> user_component;
    if (!client_->GetComponent(kComponentType_User, user_component)) {
      // Internal error, disconnect client
      return false;
    }

    // Get the chat client
    std::shared_ptr<ChatUser> chat_user;
    if (!user_component->GetChatUser(chat_user)) {
      // Internal error, disconnect client
      return false;
    }

    // Remove the ChatChannel and do necessary actions
    channels_mutex_.lock();
    for (auto it = channels_.begin(); it != channels_.end(); ++it) {
      std::shared_ptr<ChatChannel> &chat_channel = *it;
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        OnChannelLeft(*chat_channel, *chat_user);

        // Disable the channel
        chat_channel->Enabled = false;

        // Clear all information
        chat_channel->OperatorsMutex.lock();
        chat_channel->Operators.clear();
        chat_channel->OperatorsMutex.unlock();

        chat_channel->ClientsMutex.lock();
        chat_channel->Clients.clear();
        chat_channel->ClientsMutex.unlock();

        chat_channel->BannedUsersMutex.lock();
        chat_channel->BannedUsers.clear();
        chat_channel->BannedUsersMutex.unlock();

        // Remove the channel
        channels_.erase(it);
        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_SendMessage_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }
    std::string message;
    if (!buffer.ReadString(message)) {
      return false;
    }
    OnSendMessageCompleted(static_cast<ChannelMessageResult>(message_result),
      channel_name, message);

    // Get user component
    std::shared_ptr<UserComponent> user_component;
    if (!client_->GetComponent(kComponentType_User, user_component)) {
      // Internal error, disconnect client
      return false;
    }

    // Get the chat client
    std::shared_ptr<ChatUser> chat_user;
    if (!user_component->GetChatUser(chat_user)) {
      // Internal error, disconnect client
      return false;
    }

    channels_mutex_.lock();
    for (auto it = channels_.begin(); it != channels_.end(); ++it) {
      std::shared_ptr<ChatChannel> &chat_channel = *it;
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        OnChannelMessage(*chat_channel, *chat_user, message);
        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_OpUser_Complete) {
    // TODO: Implement

    return true;
  } else if (message_type == kChannelMessageType_DeopUser_Complete) {
    // TODO: Implement

    return true;
  } else if (message_type == kChannelMessageType_KickUser_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }
    std::string target;
    if (!buffer.ReadString(target)) {
      return false;
    }
    OnKickUserCompleted(static_cast<ChannelMessageResult>(message_result),
      channel_name, target);

    if (message_result != kChannelMessageResult_Ok) {
      return true;
    }

    std::string username;
    if (!buffer.ReadString(username)) {
      return false;
    }
    std::string hostname;
    if (!buffer.ReadString(hostname)) {
      return false;
    }

    channels_mutex_.lock();
    for (auto it = channels_.begin(); it != channels_.end(); ++it) {
      std::shared_ptr<ChatChannel> &chat_channel = *it;
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        chat_channel->ClientsMutex.lock();
        for (auto it = chat_channel->Clients.begin();
          it != chat_channel->Clients.end(); ++it) {
          std::shared_ptr<ChatUser> &chat_user = *it;
          if (chat_user->Username == username
            && chat_user->Hostname == hostname) {
            OnChannelUserKicked(*chat_channel, *chat_user);
            chat_channel->Clients.erase(it);
            break;
          }
        }
        chat_channel->ClientsMutex.unlock();

        chat_channel->OperatorsMutex.lock();
        for (auto it = chat_channel->Operators.begin();
          it != chat_channel->Operators.end(); ++it) {
          std::shared_ptr<ChatUser> &chat_user = *it;
          if (chat_user->Username == username
            && chat_user->Hostname == hostname) {
            chat_channel->Operators.erase(it);
            break;
          }
        }
        chat_channel->OperatorsMutex.unlock();

        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_BanUser_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }
    std::string target;
    if (!buffer.ReadString(target)) {
      return false;
    }
    OnBanUserCompleted(static_cast<ChannelMessageResult>(message_result),
      channel_name, target);

    if (message_result != kChannelMessageResult_Ok) {
      return true;
    }

    std::string username;
    if (!buffer.ReadString(username)) {
      return false;
    }
    std::string hostname;
    if (!buffer.ReadString(hostname)) {
      return false;
    }

    channels_mutex_.lock();
    for (auto it = channels_.begin(); it != channels_.end(); ++it) {
      std::shared_ptr<ChatChannel> &chat_channel = *it;
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        chat_channel->ClientsMutex.lock();
        for (auto it = chat_channel->Clients.begin();
          it != chat_channel->Clients.end(); ++it) {
          std::shared_ptr<ChatUser> &chat_user = *it;
          if (chat_user->Username == username
            && chat_user->Hostname == hostname) {
            OnChannelUserBanned(*chat_channel, *chat_user);
            chat_channel->Clients.erase(it);
            break;
          }
        }
        chat_channel->ClientsMutex.unlock();

        chat_channel->OperatorsMutex.lock();
        for (auto it = chat_channel->Operators.begin();
          it != chat_channel->Operators.end(); ++it) {
          std::shared_ptr<ChatUser> &chat_user = *it;
          if (chat_user->Username == username
            && chat_user->Hostname == hostname) {
            chat_channel->Operators.erase(it);
            break;
          }
        }
        chat_channel->OperatorsMutex.unlock();

        chat_channel->BannedUsersMutex.lock();
        chat_channel->BannedUsers.push_back(username + "@" + hostname);
        chat_channel->BannedUsersMutex.unlock();

        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_UnbanUser_Complete) {
    // TODO: Implement

    return true;
  } else if (message_type == kChannelMessageType_JoinChannel) {
    // Read buffer
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }

    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
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

    if (message_result != kChannelMessageResult_UserJoined) {
      return false;
    }

    // Find the channel and add the user
    channels_mutex_.lock();
    for (auto &chat_channel : channels_) {
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        // Create ChatUser
        auto user = std::make_shared<ChatUser>();
        user->Enabled = true;
        user->Identified = true;
        user->Username = username;
        user->Hostname = hostname;

        chat_channel->ClientsMutex.lock();
        chat_channel->Clients.push_back(user);
        chat_channel->ClientsMutex.unlock();

        // Trigger events
        OnChannelJoined(*chat_channel, *user);

        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_LeaveChannel) {
    // Read buffer
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }

    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
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

    if (message_result != kChannelMessageResult_UserLeft) {
      return false;
    }

    // Find the channel and remove the user
    channels_mutex_.lock();
    for (auto &chat_channel : channels_) {
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        // Remove from clients
        chat_channel->ClientsMutex.lock();
        for (auto it = chat_channel->Clients.begin();
          it != chat_channel->Clients.end(); ++it) {
          std::shared_ptr<ChatUser> &user = *it;
          if (user->Username == username && user->Hostname == hostname) {
            // Trigger events
            OnChannelLeft(*chat_channel, *user);
            chat_channel->Clients.erase(it);
            break;
          }
        }
        chat_channel->ClientsMutex.unlock();

        // Remove from operators
        chat_channel->OperatorsMutex.lock();
        for (auto it = chat_channel->Operators.begin();
          it != chat_channel->Operators.end(); ++it) {
          std::shared_ptr<ChatUser> &user = *it;
          if (user->Username == username && user->Hostname == hostname) {
            chat_channel->Operators.erase(it);
            break;
          }
        }
        chat_channel->OperatorsMutex.unlock();

        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_SendMessage) {
    // Read buffer
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }

    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
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

    if (message_result != kChannelMessageResult_MessageSent) {
      return false;
    }

    // Find the channel
    channels_mutex_.lock();
    for (auto &chat_channel : channels_) {
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        // Find the user
        chat_channel->ClientsMutex.lock();
        for (auto it = chat_channel->Clients.begin();
          it != chat_channel->Clients.end(); ++it) {
          std::shared_ptr<ChatUser> &user = *it;
          if (user->Username == username && user->Hostname == hostname) {
            // Trigger events
            OnChannelMessage(*chat_channel, *user, message);
            break;
          }
        }
        chat_channel->ClientsMutex.unlock();
        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_OpUser) {
    // TODO: Implement

    return true;
  } else if (message_type == kChannelMessageType_DeopUser) {
    // TODO: Implement

    return true;
  } else if (message_type == kChannelMessageType_KickUser) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
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

    if (message_result != kChannelMessageResult_UserKicked) {
      return false;
    }

    channels_mutex_.lock();
    for (auto it = channels_.begin(); it != channels_.end(); ++it) {
      std::shared_ptr<ChatChannel> &chat_channel = *it;
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        chat_channel->ClientsMutex.lock();
        for (auto it = chat_channel->Clients.begin();
          it != chat_channel->Clients.end(); ++it) {
          std::shared_ptr<ChatUser> &chat_user = *it;
          if (chat_user->Username == username
            && chat_user->Hostname == hostname) {
            OnChannelUserKicked(*chat_channel, *chat_user);
            chat_channel->Clients.erase(it);
            break;
          }
        }
        chat_channel->ClientsMutex.unlock();

        chat_channel->OperatorsMutex.lock();
        for (auto it = chat_channel->Operators.begin();
          it != chat_channel->Operators.end(); ++it) {
          std::shared_ptr<ChatUser> &chat_user = *it;
          if (chat_user->Username == username
            && chat_user->Hostname == hostname) {
            chat_channel->Operators.erase(it);
            break;
          }
        }
        chat_channel->OperatorsMutex.unlock();

        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_BanUser) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
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

    if (message_result != kChannelMessageResult_UserBanned) {
      return false;
    }

    channels_mutex_.lock();
    for (auto it = channels_.begin(); it != channels_.end(); ++it) {
      std::shared_ptr<ChatChannel> &chat_channel = *it;
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        chat_channel->ClientsMutex.lock();
        for (auto it = chat_channel->Clients.begin();
          it != chat_channel->Clients.end(); ++it) {
          std::shared_ptr<ChatUser> &chat_user = *it;
          if (chat_user->Username == username
            && chat_user->Hostname == hostname) {
            OnChannelUserBanned(*chat_channel, *chat_user);
            chat_channel->Clients.erase(it);
            break;
          }
        }
        chat_channel->ClientsMutex.unlock();

        chat_channel->OperatorsMutex.lock();
        for (auto it = chat_channel->Operators.begin();
          it != chat_channel->Operators.end(); ++it) {
          std::shared_ptr<ChatUser> &chat_user = *it;
          if (chat_user->Username == username
            && chat_user->Hostname == hostname) {
            chat_channel->Operators.erase(it);
            break;
          }
        }
        chat_channel->OperatorsMutex.unlock();

        chat_channel->BannedUsersMutex.lock();
        chat_channel->BannedUsers.push_back(username + "@" + hostname);
        chat_channel->BannedUsersMutex.unlock();

        break;
      }
    }
    channels_mutex_.unlock();

    return true;
  } else if (message_type == kChannelMessageType_UnbanUser) {
    // TODO: Implement

    return true;
  }

  return false;
}

bool ChannelComponent::JoinChannel(std::string channel_name) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  return client_->Send(kComponentType_Channel, kChannelMessageType_JoinChannel,
    buffer);
}

bool ChannelComponent::LeaveChannel(std::string channel_name) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  return client_->Send(kComponentType_Channel, kChannelMessageType_LeaveChannel,
    buffer);
}

bool ChannelComponent::SendMessage(std::string channel_name,
  std::string message) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  buffer.WriteString(message);
  return client_->Send(kComponentType_Channel, kChannelMessageType_SendMessage,
    buffer);
}

bool ChannelComponent::OpUser(std::string channel_name, std::string username) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  buffer.WriteString(username);
  return client_->Send(kComponentType_Channel, kChannelMessageType_OpUser,
    buffer);
}

bool ChannelComponent::DeopUser(std::string channel_name,
  std::string username) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  buffer.WriteString(username);
  return client_->Send(kComponentType_Channel, kChannelMessageType_DeopUser,
    buffer);
}

bool ChannelComponent::KickUser(std::string channel_name,
  std::string username) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  buffer.WriteString(username);
  return client_->Send(kComponentType_Channel, kChannelMessageType_KickUser,
    buffer);
}

bool ChannelComponent::BanUser(std::string channel_name,
  std::string username) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  buffer.WriteString(username);
  return client_->Send(kComponentType_Channel, kChannelMessageType_BanUser,
    buffer);
}

bool ChannelComponent::UnbanUser(std::string channel_name,
  std::string username) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  buffer.WriteString(username);
  return client_->Send(kComponentType_Channel, kChannelMessageType_UnbanUser,
    buffer);
}
}
