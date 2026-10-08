#include "shared/protocol/messages.hpp"

#include <algorithm>
#include <cstring>

namespace rail {
namespace {

constexpr size_t kHeaderSize = 8;
constexpr size_t kMaxPayload = 4096;

void put16(uint8_t* p, uint16_t value) {
  p[0] = static_cast<uint8_t>(value);
  p[1] = static_cast<uint8_t>(value >> 8);
}

void put32(uint8_t* p, uint32_t value) {
  for (int i = 0; i < 4; ++i) p[i] = static_cast<uint8_t>(value >> (8 * i));
}

uint16_t get16(const uint8_t* p) {
  return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
}

uint32_t get32(const uint8_t* p) {
  uint32_t value = 0;
  for (int i = 0; i < 4; ++i) value |= static_cast<uint32_t>(p[i]) << (8 * i);
  return value;
}

}  // namespace

ErrorCode encode_message(const Message& message, MutableByteView output, size_t& written) {
  written = 0;
  if (message.payload.size() > kMaxPayload) return ErrorCode::Overflow;
  const size_t total = kHeaderSize + message.payload.size();
  if (output.data == nullptr || output.size < total) return ErrorCode::Overflow;

  put16(output.data, static_cast<uint16_t>(message.type));
  put32(output.data + 2, message.request_id);
  put16(output.data + 6, static_cast<uint16_t>(message.payload.size()));
  if (!message.payload.empty()) {
    std::memcpy(output.data + kHeaderSize, message.payload.data(), message.payload.size());
  }
  written = total;
  return ErrorCode::Ok;
}

ErrorCode decode_message(ByteView input, Message& message) {
  if (input.data == nullptr || input.size < kHeaderSize) return ErrorCode::CorruptData;
  const size_t payload_size = get16(input.data + 6);
  if (payload_size > kMaxPayload || input.size != kHeaderSize + payload_size) {
    return ErrorCode::CorruptData;
  }
  message.type = static_cast<MessageType>(get16(input.data));
  message.request_id = get32(input.data + 2);
  message.payload.assign(input.data + kHeaderSize, input.data + input.size);
  return ErrorCode::Ok;
}

}  // namespace rail
