#include "desktop/download/task_downloader.hpp"

#include <fstream>

namespace rail {

DownloadReport TaskDownloader::download(const std::string& task_id,
                                        const std::string& file_id,
                                        const std::filesystem::path& destination,
                                        CancellationToken token,
                                        const ProgressCallback& progress) const {
  DownloadReport report;
  if (token.cancelled()) {
    report.status = Status::failure(ErrorCode::Cancelled, "download cancelled");
    return report;
  }
  std::error_code error;
  std::filesystem::create_directories(destination.parent_path(), error);
  if (error) {
    report.status = Status::failure(ErrorCode::IoError, error.message());
    return report;
  }
  const auto part = destination.string() + ".part";
  uint64_t existing = 0;
  const bool has_part = std::filesystem::exists(part, error);
  if (error) {
    report.status = Status::failure(ErrorCode::IoError, error.message());
    return report;
  }
  if (has_part) {
    existing = std::filesystem::file_size(part, error);
    if (error) {
      report.status = Status::failure(ErrorCode::IoError, "inspect partial download");
      return report;
    }
  }
  std::ofstream output(part, std::ios::binary |
                                (existing == 0 ? std::ios::trunc : std::ios::app));
  if (!output) {
    report.status = Status::failure(ErrorCode::IoError, "open temporary download");
    return report;
  }

  auto remote = service_.download(task_id, file_id, {existing, 0}, [&](ByteView chunk) {
    if (token.cancelled()) return Status::failure(ErrorCode::Cancelled, "download cancelled");
    output.write(reinterpret_cast<const char*>(chunk.data), static_cast<std::streamsize>(chunk.size));
    if (!output) return Status::failure(ErrorCode::IoError, "write temporary download");
    const auto position = output.tellp();
    if (position < 0) return Status::failure(ErrorCode::IoError, "inspect temporary download");
    if (progress) progress({static_cast<uint64_t>(position), 0});
    return Status::success();
  });
  output.close();
  if (!remote.status.ok()) {
    if (remote.status.code != ErrorCode::Cancelled && remote.status.code != ErrorCode::IoError) {
      std::filesystem::remove(part, error);
    }
    return remote;
  }
  if (remote.bytes + existing != remote.total_bytes) {
    std::filesystem::remove(part, error);
    remote.status = Status::failure(ErrorCode::CorruptData, "download length mismatch");
    return remote;
  }
  std::filesystem::remove(destination, error);
  error.clear();
  std::filesystem::rename(part, destination, error);
  if (error) {
    std::filesystem::remove(part, error);
    remote.status = Status::failure(ErrorCode::IoError, "commit downloaded file");
    return remote;
  }
  if (progress) progress({remote.total_bytes, remote.total_bytes});
  return remote;
}

}  // namespace rail
