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
#include "chat_handler.h"
#include "chat_channel.h"
#include "protocol/version.h"
#include "protocol/message_type.h"
#include "protocol/message_result.h"
#include <map>

namespace jchat {
class ChatServer {
  friend class ChatHandler;

  bool is_listening_;
  TcpServer tcp_server_;
  bool is_little_endian_;
  std::vector<ChatHandler *> handlers_;
  std::mutex handlers_mutex_;
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
  bool sendUnicast(TcpClient &client, MessageType message_type,
    TypedBuffer &buffer);
  bool sendUnicast(TcpClient *client, MessageType message_type,
      TypedBuffer &buffer);
  bool sendMulticast(std::vector<TcpClient *> clients, MessageType message_type,
    TypedBuffer &buffer);
  bool sendUnicast(RemoteChatClient &client, MessageType message_type,
    TypedBuffer &buffer);
  bool sendUnicast(RemoteChatClient *client, MessageType message_type,
    TypedBuffer &buffer);
  bool sendMulticast(std::vector<RemoteChatClient *> clients,
    MessageType message_type, TypedBuffer &buffer);

  // Channel functions
  // TODO: Create channel, Destroy channel, etc, etc...
  bool addChannelOperator(ChatChannel *channel, RemoteChatClient *client);
  bool addChannelClient(ChatChannel *channel, RemoteChatClient *client);
  bool removeChannelOperator(ChatChannel *channel, RemoteChatClient *client);
  bool removeChannelClient(ChatChannel *channel, RemoteChatClient *client);

public:
  ChatServer(const char *hostname, uint16_t port);
  ~ChatServer();

  bool Start();
  bool Stop();

  bool AddHandler(ChatHandler *handler);
  bool RemoveHandler(ChatHandler *handler);

  // Message functions
  bool SendMessageToClient(RemoteChatClient *source, RemoteChatClient *target,
    std::string message);

  Event<RemoteChatClient &> OnClientConnected;
  Event<RemoteChatClient &> OnClientDisconnected;
};
}

#endif // jchat_server_chat_server_h_
