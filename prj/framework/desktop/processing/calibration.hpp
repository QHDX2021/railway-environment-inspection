#pragma once

#include "desktop/processing/algorithm_types.hpp"
#include "desktop/processing/coordinate_frames.hpp"

#include <filesystem>
#include <vector>

namespace rail {

struct CameraIntrinsics {
  double fx{};
  double fy{};
  double cx{};
  double cy{};
};

struct CalibrationData {
  std::string version;
  std::string device_set_version;
  std::string applicable_device_id;
  std::string coordinate_frame;
  CameraIntrinsics camera;
  Transform lidar_to_imu;
  Transform camera_to_imu;
  Vec3 dual_antenna_baseline;
  Vec3 lever_arm;
  double time_offset_ns{};
};

struct CalibrationRequest {
  CalibrationData candidate;
  std::filesystem::path output_path;
};

struct CalibrationResult {
  Status status;
  CalibrationData calibration;
  double residual{};
};

CalibrationResult calibrate(const CalibrationRequest&, const CancellationToken&, const ProcessingProgressCallback&);
Status save_calibration(const std::filesystem::path&, const CalibrationData&);
Status load_calibration(const std::filesystem::path&, CalibrationData&);

}  // namespace rail
