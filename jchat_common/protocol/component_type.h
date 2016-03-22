/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_component_type_h_
#define jchat_common_component_type_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum ComponentType : uint8_t {
  kComponentType_System,
  kComponentType_User,
  kComponentType_Channel,
  kComponentType_Max,
};
}

#endif // jchat_common_component_type_h_
