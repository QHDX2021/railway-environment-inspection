#pragma once

#include "desktop/processing/algorithm_types.hpp"
#include "desktop/processing/coordinate_frames.hpp"

namespace rail {

struct ChangeRequest {
  std::string baseline_task_id;
  std::string current_task_id;
  double registration_quality{};
  double change_threshold{0.1};
  std::vector<Vec3> baseline_points;
  std::vector<Vec3> current_points;
};

struct ChangeCandidate {
  std::string baseline_task_id;
  std::string current_task_id;
  double registration_quality{};
  Vec3 region;
  double metric{};
  std::string review_state{"unreviewed"};
};

struct ChangeResult {
  Status status;
  std::vector<ChangeCandidate> candidates;
  std::string baseline_task_id;
  std::string current_task_id;
};

ChangeResult analyze_change(const ChangeRequest&, const CancellationToken&, const ProcessingProgressCallback&);

}  // namespace rail
