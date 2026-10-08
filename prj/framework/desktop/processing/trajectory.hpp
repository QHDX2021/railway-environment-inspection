#pragma once

#include "desktop/processing/algorithm_types.hpp"
#include "desktop/processing/coordinate_frames.hpp"
#include "shared/types/time.hpp"

#include <vector>

namespace rail {

struct TrajectorySample {
  TimeTag time;
  Vec3 position;
};

struct Pose {
  TimeTag time;
  Transform body_to_world;
};

struct TrajectoryRequest {
  std::string input_task_id;
  std::string calibration_version;
  std::vector<TrajectorySample> samples;
};

struct TrajectoryResult {
  Status status;
  std::vector<Pose> poses;
  std::string input_task_id;
  std::string calibration_version;
};

TrajectoryResult solve_trajectory(const TrajectoryRequest&, const CancellationToken&, const ProcessingProgressCallback&);

}  // namespace rail
