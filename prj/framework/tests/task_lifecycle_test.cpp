#include "test_support.hpp"
#include "desktop/playback/playback.hpp"
#include "linux/storage/task_writer.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

using namespace rail;

static Record make_record(uint16_t stream, uint32_t boot, uint64_t sequence) {
  Record record;
  record.type = 1;
  record.stream = stream;
  record.sequence = sequence;
  record.time = {sequence, 1000, boot, TimeQuality::Locked, ClockDomain::DeviceTicks};
  record.payload = {0, static_cast<uint8_t>(sequence)};
  return record;
}

static void run() {
  const auto root = std::filesystem::temp_directory_path() / "rail_task_lifecycle_test";
  std::filesystem::remove_all(root);
  const auto complete_dir = root / "complete";
  TaskMetadata metadata;
  metadata.id = "任务-01";
  metadata.device_set_version = "devices-v1";
  metadata.config_snapshot = "config-v3";
  metadata.calibration_ref = "calib-v2";
  metadata.simulated = true;

  {
    TaskWriter writer;
    CHECK(writer.create(complete_dir, metadata).ok());
    CHECK(writer.append(make_record(1, 11, 1)).ok());
    CHECK(writer.append(make_record(2, 11, 2)).ok());
    CHECK(writer.append(make_record(2, 12, 3)).ok());
    CHECK(writer.finish().ok());
  }

  TaskMetadata restored;
  CHECK(read_task_metadata(complete_dir, restored).ok());
  CHECK_EQ(restored.id, std::string("任务-01"));
  CHECK_EQ(restored.device_set_version, std::string("devices-v1"));
  CHECK_EQ(restored.state, TaskState::Completed);
  CHECK(std::filesystem::exists(complete_dir / "complete"));

  Playback playback;
  PlaybackFilter filter;
  filter.stream = 2;
  filter.boot_id = 11;
  CHECK(playback.open(complete_dir / "data.rrec", ReadMode::Strict, filter).ok());
  Record output;
  CHECK(playback.next(output).ok());
  CHECK_EQ(output.sequence, 2u);
  auto end = playback.next(output);
  CHECK(end.ok() && end.eof);

  CHECK(playback.open(complete_dir / "data.rrec", ReadMode::Strict).ok());
  playback.cancel();
  CHECK_EQ(playback.next(output).status.code, ErrorCode::Cancelled);

  const auto incomplete_dir = root / "incomplete";
  {
    TaskWriter writer;
    CHECK(writer.create(incomplete_dir, metadata).ok());
    CHECK(writer.append(make_record(1, 11, 4)).ok());
  }
  RecordReader reader;
  auto strict = reader.read(incomplete_dir / "data.rrec", ReadMode::Strict, [](const Record&) {});
  CHECK(!strict.ok && !strict.complete);
  auto recover = reader.read(incomplete_dir / "data.rrec", ReadMode::Recover, [](const Record&) {});
  CHECK(recover.ok && !recover.complete && recover.records == 1);

  std::ifstream complete_data(complete_dir / "data.rrec", std::ios::binary);
  std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(complete_data)), {});
  CHECK(bytes.size() > 12);
  const uint32_t first_length = static_cast<uint32_t>(bytes[4]) |
                               (static_cast<uint32_t>(bytes[5]) << 8) |
                               (static_cast<uint32_t>(bytes[6]) << 16) |
                               (static_cast<uint32_t>(bytes[7]) << 24);
  const size_t second_offset = 12u + first_length;
  CHECK(second_offset + 12 < bytes.size());
  bytes[second_offset + 9] ^= 0x01;
  const auto corrupt_path = complete_dir / "corrupt.rrec";
  std::ofstream corrupt(corrupt_path, std::ios::binary | std::ios::trunc);
  corrupt.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  corrupt.close();
  auto strict_corrupt = reader.read(corrupt_path, ReadMode::Strict, [](const Record&) {});
  CHECK(!strict_corrupt.ok && strict_corrupt.records == 1);
  auto recover_corrupt = reader.read(corrupt_path, ReadMode::Recover, [](const Record&) {});
  CHECK(recover_corrupt.ok && recover_corrupt.records == 1 && recover_corrupt.bad_offset == second_offset);
}

int main() { return test_main("task_lifecycle", run); }
