#include "desktop/processing/calibration.hpp"

#include <fstream>
#include <map>
#include <stdexcept>
#include <vector>

namespace rail {
namespace {

void write_transform(std::ostream& out, const char* name, const Transform& transform) {
  out << name << ".rotation=";
  for (size_t i = 0; i < transform.rotation.size(); ++i) out << (i ? "," : "") << transform.rotation[i];
  out << '\n' << name << ".translation=" << transform.translation.x << ','
      << transform.translation.y << ',' << transform.translation.z << '\n';
}

bool parse_values(const std::string& text, size_t count, std::vector<double>& values) {
  values.clear();
  size_t begin = 0;
  while (begin <= text.size()) {
    const size_t end = text.find(',', begin);
    try {
      values.push_back(std::stod(text.substr(begin, end == std::string::npos ? end : end - begin)));
    } catch (...) {
      return false;
    }
    if (end == std::string::npos) break;
    begin = end + 1;
  }
  return values.size() == count;
}

bool load_transform(const std::map<std::string, std::string>& values, const char* name, Transform& transform) {
  std::vector<double> parsed;
  if (!parse_values(values.at(std::string(name) + ".rotation"), 9, parsed)) return false;
  for (size_t i = 0; i < 9; ++i) transform.rotation[i] = parsed[i];
  if (!parse_values(values.at(std::string(name) + ".translation"), 3, parsed)) return false;
  transform.translation = {parsed[0], parsed[1], parsed[2]};
  return true;
}

}  // namespace

Status save_calibration(const std::filesystem::path& path, const CalibrationData& data) {
  std::ofstream out(path, std::ios::trunc);
  if (!out) return Status::failure(ErrorCode::IoError, "open calibration");
  out << "version=" << data.version << '\n'
      << "device_set_version=" << data.device_set_version << '\n'
      << "applicable_device_id=" << data.applicable_device_id << '\n'
      << "coordinate_frame=" << data.coordinate_frame << '\n'
      << "camera=" << data.camera.fx << ',' << data.camera.fy << ',' << data.camera.cx << ',' << data.camera.cy << '\n'
      << "dual_antenna_baseline=" << data.dual_antenna_baseline.x << ',' << data.dual_antenna_baseline.y << ',' << data.dual_antenna_baseline.z << '\n'
      << "lever_arm=" << data.lever_arm.x << ',' << data.lever_arm.y << ',' << data.lever_arm.z << '\n'
      << "time_offset_ns=" << data.time_offset_ns << '\n';
  write_transform(out, "lidar_to_imu", data.lidar_to_imu);
  write_transform(out, "camera_to_imu", data.camera_to_imu);
  return out.good() ? Status::success() : Status::failure(ErrorCode::IoError, "write calibration");
}

Status load_calibration(const std::filesystem::path& path, CalibrationData& data) {
  std::ifstream in(path);
  if (!in) return Status::failure(ErrorCode::IoError, "open calibration");
  std::map<std::string, std::string> values;
  std::string line;
  while (std::getline(in, line)) {
    const size_t split = line.find('=');
    if (split == std::string::npos) return Status::failure(ErrorCode::CorruptData, "bad calibration line");
    values[line.substr(0, split)] = line.substr(split + 1);
  }
  try {
    data.version = values.at("version");
    data.device_set_version = values.at("device_set_version");
    data.applicable_device_id = values.at("applicable_device_id");
    data.coordinate_frame = values.at("coordinate_frame");
    std::vector<double> parsed;
    if (!parse_values(values.at("camera"), 4, parsed)) throw std::runtime_error("camera");
    data.camera = {parsed[0], parsed[1], parsed[2], parsed[3]};
    if (!parse_values(values.at("dual_antenna_baseline"), 3, parsed)) throw std::runtime_error("baseline");
    data.dual_antenna_baseline = {parsed[0], parsed[1], parsed[2]};
    if (!parse_values(values.at("lever_arm"), 3, parsed)) throw std::runtime_error("lever arm");
    data.lever_arm = {parsed[0], parsed[1], parsed[2]};
    data.time_offset_ns = std::stod(values.at("time_offset_ns"));
    if (!load_transform(values, "lidar_to_imu", data.lidar_to_imu) ||
        !load_transform(values, "camera_to_imu", data.camera_to_imu)) throw std::runtime_error("transform");
  } catch (...) {
    return Status::failure(ErrorCode::CorruptData, "invalid calibration");
  }
  return Status::success();
}

CalibrationResult calibrate(const CalibrationRequest& request,
                            const CancellationToken& token,
                            const ProcessingProgressCallback& progress) {
  CalibrationResult result;
  if (token.cancelled()) {
    result.status = Status::failure(ErrorCode::Cancelled, "calibration cancelled");
    return result;
  }
  const auto& data = request.candidate;
  if (data.version.empty() || data.device_set_version.empty() || data.applicable_device_id.empty() ||
      data.coordinate_frame.empty() || request.output_path.empty()) {
    result.status = Status::failure(ErrorCode::InvalidArgument, "calibration metadata incomplete");
    return result;
  }
  result.status = save_calibration(request.output_path, data);
  if (result.status.ok()) {
    result.calibration = data;
    if (progress) progress(1.0);
  }
  return result;
}

}  // namespace rail
