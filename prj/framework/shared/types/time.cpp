#include "shared/types/time.hpp"

namespace rail {

TimeComparison compare_time(const TimeTag& lhs, const TimeTag& rhs) {
  if (lhs.domain != rhs.domain || lhs.boot_id != rhs.boot_id ||
      lhs.ticks_per_second != rhs.ticks_per_second) {
    return TimeComparison::Incomparable;
  }
  if (lhs.ticks < rhs.ticks) return TimeComparison::Before;
  if (lhs.ticks > rhs.ticks) return TimeComparison::After;
  return TimeComparison::Equal;
}

}  // namespace rail
