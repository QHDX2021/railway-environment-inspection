#pragma once

#include "linux/devices/device.hpp"

namespace rail {
class CameraDevice final : public ByteDevice {
 public:
  explicit CameraDevice(IByteTransport& transport)
      : ByteDevice(transport, {"camera", "hikvision-industrial", true}) {}
};
}  // namespace rail
