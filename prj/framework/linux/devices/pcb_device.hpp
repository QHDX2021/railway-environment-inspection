#pragma once

#include "linux/devices/device.hpp"

namespace rail {
class PcbDevice final : public ByteDevice {
 public:
  explicit PcbDevice(IByteTransport& transport)
      : ByteDevice(transport, {"pcb", "imu-sync-power", true}) {}
};
}  // namespace rail
