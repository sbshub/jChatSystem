/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "core/chat_server.h"

namespace jchat {
ChatServer::ChatServer(const char *hostname, uint16_t port)
  : tcp_server_(hostname, port), is_listening_(false) {
  int16_t number = 0x00FF;
  is_little_endian_ = ((uint8_t *)&number)[0] == 0xFF;

  tcp_server_.OnClientConnected.Add([this](TcpClient &client) {
    return onClientConnected(client);
  });
  tcp_server_.OnClientDisconnected.Add([this](TcpClient &client) {
    return onClientDisconnected(client);
  });
  tcp_server_.OnDataReceived.Add([this](TcpClient &client, Buffer &buffer) {
    return onDataReceived(client, buffer);
  });
}

ChatServer::~ChatServer() {
  // Remove clients
  if (!clients_.empty()) {
    for (auto client : clients_) {
      client.first->Disconnect();
      delete client.second;
    }
    clients_.clear();
  }

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

bool ChatServer::Start() {
  if (is_listening_) {
    return false;
  }

  if (!tcp_server_.Start()) {
    return false;
  }

  is_listening_ = true;

  return true;
}

bool ChatServer::Stop() {
  if (!is_listening_) {
    return false;
  }

  if (!tcp_server_.Stop()) {
    return false;
  }

  // Remove clients
  if (!clients_.empty()) {
    for (auto client : clients_) {
      client.first->Disconnect();
      delete client.second;
    }
    clients_.clear();
  }

  // Remove channels
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }

  is_listening_ = false;

  return true;
}

bool ChatServer::AddHandler(ChatHandler *handler) {
  handlers_mutex_.lock();
  for (auto it = handlers_.begin(); it != handlers_.end();) {
    if (*it == handler) {
      handlers_mutex_.unlock();
      return false;
    }
  }
  handlers_.push_back(handler);
  handlers_mutex_.unlock();

  return true;
}

bool ChatServer::RemoveHandler(ChatHandler *handler) {
  handlers_mutex_.lock();
  for (auto it = handlers_.begin(); it != handlers_.end();) {
    if (*it == handler) {
      handlers_.erase(it);
      handlers_mutex_.unlock();
      return true;
    }
  }
  handlers_mutex_.unlock();

  return false;
}

bool ChatServer::SendMessageToClient(RemoteChatClient *source,
  RemoteChatClient *target, std::string message) {
  TypedBuffer buffer = createBuffer();
  buffer.WriteUInt16(kMessageResult_Succeeded); // Result
  buffer.WriteString(source->Username); // Source username
  buffer.WriteString(source->Hostname); // Source hostname
  buffer.WriteString(message); // Actual message
  return sendUnicast(target, kMessageType_Complete_SendMessage, buffer);
}

bool ChatServer::onClientConnected(TcpClient &tcp_client) {
  RemoteChatClient *chat_client = new RemoteChatClient();

  // Set the endpoint for the client as the remote endpoint (the client's
  // address and port)
  chat_client->Endpoint = tcp_client.GetRemoteEndpoint();

  // This username should be fine as the client will not be able to
  // join any channels or do anything unless they've identified
  chat_client->Username = "guest";

  // The client's hostname is not masked until they've identified
  chat_client->Hostname = chat_client->Endpoint.GetAddressString();

  clients_mutex_.lock();
  clients_[&tcp_client] = chat_client;
  clients_mutex_.unlock();

  OnClientConnected(*chat_client);

  return true;
}

bool ChatServer::onClientDisconnected(TcpClient &tcp_client) {
  RemoteChatClient *chat_client = clients_[&tcp_client];

  // Remove client from channels
  channels_mutex_.lock();
  for (auto channel : channels_) {
    removeChannelOperator(channel, chat_client);
    removeChannelClient(channel, chat_client);
  }
  channels_mutex_.unlock();

  // Remove client
  clients_mutex_.lock();
  clients_.erase(&tcp_client);
  clients_mutex_.unlock();

  OnClientDisconnected(*chat_client);
  delete chat_client;

  return true;
}

bool ChatServer::onDataReceived(TcpClient &tcp_client, Buffer &buffer) {
  uint16_t message_type = 0;
  uint32_t size = 0;

  // Flip data endian order if needed
  buffer.SetFlipEndian(!is_little_endian_);

  // Check if the packet is valid
  if (!buffer.Read(&message_type) || !buffer.Read(&size)
    || buffer.GetSize() - buffer.GetPosition() != size
    || message_type >= kMessageType_Max) {
    // Drop connection
    return false;
  }

  // Read the packet into a typed buffer
  TypedBuffer typed_buffer(buffer.GetBuffer() + buffer.GetPosition(),
    size, !is_little_endian_);

  RemoteChatClient *chat_client = clients_[&tcp_client];

  // Try to handle the request, if it is unhandled, drop the connection
  bool handled = false;
  handlers_mutex_.lock();
  for (auto handler : handlers_) {
    if (handler->GetType() == message_type) {
      if (handler->Handle(*this, *chat_client, typed_buffer)) {
        handled = true;
      }
    }
  }
  handlers_mutex_.unlock();

  return handled;
}

TypedBuffer ChatServer::createBuffer() {
    return TypedBuffer(!is_little_endian_);
}

bool ChatServer::getTcpClient(RemoteChatClient &client, TcpClient *out_client) {
  clients_mutex_.lock();
  for (auto pair : clients_) {
    if (pair.second == &client) {
      clients_mutex_.unlock();
      out_client = pair.first;
      return true;
    }
  }
  clients_mutex_.unlock();
  return false;
}

bool ChatServer::sendUnicast(TcpClient &client, MessageType message_type,
  TypedBuffer &buffer) {
  Buffer temp_buffer(!is_little_endian_);

  // Write header
  temp_buffer.Write<uint16_t>(message_type);
  temp_buffer.Write<uint32_t>(buffer.GetSize());

  // Write body
  temp_buffer.WriteArray<uint8_t>(buffer.GetBuffer(), buffer.GetSize());

  return tcp_server_.Send(client, temp_buffer);
}

bool ChatServer::sendUnicast(TcpClient *client, MessageType message_type,
  TypedBuffer &buffer) {
  return sendUnicast(*client, message_type, buffer);
}

bool ChatServer::sendMulticast(std::vector<TcpClient *> clients,
  MessageType message_type, TypedBuffer &buffer) {
  bool success = true;
  for (auto client : clients) {
    if (!sendUnicast(client, message_type, buffer)) {
      success = false;
    }
  }
  return success;
}

bool ChatServer::sendUnicast(RemoteChatClient &client, MessageType message_type,
  TypedBuffer &buffer) {
  TcpClient *tcp_client = NULL;
  if (!getTcpClient(client, tcp_client)) {
    return false;
  }
  return sendUnicast(tcp_client, message_type, buffer);
}

bool ChatServer::sendUnicast(RemoteChatClient *client, MessageType message_type,
  TypedBuffer &buffer) {
  TcpClient *tcp_client = NULL;
  if (!getTcpClient(*client, tcp_client)) {
    return false;
  }
  return sendUnicast(tcp_client, message_type, buffer);
}

bool ChatServer::sendMulticast(std::vector<RemoteChatClient *> clients,
  MessageType message_type, TypedBuffer &buffer) {
  bool success = true;
  for (auto client : clients) {
    TcpClient *tcp_client = NULL;
    if (!getTcpClient(*client, tcp_client)) {
      return false;
    }
    if (!sendUnicast(tcp_client, message_type, buffer)) {
      success = false;
    }
  }
  return success;
}

bool addChannelOperator(ChatChannel *channel, RemoteChatClient *client) {
  channel->OperatorsMutex.lock();
  for (auto it = channel->Operators.begin(); it != channel->Operators.end();) {
    if (*it == client) {
      channel->OperatorsMutex.unlock();
      return false;
    }
  }

  channel->Operators.push_back(client);
  channel->OperatorsMutex.unlock();

  // TODO: Broadcast/Multicast (Do this in the handlers, not here)

  return true;
}

bool addChannelClient(ChatChannel *channel, RemoteChatClient *client) {
  channel->ClientsMutex.lock();
  for (auto it = channel->Clients.begin(); it != channel->Clients.end();) {
    if (*it == client) {
      channel->ClientsMutex.unlock();
      return false;
    }
  }

  channel->Clients.push_back(client);
  channel->ClientsMutex.unlock();

  client->ChannelsMutex.lock();
  client->Channels.push_back(channel->Name);
  client->ChannelsMutex.unlock();

  // TODO: Broadcast/Multicast

  return true;
}

bool ChatServer::removeChannelOperator(ChatChannel *channel,
  RemoteChatClient *client) {
  for (auto it = channel->Operators.begin(); it != channel->Operators.end();) {
    if (*it == client) {
      // TODO: Broadcast/Multicast
      it = channel->Operators.erase(it);
      return true;
    }
  }
  return false;
}

bool ChatServer::removeChannelClient(ChatChannel *channel,
  RemoteChatClient *client) {
  for (auto it = channel->Clients.begin(); it != channel->Clients.end();) {
    if (*it == client) {
      client->ChannelsMutex.lock();
      for (auto itc = client->Channels.begin();
        itc != client->Channels.end();) {
        if (*itc == channel->Name) {
          client->Channels.erase(itc);
          break;
        }
      }
      client->ChannelsMutex.unlock();
      // TODO: Broadcast/Multicast
      it = channel->Clients.erase(it);
      return true;
    }
  }
  return false;
}
}