#pragma once

#include "shared/types/types.hpp"

namespace rail {

enum class TimeComparison : uint8_t {
  Before,
  Equal,
  After,
  Incomparable,
};

TimeComparison compare_time(const TimeTag& lhs, const TimeTag& rhs);

}  // namespace rail
