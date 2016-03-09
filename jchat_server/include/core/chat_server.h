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
#include <map>

namespace jchat {
class ChatServer {
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
  bool getTcpClient(RemoteChatClient &client, TcpClient *out_client);

  // Send functions
  bool sendUnicast(TcpClient &client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool sendUnicast(TcpClient *client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool sendMulticast(std::vector<TcpClient *> clients,
    ComponentType component_type, uint8_t message_type, TypedBuffer &buffer);

public:
  ChatServer(const char *hostname, uint16_t port);
  ~ChatServer();

  bool Start();
  bool Stop();

  bool AddComponent(ChatComponent *component);
  bool RemoveComponent(ChatComponent *component);

  bool GetComponent(ComponentType component_type, ChatComponent *out_component);
  template<typename _TComponent>
  bool GetComponent(ComponentType component_type, _TComponent *out_component) {
     return GetComponent(component_type,
       reinterpret_cast<ChatComponent *>(out_component));
  }
  
  TypedBuffer CreateBuffer();
  bool SendUnicast(RemoteChatClient &client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool SendUnicast(RemoteChatClient *client, ComponentType component_type,
    uint8_t message_type, TypedBuffer &buffer);
  bool SendMulticast(std::vector<RemoteChatClient *> clients,
    ComponentType component_type, uint8_t message_type, TypedBuffer &buffer);

  IPEndpoint GetListenEndpoint();

  Event<RemoteChatClient &> OnClientConnected;
  Event<RemoteChatClient &> OnClientDisconnected;
};
}

#endif // jchat_server_chat_server_h_
