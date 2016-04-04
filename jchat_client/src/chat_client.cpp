/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "chat_client.h"

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

  is_connected_ = false;

  return true;
}

bool ChatClient::AddComponent(std::shared_ptr<ChatComponent> component) {
  if (is_connected_) {
    return false;
  }

  for (auto it = components_.begin(); it != components_.end(); ++it) {
    if (*it == component) {
      return false;
    }
  }
  if (!component->Initialize(*this)) {
    return false;
  }
  components_.push_back(component);

  return true;
}

bool ChatClient::RemoveComponent(std::shared_ptr<ChatComponent> component) {
  if (is_connected_) {
    return false;
  }

  for (auto it = components_.begin(); it != components_.end(); ++it) {
    if (*it == component) {
      if (!component->Shutdown()) {
        return false;
      }
      components_.erase(it);
      return true;
    }
  }

  return false;
}

bool ChatClient::GetComponent(ComponentType component_type,
  std::shared_ptr<ChatComponent> &out_component) {
  for (auto component : components_) {
    if (component->GetType() == component_type) {
      out_component = component;
      return true;
    }
  }
  return false;
}

TypedBuffer ChatClient::CreateBuffer() {
  return TypedBuffer(!is_little_endian_);
}

bool ChatClient::Send(ComponentType component_type, uint8_t message_type,
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

IPEndpoint ChatClient::GetLocalEndpoint() {
  return tcp_client_.GetLocalEndpoint();
}

IPEndpoint ChatClient::GetRemoteEndpoint() {
  return tcp_client_.GetRemoteEndpoint();
}

bool ChatClient::onConnected() {
  for (auto component : components_) {
    component->OnConnected();
  }

  return OnConnected();
}

bool ChatClient::onDisconnected() {
  for (auto component : components_) {
    component->OnDisconnected();
  }

  return OnDisconnected();
}

bool ChatClient::onDataReceived(Buffer &buffer) {
  uint8_t component_type = 0;
  uint16_t message_type = 0;
  uint32_t size = 0;

  // Determine the header size
  size_t header_size = sizeof(component_type) + sizeof(message_type)
    + sizeof(size);

  // Try to handle the requests, if any are unhandled, drop the connection
  bool handled = false;

  // Keep reading the buffer till the end
  while (buffer.GetSize() - buffer.GetPosition() >= header_size) {
    // Flip data endian order if needed
    buffer.SetFlipEndian(!is_little_endian_);

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
    buffer.SetPosition(buffer.GetPosition() + size);

    for (auto component : components_) {
      if (component->GetType() == static_cast<ComponentType>(component_type)) {
        if (component->Handle(message_type, typed_buffer)) {
          handled = true;
          break;
        }
      }
    }
  }

  return handled;
}
}
