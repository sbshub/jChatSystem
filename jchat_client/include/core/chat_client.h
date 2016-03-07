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
#include "chat_handler.h"
#include "chat_channel.h"
#include "protocol/version.h"
#include "protocol/message_type.h"
#include "protocol/message_result.h"

namespace jchat {
class ChatClient {
  friend class ChatHandler;

  bool is_connected_;
  TcpClient tcp_client_;
  bool is_little_endian_;
  std::vector<ChatHandler *> handlers_;
  std::mutex handlers_mutex_;
  // NOTE: These are the channels that the client is in...
  std::vector<ChatChannel *> channels_;
  std::mutex channels_mutex_;

  // Internal events
  bool onConnected();
  bool onDisconnected();
  bool onDataReceived(Buffer &buffer);

  // Internal functions
  TypedBuffer createBuffer();

  // Send functions
  bool send(MessageType message_type, TypedBuffer &buffer);

  // Message functions
  bool sendMessageToClient(RemoteChatClient *target, std::string message);

public:
  ChatClient(const char *hostname, uint16_t port);
  ~ChatClient();

  bool Connect();
  bool Disconnect();

  bool AddHandler(ChatHandler *handler);
  bool RemoveHandler(ChatHandler *handler);


  Event<> OnConnected;
  Event<> OnDisconnected;
};
}

#endif // jchat_client_chat_client_h_
