#pragma once

#include "linux/devices/byte_transport.hpp"

namespace rail {

struct DeviceCapabilities {
  std::string device_id;
  std::string kind;
  bool hardware_timestamp{};
};

struct DeviceConfig {
  std::string device_id;
  uint32_t sample_rate_hz{};
  std::string parameters;
};

struct DeviceHealth {
  std::string device_id;
  bool connected{};
  uint64_t samples{};
  ErrorCode last_error{ErrorCode::Ok};
};

class IDevice {
 public:
  virtual ~IDevice() = default;
  virtual DeviceCapabilities capabilities() const = 0;
  virtual Status configure(const DeviceConfig&) = 0;
  virtual Status start() = 0;
  virtual Status stop() = 0;
  virtual DeviceHealth health() const = 0;
};

class ByteDevice : public IDevice {
 public:
  ByteDevice(IByteTransport& transport, DeviceCapabilities capabilities)
      : transport_(transport), capabilities_(std::move(capabilities)) {}

  DeviceCapabilities capabilities() const override { return capabilities_; }
  Status configure(const DeviceConfig&) override;
  Status start() override;
  Status stop() override;
  DeviceHealth health() const override { return health_; }

 protected:
  IByteTransport& transport_;
  DeviceCapabilities capabilities_;
  DeviceConfig config_;
  DeviceHealth health_;
};

}  // namespace rail
