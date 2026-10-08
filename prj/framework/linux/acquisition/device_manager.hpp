#pragma once

#include "linux/devices/device.hpp"

#include <vector>

namespace rail {

struct TaskConfig {
  std::string task_id;
  std::vector<std::string> device_ids;
};

class DeviceManager {
 public:
  Status add(IDevice& device);
  Status start_task(const TaskConfig&);
  Status stop_task();
  std::vector<DeviceHealth> health() const;
  bool running() const { return running_; }

 private:
  std::vector<IDevice*> devices_;
  bool running_{};
};

}  // namespace rail
