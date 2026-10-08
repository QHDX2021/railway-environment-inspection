#include "shared/recording/reader.hpp"

#include <fstream>

namespace rail {

ReadReport RecordReader::read(const std::filesystem::path& path,
                              ReadMode mode,
                              const std::function<void(const Record&)>& callback) {
  std::ifstream input(path, std::ios::binary);
  if (!input) return {false, 0, 0, false};

  std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
  const bool complete_marker = std::filesystem::exists(path.parent_path() / "complete");
  size_t offset = 0;
  size_t records = 0;
  while (offset < bytes.size()) {
    Record record;
    size_t used = 0;
    auto status = read_record(bytes.data() + offset, bytes.size() - offset, record, used);
    if (!status.ok()) {
      if (mode == ReadMode::Recover && records > 0) {
        return {true, records, offset, false};
      }
      return {false, records, offset, false};
    }
    if (callback) callback(record);
    offset += used;
    ++records;
  }

  if (!complete_marker && mode == ReadMode::Strict) {
    return {false, records, offset, false};
  }
  return {true, records, 0, complete_marker};
}

}  // namespace rail
