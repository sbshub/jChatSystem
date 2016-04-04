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
#elif defined(__APPLE__)
#define OS_OSX
#elif defined(__unix)
#define OS_UNIX
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) \
  || defined(__MINGW32__)
#define OS_WIN
#else
#error "Unsupported platform!"
#endif

#endif // jchat_lib_platform_h_
