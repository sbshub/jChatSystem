/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_lib_string_hpp_
#define jchat_lib_string_hpp_

// Required libraries
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <vector>

#ifndef JCHAT_STRING_BUFFER_SIZE
#define JCHAT_STRING_BUFFER_SIZE 1024
#endif // JCHAT_STRING_BUFFER_SIZE

namespace jchat {
class String {
public:
  static bool Equals(std::string str1, std::string str2) {
  	return strcmp(str1.c_str(), str2.c_str()) == 0;
  }

  static bool Contains(std::string target, std::string contains) {
  	return strstr(target.c_str(), contains.c_str()) != 0;
  }

  static std::string Format(std::string format, ...) {
  	std::string output;
  	char temp_string[JCHAT_STRING_BUFFER_SIZE];

  	va_list argument_list;
  	va_start(argument_list, format);

  	vsnprintf(temp_string, sizeof(temp_string), format.c_str(), argument_list);

  	va_end(argument_list);

  	output.append(temp_string);

  	return output;
  }

  static std::string PadLeft(std::string target, char character,
    int32_t count) {
  	std::string output;

  	int32_t length_to_add = count - target.size();
  	if (length_to_add < 0) {
  		return target;
  	}

  	output.append(length_to_add, character);
  	output.append(target);

  	return output;
  }

  static std::string PadRight(std::string target, char character,
    int32_t count) {
  	std::string output = target;

  	int32_t length_to_add = count - target.size();
  	if (length_to_add < 0) {
  		return target;
  	}

  	output.append(length_to_add, character);

  	return output;
  }

  static std::string Replace(std::string source, std::string from,
    std::string to, bool ignore_case) {
  	std::string output = source;

  	for (size_t x = 0; x < output.size(); x++) {
  		bool same = true;
  		for (size_t y = 0; y < from.size(); y++) {
  			char s = output[x + y];
  			char f = from[y];

  			if (ignore_case) {
  				s = tolower(s);
  				f = tolower(f);
  			}

  			if (s != f) {
  				same = false;
  				break;
  			}
  		}

  		if (same) {
  			output.replace(x, from.size(), to);
  		}
  	}

  	return output;
  }

  static std::vector<std::string> Split(std::string source, std::string split) {
  	std::string input = std::string(source);
  	std::vector<std::string> output;

  	int i = 0;
  	while ((i = input.find(split)) != std::string::npos) {
  		output.push_back(input.substr(0, i));
  		input.erase(0, i + split.size());
  	}

  	output.push_back(input);

  	return output;
  }

  static std::string Join(std::vector<std::string> source,
    std::string delimeter) {
    std::string output;
    for (auto it = source.begin(); it != source.end();) {
      output += *it;
      ++it;
      if (it != source.end()) {
        output += delimeter;
      }
    }
    return output;
  }

  static std::wstring ToWideString(std::string string) {
  	const char *c_string = string.c_str();

  	wchar_t *w_text = new wchar_t[string.size() + 1];
  	memset(w_text, 0, string.size() * sizeof(wchar_t));
  	mbstowcs(w_text, c_string, string.size());
  	w_text[string.size()] = L'\0';

  	std::wstring w_string = std::wstring(w_text);

  	delete[] w_text;

  	return w_string;
  }

  static std::string ToString(std::wstring string) {
  	const wchar_t *w_string = string.c_str();

  	char *c_text = new char[string.size() + 1];
  	memset(c_text, 0, string.size() * sizeof(char));
  	wcstombs(c_text, w_string, string.size());
  	c_text[string.size()] = '\0';

  	std::string c_string = std::string(c_text);

  	delete[] c_text;

  	return c_string;
  }
};
}

#endif // jchat_lib_string_hpp_
