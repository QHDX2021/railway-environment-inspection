#pragma once

#include "shared/types/types.hpp"

namespace rail {

enum class MessageType : uint16_t {
  Capabilities = 1,
  Status = 2,
  ParameterGet = 3,
  ParameterSet = 4,
  TaskStart = 5,
  TaskStop = 6,
  ImuData = 7,
  PpsMapping = 8,
  Trigger = 9,
  PowerEvent = 10,
  Shutdown = 11,
  Error = 12,
};

struct Message {
  MessageType type{MessageType::Error};
  uint32_t request_id{};
  Bytes payload;
};

ErrorCode encode_message(const Message& message, MutableByteView output, size_t& written);
ErrorCode decode_message(ByteView input, Message& message);

}  // namespace rail
