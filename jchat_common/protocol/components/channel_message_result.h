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
  kChannelMessageResult_InvalidChannelName,
  kChannelMessageResult_InvalidUsername,
  kChannelMessageResult_NotInChannel,
  kChannelMessageResult_NotPermitted,
  kChannelMessageResult_UserNotInChannel,

  // JoinChannel
  kChannelMessageResult_ChannelCreated,
  kChannelMessageResult_ChannelNameTooLong,
  kChannelMessageResult_AlreadyInChannel,
  kChannelMessageResult_BannedFromChannel,
  kChannelMessageResult_UserJoined,

  // LeaveChannel
  kChannelMessageResult_ChannelDestroyed,
  kChannelMessageResult_UserLeft,

  // SendMessage
  kChannelMessageResult_InvalidMessage,
  kChannelMessageResult_MessageTooLong,
  kChannelMessageResult_MessageSent,

  // OpUser
  kChannelMessageResult_AlreadyOperator,
  kChannelMessageResult_CannotOpSelf,
  kChannelMessageResult_UserOpped,

  // DeopUser
  kChannelMessageResult_AlreadyNotOperator,
  kChannelMessageResult_UserDeopped,

  // KickUser
  kChannelMessageResult_CannotKickSelf,
  kChannelMessageResult_UserKicked,

  // BanUser
  kChannelMessageResult_AlreadyBanned,
  kChannelMessageResult_CannotBanSelf,
  kChannelMessageResult_UserBanned,

  // UnbanUser
  kChannelMessageResult_NotBanned,
  kChannelMessageResult_CannotUnbanSelf,
  kChannelMessageResult_UserUnbanned,

  kChannelMessageResult_Max
};
}

#endif // jchat_common_channel_message_result_h_
