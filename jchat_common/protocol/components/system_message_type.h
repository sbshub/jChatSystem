/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_system_message_type_h_
#define jchat_common_system_message_type_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum SystemMessageType : uint16_t {
  kSystemMessageType_Hello,
  kSystemMessageType_Complete_Hello,
  kSystemMessageType_Max,
};
}

#endif // jchat_common_system_message_type_h_
