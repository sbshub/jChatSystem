/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_server_chat_server_h_
#define jchat_server_chat_server_h_

// Required libraries
#include "tcp_server.hpp"
#include "remote_chat_client.h"
#include "chat_component.h"
#include "protocol/protocol.h"
#include "protocol/component_type.h"
#include <map>

namespace jchat {
class ChatServer {
  bool is_listening_;
  TcpServer tcp_server_;
  bool is_little_endian_;
  std::vector<std::shared_ptr<ChatComponent>> components_;
  std::map<TcpClient *, RemoteChatClient *> clients_;
  std::mutex clients_mutex_;

  // Internal events
  bool onClientConnected(TcpClient &tcp_client);
  bool onClientDisconnected(TcpClient &tcp_client);
  bool onDataReceived(TcpClient &tcp_client, Buffer &buffer);

  // Internal functions
  bool getTcpClient(RemoteChatClient &client, TcpClient **out_client);

  // Send functions
  bool send(TcpClient &client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool send(TcpClient *client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);

public:
  ChatServer(const char *hostname, uint16_t port);
  ~ChatServer();

  bool Start();
  bool Stop();

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
  bool Send(RemoteChatClient &client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool Send(RemoteChatClient *client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);

  IPEndpoint GetListenEndpoint();

  Event<RemoteChatClient &> OnClientConnected;
  Event<RemoteChatClient &> OnClientDisconnected;
};
}

#endif // jchat_server_chat_server_h_
