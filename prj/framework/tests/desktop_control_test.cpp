#include "test_support.hpp"
#include "desktop/control/device_client.hpp"
#include "desktop/download/task_downloader.hpp"

#include <filesystem>
#include <fstream>

using namespace rail;

static void run() {
  ParameterService parameters;
  parameters.define({"exposure", int64_t(10), 1, 100, "ms", false, ApplyMode::Immediate, false});
  ControlService service(parameters);
  DeviceClient client(service, 501);
  CHECK(client.acquire_control().ok());
  ClientCapabilities capabilities;
  CHECK(client.get_capabilities(capabilities).ok());
  CHECK(!capabilities.device_ids.empty());
  ParameterSnapshot snapshot;
  CHECK(client.get_parameters(snapshot).ok());
  auto changed = client.set_parameter("exposure", int64_t(20), snapshot.revision, 1);
  CHECK(changed.status.ok());
  CHECK(client.start_task("task-1", 2).ok());
  CHECK(client.stop_task("task-1", 3).ok());
  CHECK(client.release_control().ok());

  const auto root = std::filesystem::temp_directory_path() / "rail_desktop_download";
  std::filesystem::remove_all(root);
  std::filesystem::create_directories(root / "task-1");
  std::ofstream(root / "task-1" / "data.rrec", std::ios::binary) << "abcdef";
  std::ofstream(root / "task-1" / "complete") << "ok\n";
  DownloadService service_download(root);
  TaskDownloader downloader(service_download);
  const auto destination = root / "result.bin";
  uint64_t progress_bytes = 0;
  auto report = downloader.download("task-1", "data.rrec", destination, {},
                                    [&](const DownloadProgress& progress) { progress_bytes = progress.received; });
  CHECK(report.status.ok());
  CHECK_EQ(progress_bytes, 6u);
  CHECK_EQ(std::filesystem::file_size(destination), 6u);
  CHECK(!std::filesystem::exists(destination.string() + ".part"));

  std::atomic_bool cancelled{true};
  const auto cancelled_destination = root / "cancelled.bin";
  auto cancelled_report = downloader.download("task-1", "data.rrec", cancelled_destination,
                                              {&cancelled}, {});
  CHECK_EQ(cancelled_report.status.code, ErrorCode::Cancelled);
  CHECK(!std::filesystem::exists(cancelled_destination));
}

int main() { return test_main("desktop_control", run); }
