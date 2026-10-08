#include "desktop/processing/change_analyzer.hpp"

#include <cmath>

namespace rail {

ChangeResult analyze_change(const ChangeRequest& request,
                            const CancellationToken& token,
                            const ProcessingProgressCallback& progress) {
  ChangeResult result;
  result.baseline_task_id = request.baseline_task_id;
  result.current_task_id = request.current_task_id;
  if (request.baseline_task_id.empty() || request.current_task_id.empty() ||
      request.registration_quality < 0.95 || request.baseline_points.size() != request.current_points.size()) {
    result.status = Status::failure(ErrorCode::InvalidArgument, "change input or registration quality invalid");
    return result;
  }
  if (token.cancelled()) {
    result.status = Status::failure(ErrorCode::Cancelled, "change analysis cancelled");
    return result;
  }
  for (size_t i = 0; i < request.baseline_points.size(); ++i) {
    const auto& before = request.baseline_points[i];
    const auto& after = request.current_points[i];
    const double dx = after.x - before.x;
    const double dy = after.y - before.y;
    const double dz = after.z - before.z;
    const double metric = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (metric > request.change_threshold) {
      result.candidates.push_back({request.baseline_task_id, request.current_task_id,
                                   request.registration_quality,
                                   {(before.x + after.x) * 0.5, (before.y + after.y) * 0.5,
                                    (before.z + after.z) * 0.5},
                                   metric, "unreviewed"});
    }
    if (progress) progress(request.baseline_points.empty() ? 1.0 : static_cast<double>(i + 1) / request.baseline_points.size());
  }
  result.status = Status::success();
  return result;
}

}  // namespace rail
