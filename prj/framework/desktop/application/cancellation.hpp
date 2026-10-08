#pragma once

#include <atomic>

namespace rail {

class CancellationToken {
 public:
  CancellationToken() = default;
  CancellationToken(const bool* flag) : bool_flag_(flag) {}
  CancellationToken(std::atomic_bool* flag) : atomic_flag_(flag) {}

  bool cancelled() const {
    return (bool_flag_ != nullptr && *bool_flag_) ||
           (atomic_flag_ != nullptr && atomic_flag_->load());
  }

 private:
  const bool* bool_flag_{};
  std::atomic_bool* atomic_flag_{};
};

}  // namespace rail
