#pragma once

#include "linux/control/control_lock.hpp"
#include "shared/configuration/parameters.hpp"

#include <set>

namespace rail {

struct ParameterWriteResult {
  Status status;
  uint64_t revision{};
  bool duplicate_request{};
};

class ControlService {
 public:
  explicit ControlService(ParameterService& parameters) : parameters_(parameters) {}

  Status acquire_control(uint64_t owner) { return lock_.acquire(owner); }
  Status release_control(uint64_t owner);
  ParameterWriteResult set_parameter(uint64_t owner, const std::string&, const ParameterValue&,
                                     uint64_t expected_revision, uint64_t request_id);
  ParameterWriteResult set_parameter(const std::string& name, const ParameterValue& value,
                                     uint64_t expected_revision, uint64_t request_id);
  Status start_task(uint64_t owner, const std::string& task_id, uint64_t request_id);
  Status start_task(const std::string& task_id, uint64_t request_id);
  Status stop_task(uint64_t owner, const std::string& task_id, uint64_t request_id);
  Status stop_task(const std::string& task_id, uint64_t request_id);
  bool task_running() const { return task_running_; }
  uint64_t revision() const { return parameters_.revision(); }
  std::map<std::string, ParameterValue> parameters_snapshot() const { return parameters_.snapshot(); }

 private:
  bool accept_request(uint64_t request_id);
  Status require_owner(uint64_t owner) const;

  ParameterService& parameters_;
  ControlLock lock_;
  std::set<uint64_t> request_ids_;
  bool task_running_{};
  std::string task_id_;
};

}  // namespace rail
