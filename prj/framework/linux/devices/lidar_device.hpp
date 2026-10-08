#pragma once

#include "linux/devices/device.hpp"

namespace rail {
class LidarDevice final : public ByteDevice {
 public:
  explicit LidarDevice(IByteTransport& transport)
      : ByteDevice(transport, {"lidar", "xt32", true}) {}
};
}  // namespace rail
