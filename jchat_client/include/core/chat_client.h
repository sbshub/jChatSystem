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
#include "protocol/component_type.h"
#include "protocol/message_result.h"
#include "protocol/user/user_message_type.h"

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

  // User functions
  bool userIdentify(std::string &username);

  // Message functions
  bool messageSendToUser(RemoteChatClient *target, std::string message);
  bool messageSendToUser(std::string &target, std::string message);

  // Channel functions


public:
  ChatClient(const char *hostname, uint16_t port);
  ~ChatClient();

  bool Connect();
  bool Disconnect();

  bool AddHandler(ChatHandler *handler);
  bool RemoveHandler(ChatHandler *handler);

  IPEndpoint GetLocalEndpoint();
  IPEndpoint GetRemoteEndpoint();

  // Alias functions
  // User functions
  bool UserIdentify(std::string &username);

  // Message functions
  bool MessageSendToUser(std::string target, std::string message);

  Event<> OnConnected;
  Event<> OnDisconnected;
  Event<MessageResult &> OnIdenfityCompleted;
};
}

#endif // jchat_client_chat_client_h_
