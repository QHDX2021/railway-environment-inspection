#include "desktop/processing/trajectory.hpp"

namespace rail {

TrajectoryResult solve_trajectory(const TrajectoryRequest& request,
                                  const CancellationToken& token,
                                  const ProcessingProgressCallback& progress) {
  TrajectoryResult result;
  result.input_task_id = request.input_task_id;
  result.calibration_version = request.calibration_version;
  if (token.cancelled()) {
    result.status = Status::failure(ErrorCode::Cancelled, "trajectory cancelled");
    return result;
  }
  if (request.input_task_id.empty() || request.calibration_version.empty() || request.samples.empty()) {
    result.status = Status::failure(ErrorCode::InvalidArgument, "trajectory input incomplete");
    return result;
  }
  result.poses.reserve(request.samples.size());
  for (size_t i = 0; i < request.samples.size(); ++i) {
    const auto& sample = request.samples[i];
    if (sample.time.quality == TimeQuality::Invalid) {
      result.status = Status::failure(ErrorCode::InvalidArgument, "invalid time quality");
      result.poses.clear();
      return result;
    }
    if (i > 0) {
      const auto relation = compare_time(request.samples[i - 1].time, sample.time);
      if (relation == TimeComparison::Incomparable || relation == TimeComparison::After) {
        result.status = Status::failure(ErrorCode::InvalidArgument, "trajectory time sequence invalid");
        result.poses.clear();
        return result;
      }
    }
    Pose pose;
    pose.time = sample.time;
    pose.body_to_world.translation = sample.position;
    result.poses.push_back(pose);
    if (progress) progress(static_cast<double>(i + 1) / request.samples.size());
  }
  result.status = Status::success();
  return result;
}

}  // namespace rail
