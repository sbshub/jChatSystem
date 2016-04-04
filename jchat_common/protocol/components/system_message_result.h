/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_system_message_result_h_
#define jchat_common_system_message_result_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum SystemMessageResult : uint16_t {
  // General
  kSystemMessageResult_Ok,
  kSystemMessageResult_Fail,

  // Hello
  kSystemMessageResult_InvalidProtocolVersion,

  kSystemMessageResult_Max
};
}

#endif // jchat_common_system_message_result_h_
