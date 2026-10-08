#pragma once

#include "shared/types/types.hpp"

namespace rail {

class IPowerHal {
 public:
  virtual ~IPowerHal() = default;
  virtual bool external_power_present() const = 0;
  virtual Status request_shutdown() = 0;
  virtual Status cut_power() = 0;
};

}  // namespace rail
