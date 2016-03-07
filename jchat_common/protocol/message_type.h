/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_message_type_h_
#define jchat_common_message_type_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum MessageType : uint16_t {
  kMessageType_Hello,
  kMessageType_Complete_Hello,
  kMessageType_Quit,
  kMessageType_Complete_Quit,
  kMessageType_Idenfity,
  kMessageType_Complete_Idenfity,
  kMessageType_SendMessage,
  kMessageType_Complete_SendMessage,
  kMessageType_JoinChannel,
  kMessageType_Complete_JoinChannel,
  kMessageType_PartChannel,
  kMessageType_Complete_PartChannel,
  kMessageType_OpClientInChannel,
  kMessageType_Complete_OpClientInChannel,
  kMessageType_DeOpClientInChannel,
  kMessageType_Complete_DeOpClientInChannel,
  kMessageType_KickClientFromChannel,
  kMessageType_Complete_KickClientFromChannel,
  kMessageType_BanClientFromChannel,
  kMessageType_Complete_BanClientFromChannel,
  kMessageType_Max,
};
}

#endif // jchat_common_message_type_h_
