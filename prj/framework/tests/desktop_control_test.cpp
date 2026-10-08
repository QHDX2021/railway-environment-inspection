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

  const auto resumed_destination = root / "resumed.bin";
  const auto resumed_part = std::filesystem::path(resumed_destination.string() + ".part");
  std::ofstream(resumed_part, std::ios::binary) << "ab";
  auto resumed_report = downloader.download("task-1", "data.rrec", resumed_destination, {}, {});
  CHECK(resumed_report.status.ok());
  CHECK_EQ(resumed_report.bytes, 4u);
  CHECK_EQ(resumed_report.total_bytes, 6u);
  std::ifstream resumed_input(resumed_destination, std::ios::binary);
  CHECK_EQ(std::string((std::istreambuf_iterator<char>(resumed_input)), {}), std::string("abcdef"));

  std::filesystem::create_directories(root / "task-2");
  const std::string large_data(100000, 'z');
  std::ofstream(root / "task-2" / "data.rrec", std::ios::binary).write(large_data.data(),
                                                                           static_cast<std::streamsize>(large_data.size()));
  std::ofstream(root / "task-2" / "complete") << "ok\n";
  const auto interrupted_destination = root / "interrupted.bin";
  std::atomic_bool interrupt{false};
  auto interrupted_report = downloader.download(
      "task-2", "data.rrec", interrupted_destination, {&interrupt},
      [&](const DownloadProgress& progress) {
        if (progress.received > 0) interrupt.store(true);
      });
  CHECK_EQ(interrupted_report.status.code, ErrorCode::Cancelled);
  CHECK(std::filesystem::exists(interrupted_destination.string() + ".part"));
  CHECK(std::filesystem::file_size(interrupted_destination.string() + ".part") > 0);

  std::atomic_bool resumed_cancel{false};
  auto completed_report = downloader.download("task-2", "data.rrec", interrupted_destination,
                                              {&resumed_cancel}, {});
  CHECK(completed_report.status.ok());
  CHECK_EQ(std::filesystem::file_size(interrupted_destination), large_data.size());

  std::atomic_bool cancelled{true};
  const auto cancelled_destination = root / "cancelled.bin";
  auto cancelled_report = downloader.download("task-1", "data.rrec", cancelled_destination,
                                              {&cancelled}, {});
  CHECK_EQ(cancelled_report.status.code, ErrorCode::Cancelled);
  CHECK(!std::filesystem::exists(cancelled_destination));
}

int main() { return test_main("desktop_control", run); }
