/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_lib_command_line_hpp_
#define jchat_lib_command_line_hpp_

// Required libraries
#include "string.hpp"
#include <map>
#include <stdlib.h>
#include <ostream>

namespace jchat {
class CommandLine {
  std::map<std::string, std::string> arguments_;

public:
  CommandLine(int argc, char **argv) {
    for (size_t i = 0; i < argc; i++) {
      std::string argument = argv[i];
      if (!argument.empty() && argument[0] == '-') {
        bool inserted = false;
        argument = argument.substr(1);
        if (i + 1 < argc) {
          std::string value = argv[i + 1];
          if (!value.empty() && value[0] != '-') {
            arguments_.insert(std::make_pair(argument, value));
            inserted = true;
          }
         } else if (!inserted) {
           arguments_.insert(std::make_pair(argument, ""));
         }
       }
     }
   }

  bool FlagExists(std::string key_name) {
    return arguments_.find(key_name) != arguments_.end();
  }

  int32_t GetInt32(std::string key_name, int32_t default_value) {
    auto value = arguments_.find(key_name);
    if (value == arguments_.end()) {
      return default_value;
    }
    return atoi(value->second.c_str());
  }

  std::string GetString(std::string key_name, std::string default_value) {
    auto value = arguments_.find(key_name);
    if (value == arguments_.end()) {
      return default_value;
    }
    return value->second;
  }

  friend std::ostream &operator<<(std::ostream &stream,
    CommandLine &command_line) {
    stream << "Command line: ";
    if (command_line.arguments_.size() > 0) {
      stream << std::endl;
    }
    for (auto &pair : command_line.arguments_) {
      stream << "\t- " << pair.first << " = " << pair.second << std::endl;
    }
    return stream;
  }
};
}

#endif // jchat_lib_command_line_hpp_
