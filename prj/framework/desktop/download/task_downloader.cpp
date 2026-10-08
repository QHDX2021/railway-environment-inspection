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
  std::ofstream output(part, std::ios::binary | std::ios::trunc);
  if (!output) {
    report.status = Status::failure(ErrorCode::IoError, "open temporary download");
    return report;
  }

  auto remote = service_.download(task_id, file_id, {}, [&](ByteView chunk) {
    if (token.cancelled()) return Status::failure(ErrorCode::Cancelled, "download cancelled");
    output.write(reinterpret_cast<const char*>(chunk.data), static_cast<std::streamsize>(chunk.size));
    if (!output) return Status::failure(ErrorCode::IoError, "write temporary download");
    if (progress) progress({static_cast<uint64_t>(output.tellp()), 0});
    return Status::success();
  });
  output.close();
  if (!remote.status.ok()) {
    std::filesystem::remove(part, error);
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
  if (progress) progress({remote.bytes, remote.bytes});
  return remote;
}

}  // namespace rail
