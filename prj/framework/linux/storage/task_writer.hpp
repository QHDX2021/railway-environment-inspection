#pragma once
#include "shared/recording/codec.hpp"
#include <filesystem>
#include <fstream>
namespace rail {

Status read_task_metadata(const std::filesystem::path&, TaskMetadata&);

class IRecordingWriter {
 public:
  virtual ~IRecordingWriter() = default;
  virtual Status append(const Record&) = 0;
  virtual Status finish() = 0;
  virtual Status abort(ErrorCode) { return Status::success(); }
};

class TaskWriter : public IRecordingWriter {
  std::filesystem::path dir_;
  std::ofstream out_;
  TaskMetadata metadata_;
  bool finished_{};

 public:
  Status create(const std::filesystem::path&, const TaskMetadata&);
  Status append(const Record&) override;
  Status finish() override;
  Status abort(ErrorCode) override;
  bool finished() const { return finished_; }
};

}  // namespace rail
