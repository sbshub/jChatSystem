/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*
*/

#ifndef jchat_lib_event_hpp_
#define jchat_lib_event_hpp_

// Required libraries
#include <mutex>
#include <vector>
#include <functional>

namespace jchat {
template<typename... _TArgs>
struct EventCallback {
  std::function<bool(_TArgs...)> Function;
  bool IsTemporary;
};

template<typename... _TArgs>
class Event {
  std::mutex event_mutex_;
	std::vector<EventCallback<_TArgs...>> callbacks_;

public:
  Event() {}
  Event(const Event &event) {}

  template<typename _TFunction>
	Event &Add(_TFunction function, bool is_temporary = false) {
    event_mutex_.lock();

		EventCallback<_TArgs...> callback;
		callback.Function = function;
		callback.IsTemporary = is_temporary;
		callbacks_.push_back(callback);

		event_mutex_.unlock();

		return *this;
  }

  bool operator()(_TArgs... arguments) {
    event_mutex_.lock();

		bool success = true;

		for (auto iterator = callbacks_.begin(); iterator != callbacks_.end();) {
			if (!iterator->Function(arguments...)) {
				success = false;
			}

			if (iterator->IsTemporary) {
				iterator = callbacks_.erase(iterator);
			} else {
				++iterator;
			}
		}

		event_mutex_.unlock();

		return success;
  }
};
}

#endif // jchat_lib_event_hpp_
