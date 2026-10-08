#include "firmware/core/imu_capture.hpp"

namespace rail {

Status ImuCapture::on_data_ready() {
  if (!imu_.data_ready()) return Status::success();

  ImuSample sample;
  auto status = imu_.read_burst(sample);
  if (!status.ok()) {
    ++stats_.read_error_count;
    return status;
  }
  sample.sequence = next_sequence_++;

  if (size_ >= capacity_) {
    ++stats_.overflow_count;
    return Status::failure(ErrorCode::Overflow, "IMU capture queue full");
  }
  const size_t slot = (head_ + size_) % capacity_;
  queue_[slot] = sample;
  ++size_;
  ++stats_.queued_count;
  return Status::success();
}

bool ImuCapture::pop(ImuSample& sample) {
  if (size_ == 0) return false;
  sample = queue_[head_];
  head_ = (head_ + 1) % capacity_;
  --size_;
  return true;
}

}  // namespace rail
