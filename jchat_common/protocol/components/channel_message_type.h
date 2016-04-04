/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_channel_message_type_h_
#define jchat_common_channel_message_type_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum ChannelMessageType : uint16_t {
  kChannelMessageType_JoinChannel,
  kChannelMessageType_JoinChannel_Complete,
  kChannelMessageType_LeaveChannel,
  kChannelMessageType_LeaveChannel_Complete,
  kChannelMessageType_SendMessage,
  kChannelMessageType_SendMessage_Complete,
  kChannelMessageType_OpUser,
  kChannelMessageType_OpUser_Complete,
  kChannelMessageType_DeopUser,
  kChannelMessageType_DeopUser_Complete,
  kChannelMessageType_KickUser,
  kChannelMessageType_KickUser_Complete,
  kChannelMessageType_BanUser,
  kChannelMessageType_BanUser_Complete,
  kChannelMessageType_UnbanUser,
  kChannelMessageType_UnbanUser_Complete,

  kChannelMessageType_Max,
};
}

#endif // jchat_common_channel_message_type_h_
