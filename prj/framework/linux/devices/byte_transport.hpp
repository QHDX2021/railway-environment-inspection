#pragma once

#include "shared/types/types.hpp"

namespace rail {

struct TransportConfig {
  std::string endpoint;
  uint32_t baud_rate{};
  uint32_t timeout_ms{100};
};

class IByteTransport {
 public:
  virtual ~IByteTransport() = default;
  virtual Status open(const TransportConfig&) = 0;
  virtual Status read(MutableByteView) = 0;
  virtual Status write(ByteView) = 0;
  virtual void close() = 0;
};

}  // namespace rail
