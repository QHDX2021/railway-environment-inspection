#include "linux/acquisition/device_manager.hpp"

#include <algorithm>

namespace rail {
namespace {
bool selected(const TaskConfig& config, const std::string& id) {
  return config.device_ids.empty() ||
         std::find(config.device_ids.begin(), config.device_ids.end(), id) != config.device_ids.end();
}
}  // namespace

Status DeviceManager::add(IDevice& device) {
  const auto id = device.capabilities().device_id;
  if (id.empty()) return Status::failure(ErrorCode::InvalidArgument, "device id is empty");
  for (auto* existing : devices_) {
    if (existing->capabilities().device_id == id) {
      return Status::failure(ErrorCode::Conflict, "duplicate device");
    }
  }
  devices_.push_back(&device);
  return Status::success();
}

Status DeviceManager::start_task(const TaskConfig& config) {
  if (config.task_id.empty()) return Status::failure(ErrorCode::InvalidArgument, "empty task id");
  if (running_) return Status::failure(ErrorCode::Conflict, "task already running");

  size_t selected_count = 0;
  for (auto* device : devices_) {
    if (!selected(config, device->capabilities().device_id)) continue;
    ++selected_count;
    auto status = device->start();
    if (!status.ok()) {
      for (auto* rollback : devices_) rollback->stop();
      return status;
    }
  }
  if (selected_count == 0) return Status::failure(ErrorCode::InvalidArgument, "no selected device");
  running_ = true;
  return Status::success();
}

Status DeviceManager::stop_task() {
  if (!running_) return Status::success();
  Status result = Status::success();
  for (auto* device : devices_) {
    auto status = device->stop();
    if (!status.ok() && result.ok()) result = status;
  }
  running_ = false;
  return result;
}

std::vector<DeviceHealth> DeviceManager::health() const {
  std::vector<DeviceHealth> result;
  result.reserve(devices_.size());
  for (auto* device : devices_) result.push_back(device->health());
  return result;
}

}  // namespace rail
