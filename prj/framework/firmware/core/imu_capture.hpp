#pragma once

#include "firmware/hal/imu_hal.hpp"

#include <array>

namespace rail {

struct ImuCaptureStats {
  uint64_t overflow_count{};
  uint64_t read_error_count{};
  uint64_t queued_count{};
};

class ImuCapture {
 public:
  static constexpr size_t MaxCapacity = 256;

  explicit ImuCapture(IImuHal& imu, size_t capacity = MaxCapacity)
      : imu_(imu), capacity_(capacity == 0 ? 1 : (capacity > MaxCapacity ? MaxCapacity : capacity)) {}

  Status configure(const ImuConfig& config) { return imu_.configure(config); }
  Status on_data_ready();
  bool pop(ImuSample&);
  const ImuCaptureStats& stats() const { return stats_; }

 private:
  IImuHal& imu_;
  std::array<ImuSample, MaxCapacity> queue_{};
  size_t capacity_{};
  size_t head_{};
  size_t size_{};
  uint64_t next_sequence_{};
  ImuCaptureStats stats_{};
};

}  // namespace rail
