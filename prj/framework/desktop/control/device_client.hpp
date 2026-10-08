#pragma once

#include "linux/control/service.hpp"

namespace rail {

struct ClientCapabilities {
  std::vector<std::string> device_ids;
};

struct ParameterSnapshot {
  std::map<std::string, ParameterValue> values;
  uint64_t revision{};
};

class DeviceClient {
 public:
  DeviceClient(ControlService& service, uint64_t owner) : service_(service), owner_(owner) {}

  Status acquire_control();
  Status release_control();
  Status get_capabilities(ClientCapabilities&) const;
  Status get_parameters(ParameterSnapshot&) const;
  ParameterWriteResult set_parameter(const std::string&, const ParameterValue&,
                                     uint64_t expected_revision, uint64_t request_id);
  Status start_task(const std::string& task_id, uint64_t request_id);
  Status stop_task(const std::string& task_id, uint64_t request_id);

 private:
  ControlService& service_;
  uint64_t owner_{};
};

}  // namespace rail
