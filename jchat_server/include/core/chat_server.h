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
#include "chat_channel.h"
#include "protocol/version.h"
#include "protocol/component_type.h"
#include "protocol/message_result.h"
#include "protocol/user/user_message_type.h"
#include <map>

namespace jchat {
class ChatServer {
  friend class ChatComponent;

  bool is_listening_;
  TcpServer tcp_server_;
  bool is_little_endian_;
  std::vector<ChatComponent *> components_;
  std::mutex components_mutex_;
  std::vector<ChatChannel *> channels_;
  std::mutex channels_mutex_;
  std::map<TcpClient *, RemoteChatClient *> clients_;
  std::mutex clients_mutex_;

  // Internal events
  bool onClientConnected(TcpClient &tcp_client);
  bool onClientDisconnected(TcpClient &tcp_client);
  bool onDataReceived(TcpClient &tcp_client, Buffer &buffer);

  // Internal functions
  TypedBuffer createBuffer();
  bool getTcpClient(RemoteChatClient &client, TcpClient *out_client);

  // Send functions
  bool sendUnicast(TcpClient &client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool sendUnicast(TcpClient *client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool sendMulticast(std::vector<TcpClient *> clients,
    ComponentType component_type, uint8_t message_type, TypedBuffer &buffer);
  bool sendUnicast(RemoteChatClient &client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool sendUnicast(RemoteChatClient *client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool sendMulticast(std::vector<RemoteChatClient *> clients,
    ComponentType component_type, uint8_t message_type, TypedBuffer &buffer);

public:
  ChatServer(const char *hostname, uint16_t port);
  ~ChatServer();

  bool Start();
  bool Stop();

  bool AddComponent(ChatComponent *component);
  bool RemoveComponent(ChatComponent *component);

  ChatComponent *GetComponent(ComponentType component_type);
  template<typename _TComponent>
  _TComponent *GetComponent(ComponentType component_type) {
    return reinterpret_cast<_TComponent *>(GetComponent(component_type));
  }

  IPEndpoint GetListenEndpoint();

  Event<RemoteChatClient &> OnClientConnected;
  Event<RemoteChatClient &> OnClientDisconnected;
};
}

#endif // jchat_server_chat_server_h_
