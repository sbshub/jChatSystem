/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_typed_buffer_hpp_
#define jchat_common_typed_buffer_hpp_

// Required libraries
#include "buffer.hpp"

namespace jchat {
class TypedBuffer : Buffer {
  enum DataType : uint8_t {
    kDataType_Bool,
    kDataType_Char,
    kDataType_Int8,
    kDataType_UInt8,
    kDataType_Int16,
    kDataType_UInt16,
    kDataType_Int32,
    kDataType_UInt32,
    kDataType_Int64,
    kDataType_UInt64,
    kDataType_Float,
    kDataType_String,
    kDataType_Blob,
  };

  bool verifyDataType(DataType expected_type) {
    // Check to see if we're not going to be reading past the end of the buffer
    if (Buffer::GetPosition() == Buffer::GetSize()) {
      return false;
    }

    // Peek ahead instead of reading
    uint8_t type = Buffer::GetBuffer()[Buffer::GetPosition()];

    // Verify the data type
    if (type != (uint8_t)expected_type) {
      return false;
    }

    return true;
  }

public:
  TypedBuffer(bool flip_endian = false) : Buffer(flip_endian) {
  }

  TypedBuffer(const uint8_t *buffer, size_t size, bool flip_endian = false)
    : Buffer(buffer, size, flip_endian) {
  }

  bool ReadBoolean(bool &obj) {
    if (!verifyDataType(kDataType_Bool)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadChar(char &obj) {
    if (!verifyDataType(kDataType_Char)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadInt8(int8_t &obj) {
    if (!verifyDataType(kDataType_Int8)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadUInt8(uint8_t &obj) {
    if (!verifyDataType(kDataType_UInt8)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadInt16(int16_t &obj) {
    if (!verifyDataType(kDataType_Int16)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadUInt16(uint16_t &obj) {
    if (!verifyDataType(kDataType_UInt16)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadInt32(int32_t &obj) {
    if (!verifyDataType(kDataType_Int32)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadUInt32(uint32_t &obj) {
    if (!verifyDataType(kDataType_UInt32)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadInt64(int64_t &obj) {
    if (!verifyDataType(kDataType_Int64)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadUInt64(uint64_t &obj) {
    if (!verifyDataType(kDataType_UInt64)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadFloat(float &obj) {
    if (!verifyDataType(kDataType_Float)) {
      return false;
    }

    return Buffer::Read(&obj);
  }

  bool ReadString(std::string &obj) {
    if (!verifyDataType(kDataType_String)) {
      return false;
    }

    size_t length = 0;
    if (!Buffer::Read(&length)) {
      return false;
    }
    obj.resize(length);
    return Buffer::ReadArray<char>(const_cast<char *>(obj.c_str()), length);
  }

  bool ReadBlob(std::basic_string<uint8_t> &obj) {
    if (!verifyDataType(kDataType_Blob)) {
      return false;
    }

    size_t length = 0;
    if (!Buffer::Read(&length)) {
      return false;
    }
    obj.resize(length);
    return Buffer::ReadArray<uint8_t>(const_cast<uint8_t *>(obj.c_str()),
      length);
  }

  void WriteBoolean(bool obj) {
    Buffer::Write<uint8_t>(kDataType_Bool);
    Buffer::Write<bool>(obj);
  }

  void WriteChar(char obj) {
    Buffer::Write<uint8_t>(kDataType_Char);
    Buffer::Write<char>(obj);
  }

  void WriteInt8(int8_t obj) {
    Buffer::Write<uint8_t>(kDataType_Int8);
    Buffer::Write<int8_t>(obj);
  }

  void WriteUInt8(uint8_t obj) {
    Buffer::Write<uint8_t>(kDataType_UInt8);
    Buffer::Write<uint8_t>(obj);
  }

  void WriteInt16(int16_t obj) {
    Buffer::Write<uint8_t>(kDataType_Int16);
    Buffer::Write<int16_t>(obj);
  }

  void WriteUInt16(uint16_t obj) {
    Buffer::Write<uint8_t>(kDataType_UInt16);
    Buffer::Write<uint16_t>(obj);
  }

  void WriteInt32(int32_t obj) {
    Buffer::Write<uint8_t>(kDataType_Int32);
    Buffer::Write<int32_t>(obj);
  }

  void WriteUInt32(uint32_t obj) {
    Buffer::Write<uint8_t>(kDataType_UInt32);
    Buffer::Write<uint32_t>(obj);
  }

  void WriteInt64(int64_t obj) {
    Buffer::Write<uint8_t>(kDataType_Int64);
    Buffer::Write<int64_t>(obj);
  }

  void WriteUInt64(uint64_t obj) {
    Buffer::Write<uint8_t>(kDataType_UInt64);
    Buffer::Write<uint64_t>(obj);
  }

  void WriteFloat(float obj) {
    Buffer::Write<uint8_t>(kDataType_Float);
    Buffer::Write<float>(obj);
  }

  void WriteString(std::string obj) {
    Buffer::Write<uint8_t>(kDataType_String);
    size_t length = obj.size();
    Buffer::Write(length);
    Buffer::WriteArray<char>(obj.c_str(), length);
  }

  void WriteBlob(std::basic_string<uint8_t> obj) {
    Buffer::Write<uint8_t>(kDataType_Blob);
    size_t length = obj.size();
    Buffer::Write(length);
    Buffer::WriteArray<uint8_t>(obj.c_str(), length);
  }

  bool IsFlippingEndian() {
    return Buffer::IsFlippingEndian();
  }

  void SetFlipEndian(bool flip_endian) {
    Buffer::SetFlipEndian(flip_endian);
  }

  void Rewind() {
    Buffer::Rewind();
  }

  const uint8_t *GetBuffer() {
    return Buffer::GetBuffer();
  }

  size_t GetSize() {
    return Buffer::GetSize();
  }

  void Clear() {
    Buffer::Clear();
  }
};
}

#endif // jchat_common_typed_buffer_hpp_
