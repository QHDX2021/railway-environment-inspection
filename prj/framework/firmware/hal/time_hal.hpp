#pragma once

#include "shared/types/types.hpp"

namespace rail {

class ITimeHal {
 public:
  virtual ~ITimeHal() = default;
  virtual uint64_t capture_pps() = 0;
  virtual Status map_second(uint64_t pps_tick, uint64_t device_tick) = 0;
};

}  // namespace rail
