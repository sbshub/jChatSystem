/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_server_user_component_h_
#define jchat_server_user_component_h_

#include "chat_component.h"
#include "chat_user.h"
#include "protocol/components/user_message_result.h"
#include "event.hpp"
#include <map>
#include <memory>

namespace jchat {
class UserComponent : public ChatComponent {
private:
  ChatServer *server_;
  std::map<RemoteChatClient *, std::shared_ptr<ChatUser>> users_;
  std::mutex users_mutex_;

public:
  UserComponent();
  ~UserComponent();

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
  bool GetChatUser(RemoteChatClient &client,
    std::shared_ptr<ChatUser> &out_user);

  // API events

};
}

#endif // jchat_server_user_component_h_
