/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*
*/

#ifndef common_buffer_hpp_
#define common_buffer_hpp_

#include <vector>
#include <stdint.h>
#include <stdlib.h>

namespace jchat {
class Buffer {
  std::vector<uint8_t> buffer_;
  size_t current_position_;
  bool flip_endian_;

  static void FlipEndian(uint8_t *buffer, size_t size) {
		for (size_t i = 0; i < size / 2; i++) {
			uint8_t tmp = buffer[i];
			buffer[i] = buffer[size - 1 - i];
			buffer[size - 1 - i] = tmp;
		}
	}

public:
  Buffer(bool flip_endian = false) : current_position_(0),
    flip_endian_(flip_endian) {
  }

  Buffer(const uint8_t *buffer, size_t size, bool flip_endian = false)
    : buffer_(buffer, size), current_position_(0), flip_endian_(flip_endian) {
  }

  ~Buffer() {
    Clear();
  }

  template<typename _TData>
  bool Read(_TData *obj) {
    auto size = sizeof(_TData);
    if (current_position_ + size > buffer_.size()) {
      return false;
    }
    memcpy(obj, buffer_.data() + current_position_, size);
    if (flip_endian_) {
      FlipEndian(obj, size);
    }
    return true;
  }

  template<typename _TData>
  bool ReadArray(_TData *obj, size_t size) {
    if (current_position_ + size * sizeof(_TData) - 1 > buffer_.size()) {
      return false;
    }
    for (size_t i = 0; i < size; i++) {
      if (!Read<_TData>(&obj[i])) {
        return false;
      }
    }
    return true;
  }

  template<typename _TData>
  void Write(_TData obj) {
    auto size = sizeof(_TData);
    if (flip_endian_) {
      FlipEndian(&obj, size);
    }
    for (size_t i = 0; i < size; i++) {
      if (current_position_ + i > this->buffer_.size()) {
        buffer_.push_back(&obj[i]);
      } else {
        buffer_[current_position_] = &obj[i];
      }
      current_position_++;
    }
  }

  template<typename _TData>
	void WriteArray(_TData *obj, size_t size) {
		for (size_t i = 0; i < size; i++) {
			Write(obj[i]);
		}
	}

  size_t GetPosition() {
    return current_position_;
  }

  bool SetPosition(size_t current_position) {
    if (current_position > buffer_.size() - 1) {
      return false;
    }
    current_position_ = current_position;
    return true;
  }

  void Rewind() {
    current_position_ = 0;
  }

  size_t GetSize() {
    return buffer_.size();
  }

  const uint8_t *GetBuffer() {
    return buffer_.data();
  }

  void Clear() {
    memset(buffer_.data(), 0, buffer_.size());
    buffer_.clear();
    buffer_.shrink_to_fit();
    current_position_ = 0;
  }
};
}

#endif // common_buffer_hpp_
