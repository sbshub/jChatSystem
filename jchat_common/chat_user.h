/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_chat_user_h_
#define jchat_common_chat_user_h_

namespace jchat {
struct ChatUser {
  bool Enabled;
  std::string Username;
  std::string Hostname;
  bool Identified;
};
}

#endif // jchat_common_chat_user_h_
