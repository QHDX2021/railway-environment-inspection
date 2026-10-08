#pragma once
#include "shared/types/types.hpp"
namespace rail {

struct Record {
  uint16_t type{};
  uint16_t stream{};
  std::string device_id;
  TimeTag time{};
  uint64_t sequence{};
  uint64_t host_monotonic_ns{};
  bool simulated{};
  Bytes payload;
};

enum class TaskState : uint8_t { Created, Recording, Completed, Failed, Cancelled };

struct TaskMetadata {
  std::string id;
  std::string device_set_version;
  std::string config_snapshot;
  std::string calibration_ref;
  bool simulated{};
  TaskState state{TaskState::Created};

  TaskMetadata() = default;
  TaskMetadata(std::string task_id, bool is_simulated)
      : id(std::move(task_id)), simulated(is_simulated) {}
  TaskMetadata(std::string task_id,
               std::string devices,
               std::string config,
               std::string calibration,
               bool is_simulated,
               TaskState task_state = TaskState::Created)
      : id(std::move(task_id)),
        device_set_version(std::move(devices)),
        config_snapshot(std::move(config)),
        calibration_ref(std::move(calibration)),
        simulated(is_simulated),
        state(task_state) {}
};

}  // namespace rail
