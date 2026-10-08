#include "desktop/processing/point_cloud.hpp"

namespace rail {

PointCloudResult build_point_cloud(const PointCloudRequest& request,
                                   const CancellationToken& token,
                                   const ProcessingProgressCallback& progress) {
  PointCloudResult result;
  result.input_task_id = request.input_task_id;
  result.calibration_version = request.calibration_version;
  if (token.cancelled()) {
    result.status = Status::failure(ErrorCode::Cancelled, "point cloud cancelled");
    return result;
  }
  if (request.input_task_id.empty() || request.calibration_version.empty() || request.poses.empty()) {
    result.status = Status::failure(ErrorCode::InvalidArgument, "point cloud input incomplete");
    return result;
  }
  result.points.reserve(request.points.size());
  for (size_t i = 0; i < request.points.size(); ++i) {
    if (request.points[i].pose_index >= request.poses.size()) {
      result.points.clear();
      result.status = Status::failure(ErrorCode::InvalidArgument, "point pose missing");
      return result;
    }
    result.points.push_back(request.poses[request.points[i].pose_index].body_to_world.apply(request.points[i].point));
    if (progress) progress(request.points.empty() ? 1.0 : static_cast<double>(i + 1) / request.points.size());
  }
  result.status = Status::success();
  return result;
}

}  // namespace rail
