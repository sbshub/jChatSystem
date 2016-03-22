/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_channel_message_result_h_
#define jchat_common_channel_message_result_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum ChannelMessageResult : uint16_t {
  // General
  kChannelMessageResult_Ok,
  kChannelMessageResult_Fail,

  kChannelMessageResult_NotIdentified,

  // JoinChannel
  kChannelMessageResult_InvalidChannelName,
  kChannelMessageResult_ChannelCreated,
  kChannelMessageResult_UserJoined,

  // SendMessage
  kChannelMessageResult_NotInChannel,

  kChannelMessageResult_Max
};
}

#endif // jchat_common_channel_message_result_h_
