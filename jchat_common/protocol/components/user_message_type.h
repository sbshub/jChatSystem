/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_user_message_type_h_
#define jchat_common_user_message_type_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum UserMessageType : uint16_t {
  kUserMessageType_Identify,
  kUserMessageType_Identify_Complete,
  kUserMessageType_SendMessage,
  kUserMessageType_SendMessage_Complete,
  kUserMessageType_Max,
};
}

#endif // jchat_common_user_message_type_h_
