/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "core/chat_client.h"

namespace jchat {
ChatClient::ChatClient(const char *hostname, uint16_t port)
  : tcp_client_(hostname, port), is_connected_(false) {
  int16_t number = 0x00FF;
  is_little_endian_ = ((uint8_t *)&number)[0] == 0xFF;

  tcp_client_.OnConnected.Add([this]() {
    return onClientConnected();
  });
  tcp_server_.OnDisconnected.Add([this]() {
    return onClientDisconnected();
  });
  tcp_server_.OnDataReceived.Add([this](Buffer &buffer) {
    return onDataReceived(buffer);
  });
}

ChatClient::~ChatClient() {
  // TODO: Write
}

bool ChatClient::Connect() {
  // TODO: Write
  return false;
}

bool ChatClient::Disconnect() {
  // TODO: Write
  return false;
}

bool ChatClient::AddHandler(ChatHandler *handler) {
  // TODO: Write
  return false;
}

bool ChatClient::RemoveHandler(ChatHandler *handler) {
  // TODO: Write
  return false;
}

bool ChatClient::SendMessageToClient(RemoteChatClient *target,
  std::string message) {
    // TODO: Write
    return false;
}

bool ChatClient::onConnected() {
  // TODO: Write
  return false;
}

bool ChatClient::onDisconnected() {
  // TODO: Write
  return false;
}

bool ChatClient::onDataReceived(Buffer &buffer) {
  // TODO: Write
  return false;
}

TypedBuffer ChatClient::createBuffer() {
  return TypedBuffer(!is_little_endian_);
}

bool ChatClient::send(MessageType message_type, TypedBuffer &buffer) {
  // TODO: Write
  return false;
}
}
