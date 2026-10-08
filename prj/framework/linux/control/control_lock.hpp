#pragma once

#include "shared/types/types.hpp"

namespace rail {

class ControlLock {
 public:
  Status acquire(uint64_t owner);
  Status release(uint64_t owner);
  bool owns(uint64_t owner) const { return locked_ && owner_ == owner; }
  bool locked() const { return locked_; }
  uint64_t owner() const { return owner_; }

 private:
  bool locked_{};
  uint64_t owner_{};
};

}  // namespace rail
