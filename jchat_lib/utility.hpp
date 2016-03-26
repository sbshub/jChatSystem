/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_lib_utility_hpp_
#define jchat_lib_utility_hpp_

// Required libraries
#include <cstdlib>
#include <ctime>
#include <mutex>

namespace jchat {
class Utility {
public:
  static uint32_t Random(uint32_t min, uint32_t max) {
    static bool seeded = false;
    static std::mutex mutex;
    mutex.lock();
    if (!seeded) {
      srand(time(0));
      seeded = true;
    }
    uint32_t random = (rand() % (max - min + 1)) + min;
    mutex.unlock();
    return random;
  }
};
}

#endif // jchat_lib_utility_hpp_