#include "desktop/control/device_client.hpp"

namespace rail {

Status DeviceClient::acquire_control() { return service_.acquire_control(owner_); }
Status DeviceClient::release_control() { return service_.release_control(owner_); }

Status DeviceClient::get_capabilities(ClientCapabilities& capabilities) const {
  capabilities.device_ids = {"pcb", "gnss", "lidar", "camera"};
  return Status::success();
}

Status DeviceClient::get_parameters(ParameterSnapshot& snapshot) const {
  snapshot.values = service_.parameters_snapshot();
  snapshot.revision = service_.revision();
  return Status::success();
}

ParameterWriteResult DeviceClient::set_parameter(const std::string& name,
                                                 const ParameterValue& value,
                                                 uint64_t expected_revision,
                                                 uint64_t request_id) {
  return service_.set_parameter(owner_, name, value, expected_revision, request_id);
}

Status DeviceClient::start_task(const std::string& task_id, uint64_t request_id) {
  return service_.start_task(owner_, task_id, request_id);
}

Status DeviceClient::stop_task(const std::string& task_id, uint64_t request_id) {
  return service_.stop_task(owner_, task_id, request_id);
}

}  // namespace rail
