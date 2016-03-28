/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_user_message_result_h_
#define jchat_common_user_message_result_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum UserMessageResult : uint16_t {
  // General
  kUserMessageResult_Ok,
  kUserMessageResult_Fail,

  // Identify
  kUserMessageResult_InvalidUsername,
  kUserMessageResult_UsernameInUse,

  kUserMessageResult_Max
};
}

#endif // jchat_common_user_message_result_h_
