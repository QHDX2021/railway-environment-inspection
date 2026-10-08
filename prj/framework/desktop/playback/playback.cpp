#include "desktop/playback/playback.hpp"

namespace rail {

Status Playback::open(const std::filesystem::path& path,
                      ReadMode mode,
                      const PlaybackFilter& filter) {
  records_.clear();
  pos_ = 0;
  cancelled_ = false;

  RecordReader reader;
  auto report = reader.read(path, mode, [&](const Record& record) {
    if (filter.stream && record.stream != *filter.stream) return;
    if (filter.clock_domain && record.time.domain != *filter.clock_domain) return;
    if (filter.boot_id && record.time.boot_id != *filter.boot_id) return;
    records_.push_back(record);
  });
  return report.ok ? Status::success()
                   : Status::failure(ErrorCode::CorruptData, "playback input invalid");
}

SourceResult Playback::next(Record& record) {
  if (cancelled_) {
    return {Status::failure(ErrorCode::Cancelled, "playback cancelled"), false};
  }
  if (pos_ >= records_.size()) return {Status::success(), true};
  record = records_[pos_++];
  return {Status::success(), false};
}

}  // namespace rail
