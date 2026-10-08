#pragma once
#include "shared/types/types.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>

namespace rail {
struct FrameView { uint16_t type{}; uint32_t sequence{}; ByteView payload{}; };
uint32_t crc32(const uint8_t* data, size_t size);
ErrorCode encode_frame(FrameView frame, MutableByteView output, size_t& written);
class FrameDecoder {
 public:
  using Callback = void(*)(void*, FrameView);
  void feed(ByteView input, void* context, Callback callback);
  size_t crc_errors() const { return crc_errors_; }
  size_t protocol_errors() const { return protocol_errors_; }
 private:
  static constexpr size_t MaxFrame = 2 + 1 + 1 + 2 + 4 + 2 + 4096 + 4;
  uint8_t buffer_[MaxFrame]{}; size_t size_{}; size_t crc_errors_{}; size_t protocol_errors_{};
};
}
