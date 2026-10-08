#include "test_support.hpp"
#include "desktop/processing/trajectory.hpp"

using namespace rail;

static void run() {
  TrajectoryRequest request;
  request.input_task_id = "task-1";
  request.calibration_version = "calib-v1";
  request.samples = {
      {{10, 1000, 7, TimeQuality::Locked, ClockDomain::GnssTime}, {0, 0, 0}},
      {{11, 1000, 7, TimeQuality::Locked, ClockDomain::GnssTime}, {1, 0, 0}},
  };
  auto result = solve_trajectory(request, {}, {});
  CHECK(result.status.ok());
  CHECK_EQ(result.poses.size(), static_cast<size_t>(2));
  CHECK_EQ(result.poses[1].body_to_world.translation.x, 1.0);

  request.samples[1].time.boot_id = 8;
  auto invalid = solve_trajectory(request, {}, {});
  CHECK_EQ(invalid.status.code, ErrorCode::InvalidArgument);
}

int main() { return test_main("trajectory", run); }
