/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "chat_server.h"
// NOTE: Debug!
#include <iostream>

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
}

bool ChatServer::Start() {
  if (is_listening_) {
    return false;
  }

  if (!tcp_server_.Start()) {
    return false;
  }

  components_mutex_.lock();
  for (auto component : components_) {
    component->OnStart();
  }
  components_mutex_.unlock();

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

  components_mutex_.lock();
  for (auto component : components_) {
    component->OnStop();
  }
  components_mutex_.unlock();

  is_listening_ = false;

  return true;
}

bool ChatServer::AddComponent(ChatComponent *component) {
  components_mutex_.lock();
  for (auto it = components_.begin(); it != components_.end(); ++it) {
    if (*it == component) {
      components_mutex_.unlock();
      return false;
    }
  }
  if (!component->Initialize(*this)) {
    components_mutex_.unlock();
    return false;
  }
  components_.push_back(component);
  components_mutex_.unlock();

  return true;
}

bool ChatServer::RemoveComponent(ChatComponent *component) {
  components_mutex_.lock();
  for (auto it = components_.begin(); it != components_.end(); ++it) {
    if (*it == component) {
      if (!component->Shutdown()) {
        return false;
      }
      components_.erase(it);
      components_mutex_.unlock();
      return true;
    }
  }
  components_mutex_.unlock();

  return false;
}

bool ChatServer::GetComponent(ComponentType component_type,
  ChatComponent *out_component) {
  components_mutex_.lock();
  for (auto component : components_) {
    if (component->GetType() == component_type) {
      components_mutex_.unlock();
      out_component = component;
      return true;
    }
  }
  components_mutex_.unlock();
  return false;
}

TypedBuffer ChatServer::CreateBuffer() {
    return TypedBuffer(!is_little_endian_);
}

bool ChatServer::SendUnicast(RemoteChatClient &client,
  ComponentType component_type, uint8_t message_type, TypedBuffer &buffer) {
  TcpClient *tcp_client = NULL;
  if (!getTcpClient(client, &tcp_client)) {
    return false;
  }
  return sendUnicast(*tcp_client, component_type, message_type, buffer);
}

bool ChatServer::SendUnicast(RemoteChatClient *client,
  ComponentType component_type, uint8_t message_type, TypedBuffer &buffer) {
  TcpClient *tcp_client = NULL;
  if (!getTcpClient(*client, &tcp_client)) {
    return false;
  }
  return sendUnicast(*tcp_client, component_type, message_type, buffer);
}

bool ChatServer::SendMulticast(std::vector<RemoteChatClient *> clients,
  ComponentType component_type, uint8_t message_type, TypedBuffer &buffer) {
  bool success = true;
  for (auto client : clients) {
    TcpClient *tcp_client = NULL;
    if (!getTcpClient(*client, &tcp_client)) {
      return false;
    }
    if (!sendUnicast(*tcp_client, component_type, message_type, buffer)) {
      success = false;
    }
  }
  return success;
}

IPEndpoint ChatServer::GetListenEndpoint() {
  return tcp_server_.GetListenEndpoint();
}

bool ChatServer::onClientConnected(TcpClient &tcp_client) {
  RemoteChatClient *chat_client = new RemoteChatClient();

  // Set the endpoint for the client as the remote endpoint (the client's
  // address and port)
  chat_client->Endpoint = tcp_client.GetRemoteEndpoint();

  components_mutex_.lock();
  for (auto component : components_) {
    component->OnClientConnected(*chat_client);
  }
  components_mutex_.unlock();

  clients_mutex_.lock();
  clients_[&tcp_client] = chat_client;
  clients_mutex_.unlock();

  OnClientConnected(*chat_client);

  return true;
}

bool ChatServer::onClientDisconnected(TcpClient &tcp_client) {
  clients_mutex_.lock();
  RemoteChatClient *chat_client = clients_[&tcp_client];
  clients_mutex_.unlock();

  components_mutex_.lock();
  for (auto component : components_) {
    component->OnClientDisconnected(*chat_client);
  }
  components_mutex_.unlock();

  // TODO/NOTE: We need to remove the client from any channels where they're in
  // or where they have operator or any privileges, and we can do this in the
  // appropriate components using the OnClientDisconnected, etc. events within
  // them -- And remove those channel/identified things from the
  // RemoteChatClient class
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

  // Determine the header size
  size_t header_size = sizeof(component_type) + sizeof(message_type)
    + sizeof(size);

  // Flip data endian order if needed
  buffer.SetFlipEndian(!is_little_endian_);

  // Try to handle the requests, if any are unhandled, drop the connection
  bool handled = false;

  // Keep reading the buffer till the end
  while (buffer.GetSize() - buffer.GetPosition() >= header_size) {
    // Check if the packet is valid
    if (!buffer.Read(&component_type) || !buffer.Read(&message_type)
      || !buffer.Read(&size) || buffer.GetSize() - buffer.GetPosition() < size
      || component_type >= kComponentType_Max) {
      // Drop connection
      return false;
    }

    // Read the packet into a typed buffer
    TypedBuffer typed_buffer(buffer.GetBuffer() + buffer.GetPosition(),
      size, !is_little_endian_);

    // Increase the position of the buffer
    buffer.SetPosition(buffer.GetPosition() + size - 1);

    clients_mutex_.lock();
    RemoteChatClient *chat_client = clients_[&tcp_client];
    clients_mutex_.unlock();

    components_mutex_.lock();
    for (auto component : components_) {
      if (component->GetType() == static_cast<ComponentType>(component_type)) {
        if (component->Handle(*chat_client, message_type, typed_buffer)) {
          handled = true;
        }
      }
    }
    components_mutex_.unlock();
  }

  return handled;
}

bool ChatServer::getTcpClient(RemoteChatClient &client, TcpClient **out_client) {
  clients_mutex_.lock();
  for (auto pair : clients_) {
    if (pair.second == &client) {
      clients_mutex_.unlock();
      *out_client = pair.first;
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
}
