/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_server_channel_component_h_
#define jchat_server_channel_component_h_

#include "chat_component.h"
#include "chat_channel.h"
#include "protocol/components/channel_message_result.h"
#include "event.hpp"

namespace jchat {
class ChannelComponent : public ChatComponent {
private:
  ChatServer *server_;
  std::vector<std::shared_ptr<ChatChannel>> channels_;
  std::mutex channels_mutex_;

public:
  ChannelComponent();
  ~ChannelComponent();

  // Internal functions
  virtual bool Initialize(ChatServer &server) override;
  virtual bool Shutdown() override;

  virtual bool OnStart() override;
  virtual bool OnStop() override;

  // Internal events
  virtual void OnClientConnected(RemoteChatClient &client) override;
  virtual void OnClientDisconnected(RemoteChatClient &client) override;

  // Handler functions
  virtual ComponentType GetType() override;
  virtual bool Handle(RemoteChatClient &client, uint16_t message_type,
    TypedBuffer &buffer) override;

  // API functions


  // API events
  // NOTE: The last argument in these (ChatUser &) is always the source user
  Event<ChannelMessageResult, std::string &, ChatUser &> OnJoinCompleted;
  Event<ChannelMessageResult, std::string &, ChatUser &> OnLeaveCompleted;
  Event<ChannelMessageResult, std::string &, std::string &,
    ChatUser &> OnSendMessageCompleted;
  Event<ChannelMessageResult, std::string &, std::string &,
    ChatUser &> OnOpUserCompleted;
  Event<ChannelMessageResult, std::string &, std::string &,
    ChatUser &> OnDeopUserCompleted;
  Event<ChannelMessageResult, std::string &, std::string &,
    ChatUser &> OnKickUserCompleted;
  Event<ChannelMessageResult, std::string &, std::string &,
    ChatUser &> OnBanUserCompleted;
  Event<ChannelMessageResult, std::string &, std::string &,
    ChatUser &> OnUnbanUserCompleted;

  Event<ChatChannel &> OnChannelCreated;
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

#endif // jchat_server_channel_component_h_
