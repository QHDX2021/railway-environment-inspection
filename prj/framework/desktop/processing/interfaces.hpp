#pragma once
#include "shared/types/types.hpp"
#include <filesystem>
namespace rail {
struct ProcessingRequest {
  std::filesystem::path input, output;
  std::string task_id;
  std::string config_snapshot;
  std::string calibration_ref;
};
struct ProcessingResult {
  Status status;
  std::vector<std::filesystem::path> artifacts;
};
inline ProcessingResult run_not_supported(const ProcessingRequest&) {
  return {Status::failure(ErrorCode::NotSupported, "algorithm plugin not installed"), {}};
}
}  // namespace rail
