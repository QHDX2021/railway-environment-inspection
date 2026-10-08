#include "linux/control/service.hpp"

namespace rail {

bool ControlService::accept_request(uint64_t request_id) {
  if (request_id == 0) return false;
  return request_ids_.insert(request_id).second;
}

Status ControlService::require_owner(uint64_t owner) const {
  return lock_.owns(owner) ? Status::success()
                           : Status::failure(ErrorCode::Conflict, "control lock required");
}

Status ControlService::release_control(uint64_t owner) {
  if (task_running_) return Status::failure(ErrorCode::InvalidState, "stop task before release");
  return lock_.release(owner);
}

ParameterWriteResult ControlService::set_parameter(uint64_t owner,
                                                   const std::string& name,
                                                   const ParameterValue& value,
                                                   uint64_t expected_revision,
                                                   uint64_t request_id) {
  ParameterWriteResult result;
  result.revision = parameters_.revision();
  auto owner_status = require_owner(owner);
  if (!owner_status.ok()) {
    result.status = owner_status;
    return result;
  }
  if (!accept_request(request_id)) {
    result.duplicate_request = true;
    result.status = Status::failure(ErrorCode::Conflict, "duplicate request");
    return result;
  }
  result.status = parameters_.set(name, value, expected_revision);
  result.revision = parameters_.revision();
  return result;
}

ParameterWriteResult ControlService::set_parameter(const std::string& name,
                                                   const ParameterValue& value,
                                                   uint64_t expected_revision,
                                                   uint64_t request_id) {
  if (!lock_.locked()) {
    return {Status::failure(ErrorCode::Conflict, "control lock required"), parameters_.revision(), false};
  }
  return set_parameter(lock_.owner(), name, value, expected_revision, request_id);
}

Status ControlService::start_task(uint64_t owner, const std::string& task_id, uint64_t request_id) {
  auto owner_status = require_owner(owner);
  if (!owner_status.ok()) return owner_status;
  if (task_id.empty()) return Status::failure(ErrorCode::InvalidArgument, "empty task id");
  if (!accept_request(request_id)) return Status::failure(ErrorCode::Conflict, "duplicate request");
  if (task_running_) return Status::failure(ErrorCode::Conflict, "task already running");
  task_id_ = task_id;
  task_running_ = true;
  return Status::success();
}

Status ControlService::start_task(const std::string& task_id, uint64_t request_id) {
  if (!lock_.locked()) return Status::failure(ErrorCode::Conflict, "control lock required");
  return start_task(lock_.owner(), task_id, request_id);
}

Status ControlService::stop_task(uint64_t owner, const std::string& task_id, uint64_t request_id) {
  auto owner_status = require_owner(owner);
  if (!owner_status.ok()) return owner_status;
  if (!accept_request(request_id)) return Status::failure(ErrorCode::Conflict, "duplicate request");
  if (!task_running_ || task_id != task_id_) {
    return Status::failure(ErrorCode::InvalidState, "task is not running");
  }
  task_running_ = false;
  task_id_.clear();
  return Status::success();
}

Status ControlService::stop_task(const std::string& task_id, uint64_t request_id) {
  if (!lock_.locked()) return Status::failure(ErrorCode::Conflict, "control lock required");
  return stop_task(lock_.owner(), task_id, request_id);
}

}  // namespace rail
