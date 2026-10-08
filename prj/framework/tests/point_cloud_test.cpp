#include "test_support.hpp"
#include "desktop/processing/point_cloud.hpp"

using namespace rail;

static void run() {
  PointCloudRequest request;
  request.input_task_id = "task-1";
  request.calibration_version = "calib-v1";
  request.points = {{{1, 0, 0}, 0}, {{0, 1, 0}, 1}};
  request.poses = {
      {{0, 1, 1, TimeQuality::Locked, ClockDomain::DeviceTicks}, {{1,0,0,0,1,0,0,0,1}, {10,0,0}}},
      {{1, 1, 1, TimeQuality::Locked, ClockDomain::DeviceTicks}, {{1,0,0,0,1,0,0,0,1}, {10,0,0}}},
  };
  auto result = build_point_cloud(request, {}, {});
  CHECK(result.status.ok());
  CHECK_EQ(result.points.size(), static_cast<size_t>(2));
  CHECK_EQ(result.points[0].x, 11.0);
  CHECK_EQ(result.points[1].x, 10.0);

  request.poses.clear();
  auto invalid = build_point_cloud(request, {}, {});
  CHECK_EQ(invalid.status.code, ErrorCode::InvalidArgument);
}

int main() { return test_main("point_cloud", run); }
