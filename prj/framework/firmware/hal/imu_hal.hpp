#pragma once

#include "shared/types/types.hpp"

namespace rail {

struct ImuConfig {
  uint32_t sample_rate_hz{};
  uint32_t range_dps{};
};

struct ImuSample {
  uint64_t device_ticks{};
  uint64_t sequence{};
  int32_t gyro[3]{};
  int32_t accel[3]{};
  TimeQuality quality{TimeQuality::Unsynchronized};
};

class IImuHal {
 public:
  virtual ~IImuHal() = default;
  virtual Status configure(const ImuConfig&) = 0;
  virtual Status read_burst(ImuSample&) = 0;
  virtual bool data_ready() const = 0;
};

}  // namespace rail
