/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_client_user_component_h_
#define jchat_client_user_component_h_

#include "chat_component.h"
#include "chat_user.h"
#include "protocol/components/user_message_result.h"
#include "event.hpp"
#include <memory>
#include <map>

namespace jchat {
class UserComponent : public ChatComponent {
private:
  ChatClient *client_;

  // Local user
  std::shared_ptr<ChatUser> user_;

public:
  UserComponent();
  ~UserComponent();

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
  bool GetChatUser(std::shared_ptr<ChatUser> &out_user);

  bool Identify(std::string username);
  bool SendMessage(std::string username, std::string message);

  // API events
  Event<UserMessageResult, std::string &> OnIdentifyCompleted;
  Event<UserMessageResult, std::string &, std::string &> OnSendMessageCompleted;

  Event<> OnIdentified;
  Event<std::string &, std::string &, std::string &, std::string &> OnMessage;
};
}

#endif // jchat_client_channel_component_h_
