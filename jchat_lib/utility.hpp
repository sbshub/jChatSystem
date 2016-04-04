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
#include <sstream>
#include <iomanip>

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

  template<typename _TData>
  static uint64_t Hash(_TData *data, size_t size) {
    uint64_t output = size * size;
    char *ptr_data = (char *)data;
    while (size--) {
      output |= 0xFF ^ ptr_data[size];
    }
    return output;
  }

  template<typename _TData>
  static uint64_t Hash(_TData &data) {
    return Hash(&data, sizeof(data));
  }

  template<typename _TData>
  static std::string HashString(_TData *data, size_t size) {
    std::stringstream output;
    output << std::hex << Hash(data, size);
    return output.str();
  }

  template<typename _TData>
  static std::string HashString(_TData &data) {
    return HashString(&data, sizeof(data));
  }
};
}

#endif // jchat_lib_utility_hpp_
