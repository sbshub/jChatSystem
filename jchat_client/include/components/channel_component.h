/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_client_channel_component_h_
#define jchat_client_channel_component_h_

#include "chat_component.h"
#include "chat_channel.h"
#include "protocol/components/channel_message_result.h"
#include "event.hpp"

namespace jchat {
class ChannelComponent : public ChatComponent {
private:
  ChatClient *client_;
  std::vector<std::shared_ptr<ChatChannel>> channels_;
  std::mutex channels_mutex_;

public:
  ChannelComponent();
  ~ChannelComponent();

  // Internal functions
  virtual bool Initialize(ChatClient &client) override;
  virtual bool Shutdown() override;

  // Internal events
  virtual void OnConnected() override;
  virtual void OnDisconnected() override;

  // Handler functions
  virtual ComponentType GetType() override;
  virtual bool Handle(uint16_t message_type, TypedBuffer &buffer) override;

  // API functions
  bool JoinChannel(std::string channel_name);
  bool LeaveChannel(std::string channel_name);
  bool SendMessage(std::string channel_name, std::string message);
  bool OpUser(std::string channel_name, std::string username);
  bool DeopUser(std::string channel_name, std::string username);
  bool KickUser(std::string channel_name, std::string username);
  bool BanUser(std::string channel_name, std::string username);
  bool UnbanUser(std::string channel_name, std::string username);

  // API events
  Event<ChannelMessageResult, std::string &> OnJoinCompleted;
  Event<ChannelMessageResult, std::string &> OnLeaveCompleted;
  Event<ChannelMessageResult, std::string &,
    std::string &> OnSendMessageCompleted;
  Event<ChannelMessageResult, std::string &, std::string &> OnOpUserCompleted;
  Event<ChannelMessageResult, std::string &, std::string &> OnDeopUserCompleted;
  Event<ChannelMessageResult, std::string &, std::string &> OnKickUserCompleted;
  Event<ChannelMessageResult, std::string &, std::string &> OnBanUserCompleted;
  Event<ChannelMessageResult, std::string &,
    std::string &> OnUnbanUserCompleted;

  Event<ChatChannel &, ChatUser &> OnChannelCreated;
  Event<ChatChannel &, ChatUser &> OnChannelJoined;
  Event<ChatChannel &, ChatUser &> OnChannelLeft;
  Event<ChatChannel &, ChatUser &, std::string &> OnChannelMessage;
  Event<ChatChannel &, ChatUser &> OnChannelUserOpped;
  Event<ChatChannel &, ChatUser &> OnChannelUserDeopped;
  Event<ChatChannel &, ChatUser &> OnChannelUserKicked;
  Event<ChatChannel &, ChatUser &> OnChannelUserBanned;
  Event<ChatChannel &, std::string &, std::string &> OnChannelUserUnbanned;
};
}

#endif // jchat_client_channel_component_h_
