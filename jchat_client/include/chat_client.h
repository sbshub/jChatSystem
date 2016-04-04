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
#include "chat_component.h"
#include "chat_channel.h"
#include "protocol/protocol.h"
#include "protocol/component_type.h"

namespace jchat {
class ChatClient {
  bool is_connected_;
  TcpClient tcp_client_;
  bool is_little_endian_;
  std::vector<std::shared_ptr<ChatComponent>> components_;

  // Internal events
  bool onConnected();
  bool onDisconnected();
  bool onDataReceived(Buffer &buffer);

public:
  ChatClient(const char *hostname, uint16_t port);
  ~ChatClient();

  bool Connect();
  bool Disconnect();

  bool AddComponent(std::shared_ptr<ChatComponent> component);
  bool RemoveComponent(std::shared_ptr<ChatComponent> component);

  bool GetComponent(ComponentType component_type,
    std::shared_ptr<ChatComponent> &out_component);
  template<typename _TComponent>
  bool GetComponent(ComponentType component_type,
    std::shared_ptr<_TComponent> &out_component) {
     return GetComponent(component_type,
       reinterpret_cast<std::shared_ptr<ChatComponent> &>(out_component));
  }

  TypedBuffer CreateBuffer();
  bool Send(ComponentType component_type, uint8_t message_type,
    TypedBuffer &buffer);

  IPEndpoint GetLocalEndpoint();
  IPEndpoint GetRemoteEndpoint();

  Event<> OnConnected;
  Event<> OnDisconnected;
};
}

#endif // jchat_client_chat_client_h_
