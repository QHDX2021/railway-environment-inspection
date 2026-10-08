#include "test_support.hpp"
#include "linux/control/control_lock.hpp"
#include "linux/control/http_server.hpp"
#include "linux/control/service.hpp"

#include <filesystem>
#include <fstream>

using namespace rail;

static void run() {
  ParameterService parameters;
  parameters.define({"gain", int64_t(1), 0, 10, "", false, ApplyMode::Immediate, false});
  ControlService service(parameters);
  CHECK(service.acquire_control(100).ok());
  auto changed = service.set_parameter(100, "gain", int64_t(5), 0, 1);
  CHECK(changed.status.ok());
  CHECK_EQ(changed.revision, 1u);
  auto conflict = service.set_parameter(100, "gain", int64_t(6), 0, 2);
  CHECK_EQ(conflict.status.code, ErrorCode::Conflict);
  CHECK_EQ(service.acquire_control(200).code, ErrorCode::Conflict);
  CHECK(service.release_control(100).ok());
  CHECK(service.acquire_control(200).ok());
  CHECK(service.start_task(200, "task-1", 10).ok());
  CHECK_EQ(service.start_task(200, "task-1", 10).code, ErrorCode::Conflict);
  CHECK(service.stop_task(200, "task-1", 11).ok());

  const auto root = std::filesystem::temp_directory_path() / "rail_download_test";
  std::filesystem::remove_all(root);
  std::filesystem::create_directories(root / "task-1");
  std::ofstream(root / "task-1" / "data.rrec", std::ios::binary) << "abcdef";
  std::ofstream(root / "task-1" / "complete") << "ok\n";
  DownloadService downloads(root);
  CHECK_EQ(downloads.list_tasks().size(), static_cast<size_t>(1));
  std::string received;
  auto report = downloads.download("task-1", "data.rrec", {1, 3},
                                   [&](ByteView chunk) {
                                     received.append(reinterpret_cast<const char*>(chunk.data), chunk.size);
                                     return Status::success();
                                   });
  CHECK(report.status.ok());
  CHECK_EQ(received, std::string("bcd"));
  CHECK_EQ(report.bytes, 3u);
  CHECK_EQ(downloads.download("../bad", "data.rrec", {}, [](ByteView) { return Status::success(); }).status.code,
           ErrorCode::InvalidArgument);
}

int main() { return test_main("control_api", run); }
