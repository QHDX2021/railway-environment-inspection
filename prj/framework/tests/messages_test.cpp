#include "test_support.hpp"
#include "shared/protocol/messages.hpp"
#include <array>

using namespace rail;

static void run() {
  Message original{MessageType::ImuData, 42, {1, 2, 0xff}};
  std::array<uint8_t, 32> wire{};
  size_t written = 0;
  CHECK_EQ(encode_message(original, {wire.data(), wire.size()}, written), ErrorCode::Ok);
  CHECK_EQ(written, static_cast<size_t>(11));

  Message decoded;
  CHECK_EQ(decode_message({wire.data(), written}, decoded), ErrorCode::Ok);
  CHECK_EQ(decoded.type, MessageType::ImuData);
  CHECK_EQ(decoded.request_id, 42u);
  CHECK_EQ(decoded.payload.size(), static_cast<size_t>(3));
  CHECK_EQ(decoded.payload[2], static_cast<uint8_t>(0xff));

  CHECK_EQ(decode_message({wire.data(), 7}, decoded), ErrorCode::CorruptData);
  std::array<uint8_t, 4> too_small{};
  CHECK_EQ(encode_message(original, {too_small.data(), too_small.size()}, written), ErrorCode::Overflow);
}

int main() { return test_main("messages", run); }
