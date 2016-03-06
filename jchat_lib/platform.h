/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_lib_platform_h_
#define jchat_lib_platform_h_

#if defined(__linux__)
#define OS_LINUX
#else
#error "Unsupported platform!"
#endif

#endif // jchat_lib_platform_h_
