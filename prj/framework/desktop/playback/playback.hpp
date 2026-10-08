#pragma once
#include "shared/recording/reader.hpp"
#include "linux/acquisition/pipeline.hpp"
#include <optional>

namespace rail {

struct PlaybackFilter {
  std::optional<uint16_t> stream;
  std::optional<ClockDomain> clock_domain;
  std::optional<uint32_t> boot_id;
};

class Playback {
  std::vector<Record> records_;
  size_t pos_{};
  bool cancelled_{};

 public:
  Status open(const std::filesystem::path&, ReadMode, const PlaybackFilter& = {});
  SourceResult next(Record&);
  void cancel() { cancelled_ = true; }
};

}  // namespace rail
