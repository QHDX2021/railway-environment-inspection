#include "linux/devices/device.hpp"

namespace rail {

Status ByteDevice::configure(const DeviceConfig& config) {
  if (!config.device_id.empty() && config.device_id != capabilities_.device_id) {
    return Status::failure(ErrorCode::InvalidArgument, "device id mismatch");
  }
  config_ = config;
  if (config_.device_id.empty()) config_.device_id = capabilities_.device_id;
  return Status::success();
}

Status ByteDevice::start() {
  if (health_.connected) return Status::failure(ErrorCode::Conflict, "device already started");
  TransportConfig transport_config;
  transport_config.endpoint = capabilities_.device_id;
  auto status = transport_.open(transport_config);
  if (!status.ok()) {
    health_.last_error = status.code;
    return status;
  }
  health_.device_id = capabilities_.device_id;
  health_.connected = true;
  health_.last_error = ErrorCode::Ok;
  return Status::success();
}

Status ByteDevice::stop() {
  if (!health_.connected) return Status::success();
  transport_.close();
  health_.connected = false;
  return Status::success();
}

}  // namespace rail
