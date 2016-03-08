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
    return onConnected();
  });
  tcp_client_.OnDisconnected.Add([this]() {
    return onDisconnected();
  });
  tcp_client_.OnDataReceived.Add([this](Buffer &buffer) {
    return onDataReceived(buffer);
  });
}

ChatClient::~ChatClient() {
  // Remove channels
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }

  // Remove handlers
  if (!handlers_.empty()) {
    for (auto handler : handlers_) {
      delete handler;
    }
    handlers_.clear();
  }
}

bool ChatClient::Connect() {
  if (is_connected_) {
    return false;
  }

  if (!tcp_client_.Connect()) {
    return false;
  }

  is_connected_ = true;

  return true;
}

bool ChatClient::Disconnect() {
  if (!is_connected_) {
    return false;
  }

  if (!tcp_client_.Disconnect()) {
    return false;
  }

  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
  channels_mutex_.unlock();

  is_connected_ = false;

  return true;
}

bool ChatClient::AddHandler(ChatHandler *handler) {
  handlers_mutex_.lock();
  for (auto it = handlers_.begin(); it != handlers_.end();) {
    if (*it == handler) {
      handlers_mutex_.unlock();
      return false;
    }
  }
  if (!handler->Initialize(*this)) {
    handlers_mutex_.unlock();
    return false;
  }
  handlers_.push_back(handler);
  handlers_mutex_.unlock();

  return true;
}

bool ChatClient::RemoveHandler(ChatHandler *handler) {
  handlers_mutex_.lock();
  for (auto it = handlers_.begin(); it != handlers_.end();) {
    if (*it == handler) {
      if (!handler->Shutdown(*this)) {
        return false;
      }
      handlers_.erase(it);
      handlers_mutex_.unlock();
      return true;
    }
  }
  handlers_mutex_.unlock();

  return false;
}

IPEndpoint ChatClient::GetLocalEndpoint() {
  return tcp_client_.GetLocalEndpoint();
}

IPEndpoint ChatClient::GetRemoteEndpoint() {
  return tcp_client_.GetRemoteEndpoint();
}

bool ChatClient::UserIdentify(std::string &username) {
  return userIdentify(username);
}

bool ChatClient::MessageSendToUser(std::string target, std::string message) {
  return messageSendToUser(target, message);
}

bool ChatClient::onConnected() {
  handlers_mutex_.lock();
  for (auto handler : handlers_) {
    handler->OnConnected(*this);
  }
  handlers_mutex_.unlock();

  return OnConnected();
}

bool ChatClient::onDisconnected() {
  handlers_mutex_.lock();
  for (auto handler : handlers_) {
    handler->OnDisconnected(*this);
  }
  handlers_mutex_.unlock();

  return OnDisconnected();
}

bool ChatClient::onDataReceived(Buffer &buffer) {
  uint8_t component_type = 0;
  uint16_t message_type = 0;
  uint32_t size = 0;

  // Flip data endian order if needed
  buffer.SetFlipEndian(!is_little_endian_);

  // Check if the packet is valid
  if (!buffer.Read(&component_type) || !buffer.Read(&message_type)
    || !buffer.Read(&size) || buffer.GetSize() - buffer.GetPosition() != size
    || component_type >= kComponentType_Max) {
    // Drop connection
    return false;
  }

  // Read the packet into a typed buffer
  TypedBuffer typed_buffer(buffer.GetBuffer() + buffer.GetPosition(),
    size, !is_little_endian_);

  // Try to handle the request, if it is unhandled, drop the connection
  bool handled = false;
  handlers_mutex_.lock();
  for (auto handler : handlers_) {
    if (handler->GetType() == static_cast<ComponentType>(component_type)) {
      if (handler->Handle(*this, message_type, typed_buffer)) {
        handled = true;
      }
    }
  }
  handlers_mutex_.unlock();

  return handled;
}

TypedBuffer ChatClient::createBuffer() {
  return TypedBuffer(!is_little_endian_);
}

bool ChatClient::send(ComponentType component_type, uint8_t message_type,
  TypedBuffer &buffer) {
  Buffer temp_buffer(!is_little_endian_);

  // Write header
  temp_buffer.Write<uint8_t>(component_type);
  temp_buffer.Write<uint16_t>(message_type);
  temp_buffer.Write<uint32_t>(buffer.GetSize());

  // Write body
  temp_buffer.WriteArray<uint8_t>(buffer.GetBuffer(), buffer.GetSize());

  return tcp_client_.Send(temp_buffer);
}

bool ChatClient::userIdentify(std::string &username) {
  TypedBuffer buffer = createBuffer();
  buffer.WriteUInt16(kMessageResult_Succeeded); // Result
  buffer.WriteString(username); // Target username
  return send(kComponentType_User, kUserMessageType_Identify, buffer);
}

bool ChatClient::messageSendToUser(RemoteChatClient *target,
  std::string message) {
  TypedBuffer buffer = createBuffer();
  buffer.WriteUInt16(kMessageResult_Succeeded); // Result
  buffer.WriteString(target->Username); // Target username
  buffer.WriteString(message); // Actual message
  return send(kComponentType_User, kUserMessageType_SendMessage, buffer);
}

bool ChatClient::messageSendToUser(std::string &target, std::string message) {
  TypedBuffer buffer = createBuffer();
  buffer.WriteUInt16(kMessageResult_Succeeded); // Result
  buffer.WriteString(target); // Target username
  buffer.WriteString(message); // Actual message
  return send(kComponentType_User, kUserMessageType_SendMessage, buffer);
}
}
