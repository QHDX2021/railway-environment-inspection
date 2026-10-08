#pragma once

#include "desktop/application/cancellation.hpp"
#include "linux/control/http_server.hpp"

#include <functional>

namespace rail {

struct DownloadProgress {
  uint64_t received{};
  uint64_t total{};
};

using ProgressCallback = std::function<void(const DownloadProgress&)>;

class TaskDownloader {
 public:
  explicit TaskDownloader(DownloadService& service) : service_(service) {}

  DownloadReport download(const std::string& task_id,
                          const std::string& file_id,
                          const std::filesystem::path& destination,
                          CancellationToken,
                          const ProgressCallback&) const;

 private:
  DownloadService& service_;
};

}  // namespace rail
