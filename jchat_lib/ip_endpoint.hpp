/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_lib_ip_endpoint_hpp_
#define jchat_lib_ip_endpoint_hpp_

// Required libraries
#include "platform.h"
#if defined(OS_LINUX) || defined(OS_OSX) || defined(OS_UNIX)
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#elif defined(OS_WIN)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#endif
#include <string>
#include <stdint.h>

namespace jchat {
class IPEndpoint {
  sockaddr_in endpoint_;
  std::string address_;
  uint16_t port_;

public:
  IPEndpoint() : address_("0.0.0.0"), port_(0) {
    endpoint_.sin_family = AF_INET;
    endpoint_.sin_addr.s_addr = inet_addr(address_.c_str());
    endpoint_.sin_port = htons(port_);
  }

  IPEndpoint(std::string address, uint16_t port) : address_(address),
    port_(port) {
    endpoint_.sin_family = AF_INET;
    endpoint_.sin_addr.s_addr = inet_addr(address_.c_str());
    endpoint_.sin_port = htons(port_);
  }

  IPEndpoint(sockaddr_in endpoint) : endpoint_(endpoint) {
    address_ = inet_ntoa(endpoint.sin_addr);
    port_ = ntohs(endpoint.sin_port);
  }

  void SetAddress(std::string address) {
    address_ = address;
    endpoint_.sin_addr.s_addr = inet_addr(address_.c_str());
  }

  void SetAddress(uint32_t address) {
    endpoint_.sin_addr.s_addr = htonl(address);
    address_ = inet_ntoa(endpoint_.sin_addr);
  }

  uint32_t GetAddress() {
    return ntohl(endpoint_.sin_addr.s_addr);
  }

  std::string GetAddressString() {
    return address_;
  }

  void SetPort(uint16_t port) {
    port_ = port;
    endpoint_.sin_port = htons(port_);
  }

  uint16_t GetPort() {
    return port_;
  }

  void SetSocketEndpoint(sockaddr_in endpoint) {
    endpoint_ = endpoint;
    address_ = inet_ntoa(endpoint.sin_addr);
    port_ = ntohs(endpoint.sin_port);
  }

  sockaddr_in GetSocketEndpoint() {
    return endpoint_;
  }

  std::string ToString() {
    return address_ + ":" + std::to_string(port_);
  }
};
}

#endif // jchat_lib_ip_endpoint_hpp_
