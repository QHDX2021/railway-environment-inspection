#pragma once

#include "linux/devices/device.hpp"

namespace rail {
class GnssDevice final : public ByteDevice {
 public:
  explicit GnssDevice(IByteTransport& transport)
      : ByteDevice(transport, {"gnss", "umd982-dual-antenna", true}) {}
};
}  // namespace rail
