#include "linux/control/http_server.hpp"

#include "shared/protocol/frame.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>

namespace rail {
namespace {

bool valid_component(const std::string& value) {
  if (value.empty() || value.size() > 128) return false;
  return std::all_of(value.begin(), value.end(), [](unsigned char c) {
    return std::isalnum(c) || c == '-' || c == '_' || c == '.';
  });
}

}  // namespace

std::vector<TaskInfo> DownloadService::list_tasks() const {
  std::vector<TaskInfo> tasks;
  std::error_code error;
  if (!std::filesystem::exists(root_, error)) return tasks;
  for (const auto& entry : std::filesystem::directory_iterator(root_, error)) {
    if (error || !entry.is_directory()) continue;
    const auto data = entry.path() / "data.rrec";
    const auto complete = entry.path() / "complete";
    if (!std::filesystem::exists(data) || !std::filesystem::exists(complete)) continue;
    std::error_code size_error;
    const auto size = std::filesystem::file_size(data, size_error);
    if (!size_error) tasks.push_back({entry.path().filename().string(), size, true});
  }
  return tasks;
}

DownloadReport DownloadService::download(const std::string& task_id,
                                         const std::string& file_id,
                                         Range range,
                                         const DownloadSink& sink) const {
  DownloadReport report;
  if (!valid_component(task_id) ||
      (file_id != "data.rrec" && file_id != "task.meta" && file_id != "complete")) {
    report.status = Status::failure(ErrorCode::InvalidArgument, "invalid task or file id");
    return report;
  }
  const auto task_dir = root_ / task_id;
  const auto complete = task_dir / "complete";
  if (file_id == "data.rrec" && !std::filesystem::exists(complete)) {
    report.status = Status::failure(ErrorCode::Conflict, "task is still recording");
    return report;
  }
  const auto file = task_dir / file_id;
  std::error_code size_error;
  const uint64_t file_size = std::filesystem::file_size(file, size_error);
  if (size_error) {
    report.status = Status::failure(ErrorCode::IoError, "file unavailable");
    return report;
  }
  if (range.offset > file_size || range.length > file_size - range.offset) {
    report.status = Status::failure(ErrorCode::InvalidArgument, "range outside file");
    return report;
  }
  const uint64_t length = range.length == 0 ? file_size - range.offset : range.length;
  std::ifstream input(file, std::ios::binary);
  if (!input) {
    report.status = Status::failure(ErrorCode::IoError, "open download file");
    return report;
  }
  input.seekg(static_cast<std::streamoff>(range.offset));
  Bytes delivered;
  delivered.reserve(static_cast<size_t>(length));
  std::array<uint8_t, 64 * 1024> buffer{};
  uint64_t remaining = length;
  while (remaining > 0) {
    const auto chunk_size = static_cast<std::streamsize>(std::min<uint64_t>(remaining, buffer.size()));
    input.read(reinterpret_cast<char*>(buffer.data()), chunk_size);
    const auto actual = input.gcount();
    if (actual <= 0) {
      report.status = Status::failure(ErrorCode::IoError, "read download file");
      return report;
    }
    ByteView view{buffer.data(), static_cast<size_t>(actual)};
    if (sink) {
      auto sink_status = sink(view);
      if (!sink_status.ok()) {
        report.status = sink_status;
        return report;
      }
    }
    delivered.insert(delivered.end(), buffer.data(), buffer.data() + actual);
    report.bytes += static_cast<uint64_t>(actual);
    remaining -= static_cast<uint64_t>(actual);
  }
  report.total_bytes = file_size;
  report.checksum = crc32(delivered.data(), delivered.size());
  report.status = Status::success();
  return report;
}

Status HttpServer::start(uint16_t port) {
  if (port == 0) return Status::failure(ErrorCode::InvalidArgument, "invalid port");
  if (running_) return Status::failure(ErrorCode::Conflict, "server already running");
  port_ = port;
  running_ = true;
  return Status::success();
}

Status HttpServer::stop() {
  running_ = false;
  return Status::success();
}

}  // namespace rail
