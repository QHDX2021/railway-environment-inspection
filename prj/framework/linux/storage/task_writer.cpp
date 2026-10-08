#include "linux/storage/task_writer.hpp"

#include <fstream>

namespace rail {
namespace {

const char* state_name(TaskState state) {
  switch (state) {
    case TaskState::Created: return "created";
    case TaskState::Recording: return "recording";
    case TaskState::Completed: return "completed";
    case TaskState::Failed: return "failed";
    case TaskState::Cancelled: return "cancelled";
  }
  return "created";
}

bool parse_state(const std::string& value, TaskState& state) {
  if (value == "created") state = TaskState::Created;
  else if (value == "recording") state = TaskState::Recording;
  else if (value == "completed") state = TaskState::Completed;
  else if (value == "failed") state = TaskState::Failed;
  else if (value == "cancelled") state = TaskState::Cancelled;
  else return false;
  return true;
}

Status write_metadata(const std::filesystem::path& dir, const TaskMetadata& metadata) {
  std::ofstream meta(dir / "task.meta", std::ios::binary | std::ios::trunc);
  if (!meta) return Status::failure(ErrorCode::IoError, "open metadata");
  meta << "id=" << metadata.id << '\n'
       << "simulated=" << (metadata.simulated ? 1 : 0) << '\n'
       << "device_set_version=" << metadata.device_set_version << '\n'
       << "config_snapshot=" << metadata.config_snapshot << '\n'
       << "calibration_ref=" << metadata.calibration_ref << '\n'
       << "state=" << state_name(metadata.state) << '\n';
  return meta.good() ? Status::success()
                     : Status::failure(ErrorCode::IoError, "write metadata");
}

}  // namespace

Status read_task_metadata(const std::filesystem::path& dir, TaskMetadata& metadata) {
  std::ifstream input(dir / "task.meta", std::ios::binary);
  if (!input) return Status::failure(ErrorCode::IoError, "open metadata");

  TaskMetadata parsed;
  std::string line;
  while (std::getline(input, line)) {
    const auto split = line.find('=');
    if (split == std::string::npos) return Status::failure(ErrorCode::CorruptData, "bad metadata");
    const std::string key = line.substr(0, split);
    const std::string value = line.substr(split + 1);
    if (key == "id") parsed.id = value;
    else if (key == "simulated") parsed.simulated = value == "1";
    else if (key == "device_set_version") parsed.device_set_version = value;
    else if (key == "config_snapshot") parsed.config_snapshot = value;
    else if (key == "calibration_ref") parsed.calibration_ref = value;
    else if (key == "state" && !parse_state(value, parsed.state)) {
      return Status::failure(ErrorCode::CorruptData, "unknown task state");
    }
  }
  if (!input.eof() || parsed.id.empty()) return Status::failure(ErrorCode::CorruptData, "incomplete metadata");
  metadata = std::move(parsed);
  return Status::success();
}

Status TaskWriter::create(const std::filesystem::path& path, const TaskMetadata& metadata) {
  if (metadata.id.empty()) return Status::failure(ErrorCode::InvalidArgument, "empty task id");
  if (std::filesystem::exists(path)) return Status::failure(ErrorCode::Conflict, "directory exists");

  std::error_code ec;
  std::filesystem::create_directories(path, ec);
  if (ec) return Status::failure(ErrorCode::IoError, ec.message());

  metadata_ = metadata;
  metadata_.state = TaskState::Recording;
  auto metadata_status = write_metadata(path, metadata_);
  if (!metadata_status.ok()) return metadata_status;

  out_.open(path / "data.rrec", std::ios::binary);
  if (!out_) return Status::failure(ErrorCode::IoError, "open data");
  dir_ = path;
  finished_ = false;
  return Status::success();
}

Status TaskWriter::append(const Record& record) {
  if (!out_ || finished_) return Status::failure(ErrorCode::InvalidState, "writer closed");
  auto status = write_record(out_, record);
  if (!status.ok()) return status;
  return out_.good() ? Status::success()
                     : Status::failure(ErrorCode::IoError, "write failed");
}

Status TaskWriter::finish() {
  if (finished_) return Status::success();
  if (!out_) return Status::failure(ErrorCode::InvalidState, "writer closed");
  out_.flush();
  if (!out_) return Status::failure(ErrorCode::IoError, "flush failed");
  out_.close();

  metadata_.state = TaskState::Completed;
  auto metadata_status = write_metadata(dir_, metadata_);
  if (!metadata_status.ok()) return metadata_status;

  std::ofstream complete(dir_ / "complete", std::ios::binary | std::ios::trunc);
  complete << "ok\n";
  if (!complete.good()) return Status::failure(ErrorCode::IoError, "complete failed");
  finished_ = true;
  return Status::success();
}

Status TaskWriter::abort(ErrorCode reason) {
  if (finished_) return Status::failure(ErrorCode::InvalidState, "writer already finished");
  if (out_) {
    out_.flush();
    out_.close();
  }
  metadata_.state = reason == ErrorCode::Cancelled ? TaskState::Cancelled : TaskState::Failed;
  return write_metadata(dir_, metadata_);
}

}  // namespace rail
