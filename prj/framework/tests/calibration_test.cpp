#include "test_support.hpp"
#include "desktop/processing/calibration.hpp"
#include "desktop/processing/coordinate_frames.hpp"

#include <filesystem>

using namespace rail;

static void run() {
  CalibrationData input;
  input.version = "calib-v1";
  input.device_set_version = "devices-v1";
  input.applicable_device_id = "vehicle-1";
  input.coordinate_frame = "enu";
  input.camera.fx = 1000;
  input.camera.fy = 1001;
  input.camera.cx = 640;
  input.camera.cy = 360;
  input.lidar_to_imu.translation = {1, 2, 3};
  input.dual_antenna_baseline = {0, 1, 0};
  input.time_offset_ns = 1250;

  const auto path = std::filesystem::temp_directory_path() / "rail_calibration.txt";
  auto result = calibrate({input, path}, {}, {});
  CHECK(result.status.ok());
  CalibrationData loaded;
  CHECK(load_calibration(path, loaded).ok());
  CHECK_EQ(loaded.version, std::string("calib-v1"));
  CHECK_EQ(loaded.lidar_to_imu.translation.x, 1.0);
  CHECK_EQ(loaded.dual_antenna_baseline.y, 1.0);

  auto invalid = calibrate({CalibrationData{}, path}, {}, {});
  CHECK_EQ(invalid.status.code, ErrorCode::InvalidArgument);
}

int main() { return test_main("calibration", run); }
