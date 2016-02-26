/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*
*/

#ifndef jchat_lib_buffer_hpp_
#define jchat_lib_buffer_hpp_

// Required libraries
#include <vector>
#include <stdint.h>
#include <stdlib.h>

namespace jchat {
class Buffer {
  std::vector<uint8_t> buffer_;
  size_t current_position_;
  bool flip_endian_;

  template<typename _TData>
  static void FlipEndian(_TData *buffer, size_t size) {
    uint8_t *p_buffer = *(uint8_t **)&buffer;
    for (size_t i = 0; i < size / 2; i++) {
      uint8_t tmp = p_buffer[i];
      p_buffer[i] = p_buffer[size - 1 - i];
      p_buffer[size - 1 - i] = tmp;
    }
  }

public:
  Buffer(bool flip_endian = false) : current_position_(0),
    flip_endian_(flip_endian) {
  }

  Buffer(const uint8_t *buffer, size_t size, bool flip_endian = false)
    : current_position_(0), flip_endian_(flip_endian) {
      for (size_t i = 0; i < size; i++) {
        buffer_.push_back(buffer[i]);
      }
  }

  ~Buffer() {
    Clear();
  }

  template<typename _TData>
  bool Read(_TData *obj) {
    size_t size = sizeof(_TData);
    if (current_position_ + size > buffer_.size()) {
      return false;
    }
    uint8_t *p_buffer = *(uint8_t **)&obj;
    for (size_t i = 0; i < size; i++) {
      p_buffer[i] = buffer_.data()[current_position_];
      current_position_++;
    }
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
    uint8_t *buffer = (uint8_t *)&obj;
    size_t size = sizeof(_TData);
    if (flip_endian_) {
      FlipEndian(buffer, size);
    }
    for (size_t i = 0; i < size; i++) {
      if (current_position_ + i >= this->buffer_.size()) {
        buffer_.push_back(buffer[i]);
      } else {
        buffer_[current_position_] = buffer[i];
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
    // Set all the data to 0 in case we had important data in the buffer
    for (size_t i = 0; i < buffer_.size(); i++) {
      buffer_[i] = 0;
    }
    buffer_.clear();
    buffer_.shrink_to_fit();
    current_position_ = 0;
  }
};
}

#endif // jchat_lib_buffer_hpp_
