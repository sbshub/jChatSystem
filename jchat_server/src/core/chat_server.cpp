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

  // Remove components
  if (!components_.empty()) {
    for (auto component : components_) {
      delete component;
    }
    components_.clear();
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
  clients_mutex_.lock();
  if (!clients_.empty()) {
    for (auto client : clients_) {
      client.first->Disconnect();
      delete client.second;
    }
    clients_.clear();
  }
  clients_mutex_.unlock();

  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
  channels_mutex_.unlock();

  is_listening_ = false;

  return true;
}

bool ChatServer::AddComponent(ChatComponent *component) {
  components_mutex_.lock();
  for (auto it = components_.begin(); it != components_.end();) {
    if (*it == component) {
      components_mutex_.unlock();
      return false;
    }
  }
  components_.push_back(component);
  components_mutex_.unlock();

  return true;
}

bool ChatServer::RemoveComponent(ChatComponent *component) {
  components_mutex_.lock();
  for (auto it = components_.begin(); it != components_.end();) {
    if (*it == component) {
      components_.erase(it);
      components_mutex_.unlock();
      return true;
    }
  }
  components_mutex_.unlock();

  return false;
}

ChatComponent *ChatServer::GetComponent(ComponentType component_type) {
  components_mutex_.lock();
  for (auto component : components_) {
    if (component->GetType() == component_type) {
      components_mutex_.unlock();
      return component;
    }
  }
  components_mutex_.unlock();
  return 0;
}

IPEndpoint ChatServer::GetListenEndpoint() {
  return tcp_server_.GetListenEndpoint();
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

  // TODO/NOTE: We need to remove the client from any channels where they're in
  // or where they have operator or any privileges, and we can do this in the
  // appropriate components using the OnClientDisconnected, etc. events within
  // them
  OnClientDisconnected(*chat_client);

  // Remove client
  clients_mutex_.lock();
  clients_.erase(&tcp_client);
  clients_mutex_.unlock();

  delete chat_client;

  return true;
}

bool ChatServer::onDataReceived(TcpClient &tcp_client, Buffer &buffer) {
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

  RemoteChatClient *chat_client = clients_[&tcp_client];

  // Try to handle the request, if it is unhandled, drop the connection
  bool handled = false;
  components_mutex_.lock();
  for (auto component : components_) {
    if (component->GetType() == message_type) {
      if (component->Handle(*this, *chat_client, message_type, typed_buffer)) {
        handled = true;
      }
    }
  }
  components_mutex_.unlock();

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

bool ChatServer::sendUnicast(TcpClient &client, ComponentType component_type,
  uint8_t message_type, TypedBuffer &buffer) {
  Buffer temp_buffer(!is_little_endian_);

  // Write header
  temp_buffer.Write<uint8_t>(component_type);
  temp_buffer.Write<uint16_t>(message_type);
  temp_buffer.Write<uint32_t>(buffer.GetSize());

  // Write body
  temp_buffer.WriteArray<uint8_t>(buffer.GetBuffer(), buffer.GetSize());

  return tcp_server_.Send(client, temp_buffer);
}

bool ChatServer::sendUnicast(TcpClient *client, ComponentType component_type,
  uint8_t message_type, TypedBuffer &buffer) {
  return sendUnicast(*client, component_type, message_type, buffer);
}

bool ChatServer::sendMulticast(std::vector<TcpClient *> clients,
  ComponentType component_type, uint8_t message_type, TypedBuffer &buffer) {
  bool success = true;
  for (auto client : clients) {
    if (!sendUnicast(client, component_type, message_type, buffer)) {
      success = false;
    }
  }
  return success;
}

bool ChatServer::sendUnicast(RemoteChatClient &client,
  ComponentType component_type, uint8_t message_type, TypedBuffer &buffer) {
  TcpClient *tcp_client = NULL;
  if (!getTcpClient(client, tcp_client)) {
    return false;
  }
  return sendUnicast(tcp_client, component_type, message_type, buffer);
}

bool ChatServer::sendUnicast(RemoteChatClient *client,
  ComponentType component_type, uint8_t message_type, TypedBuffer &buffer) {
  TcpClient *tcp_client = NULL;
  if (!getTcpClient(*client, tcp_client)) {
    return false;
  }
  return sendUnicast(tcp_client, component_type, message_type, buffer);
}

bool ChatServer::sendMulticast(std::vector<RemoteChatClient *> clients,
  ComponentType component_type, uint8_t message_type, TypedBuffer &buffer) {
  bool success = true;
  for (auto client : clients) {
    TcpClient *tcp_client = NULL;
    if (!getTcpClient(*client, tcp_client)) {
      return false;
    }
    if (!sendUnicast(tcp_client, component_type, message_type, buffer)) {
      success = false;
    }
  }
  return success;
}
}
