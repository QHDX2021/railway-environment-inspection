#pragma once

#include "linux/control/service.hpp"

#include <filesystem>
#include <functional>

namespace rail {

struct TaskInfo {
  std::string id;
  uint64_t data_bytes{};
  bool complete{};
};

struct Range {
  uint64_t offset{};
  uint64_t length{};
};

struct DownloadReport {
  Status status;
  uint64_t bytes{};
  uint64_t total_bytes{};
  uint32_t checksum{};
};

using DownloadSink = std::function<Status(ByteView)>;

class DownloadService {
 public:
  explicit DownloadService(std::filesystem::path root) : root_(std::move(root)) {}

  std::vector<TaskInfo> list_tasks() const;
  DownloadReport download(const std::string& task_id,
                          const std::string& file_id,
                          Range,
                          const DownloadSink&) const;

 private:
  std::filesystem::path root_;
};

class HttpServer {
 public:
  HttpServer(ControlService& control, DownloadService& downloads)
      : control_(control), downloads_(downloads) {}
  Status start(uint16_t port);
  Status stop();
  bool running() const { return running_; }

 private:
  ControlService& control_;
  DownloadService& downloads_;
  bool running_{};
  uint16_t port_{};
};

}  // namespace rail
