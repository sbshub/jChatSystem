/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*
*/

// Required libraries
#include "buffer.hpp"
#include <iostream>

// Required namespaces
using namespace jchat;

// Program entrypoint
int main(int argc, char **argv) {
  std::cout << "jChatSystem - Server" << std::endl;

  // Test Buffer class
  Buffer buffer;
  buffer.Write<uint64_t>(5666);

  buffer.Rewind();

  uint64_t number = 0;
  if (buffer.Read(&number)) {
    std::cout << "I read the number " << number << "!" << std::endl;
  }

  std::cout << "Buffer size = " << buffer.GetSize() << " bytes" << std::endl;

  return 0;
}
