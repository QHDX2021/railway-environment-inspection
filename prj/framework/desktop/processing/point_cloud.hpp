#pragma once

#include "desktop/processing/algorithm_types.hpp"
#include "desktop/processing/coordinate_frames.hpp"
#include "desktop/processing/trajectory.hpp"

#include <vector>

namespace rail {

struct TimedPoint {
  Vec3 point;
  size_t pose_index{};
};

struct PointCloudRequest {
  std::string input_task_id;
  std::string calibration_version;
  std::vector<TimedPoint> points;
  std::vector<Pose> poses;
};

struct PointCloudResult {
  Status status;
  std::vector<Vec3> points;
  std::string input_task_id;
  std::string calibration_version;
};

PointCloudResult build_point_cloud(const PointCloudRequest&, const CancellationToken&, const ProcessingProgressCallback&);

}  // namespace rail
