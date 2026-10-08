#include "test_support.hpp"
#include "shared/types/time.hpp"

using namespace rail;

static void run() {
  TimeTag first{};
  first.ticks = 100;
  first.ticks_per_second = 1000;
  first.boot_id = 7;
  first.domain = ClockDomain::DeviceTicks;

  TimeTag second = first;
  second.ticks = 101;
  CHECK_EQ(compare_time(first, second), TimeComparison::Before);
  CHECK_EQ(compare_time(second, first), TimeComparison::After);

  second.ticks = first.ticks;
  CHECK_EQ(compare_time(first, second), TimeComparison::Equal);
  second.boot_id++;
  CHECK_EQ(compare_time(first, second), TimeComparison::Incomparable);
}

int main() { return test_main("time_model", run); }
