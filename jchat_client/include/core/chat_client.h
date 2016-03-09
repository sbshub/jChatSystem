/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_client_chat_client_h_
#define jchat_client_chat_client_h_

// Required libraries
#include "tcp_client.hpp"
#include "remote_chat_client.h"
#include "chat_component.h"
#include "chat_channel.h"
#include "protocol/version.h"
#include "protocol/component_type.h"
#include "protocol/message_result.h"
#include "protocol/user/user_message_type.h"

namespace jchat {
class ChatClient {
  friend class ChatComponent;

  bool is_connected_;
  TcpClient tcp_client_;
  bool is_little_endian_;
  std::vector<ChatComponent *> components_;
  std::mutex components_mutex_;
  // NOTE: These are the channels that the client is in...
  std::vector<ChatChannel *> channels_;
  std::mutex channels_mutex_;
  RemoteChatClient chat_client_;

  // Internal events
  bool onConnected();
  bool onDisconnected();
  bool onDataReceived(Buffer &buffer);

  // Internal functions
  TypedBuffer createBuffer();

  // Send functions
  bool send(ComponentType component_type, uint8_t message_type,
    TypedBuffer &buffer);

public:
  ChatClient(const char *hostname, uint16_t port);
  ~ChatClient();

  bool Connect();
  bool Disconnect();

  bool AddHandler(ChatComponent *component);
  bool RemoveHandler(ChatComponent *component);

  ChatComponent *GetComponent(ComponentType component_type);
  template<typename _TComponent>
  _TComponent *GetComponent(ComponentType component_type) {
    return reinterpret_cast<_TComponent *>(GetComponent(component_type));
  }

  IPEndpoint GetLocalEndpoint();
  IPEndpoint GetRemoteEndpoint();

  Event<> OnConnected;
  Event<> OnDisconnected;
};
}

#endif // jchat_client_chat_client_h_
