#pragma once

#include "desktop/processing/algorithm_types.hpp"
#include "desktop/processing/coordinate_frames.hpp"

#include <filesystem>

namespace rail {

struct AiRequest {
  std::string input_task_id;
  std::filesystem::path input;
  std::filesystem::path output;
  std::string model_version;
  bool model_available{};
};

struct Detection {
  std::string class_name;
  double confidence{};
  std::string evidence_ref;
  TimeTag time;
  Vec3 position;
  std::string model_version;
};

struct AiResult {
  Status status;
  std::vector<Detection> detections;
  std::string input_task_id;
  std::string model_version;
};

AiResult run_ai(const AiRequest&, const CancellationToken&, const ProcessingProgressCallback&);

}  // namespace rail
