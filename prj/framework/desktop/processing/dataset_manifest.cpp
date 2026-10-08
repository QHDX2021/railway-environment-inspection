#include "desktop/processing/dataset_manifest.hpp"

#include <fstream>

namespace rail {
namespace {
const char* split_name(DatasetSplit split) {
  switch (split) {
    case DatasetSplit::Train: return "train";
    case DatasetSplit::Validation: return "validation";
    case DatasetSplit::Test: return "test";
  }
  return "train";
}
bool parse_split(const std::string& text, DatasetSplit& split) {
  if (text == "train") split = DatasetSplit::Train;
  else if (text == "validation") split = DatasetSplit::Validation;
  else if (text == "test") split = DatasetSplit::Test;
  else return false;
  return true;
}
bool valid(const DatasetManifest& manifest) {
  return !manifest.version.empty() && !manifest.scene.empty() && !manifest.route.empty() &&
         !manifest.acquisition_time.empty() && !manifest.label_version.empty() && !manifest.task_ids.empty();
}
}  // namespace

Status save_manifest(const std::filesystem::path& path, const DatasetManifest& manifest) {
  if (!valid(manifest)) return Status::failure(ErrorCode::InvalidArgument, "manifest incomplete");
  std::ofstream out(path, std::ios::trunc);
  if (!out) return Status::failure(ErrorCode::IoError, "open manifest");
  out << "version=" << manifest.version << '\n'
      << "scene=" << manifest.scene << '\n'
      << "route=" << manifest.route << '\n'
      << "acquisition_time=" << manifest.acquisition_time << '\n'
      << "weather=" << manifest.weather << '\n'
      << "label_version=" << manifest.label_version << '\n'
      << "split=" << split_name(manifest.split) << '\n'
      << "anonymized=" << (manifest.anonymized ? 1 : 0) << '\n'
      << "task_ids=";
  for (size_t i = 0; i < manifest.task_ids.size(); ++i) out << (i ? "," : "") << manifest.task_ids[i];
  out << '\n';
  return out.good() ? Status::success() : Status::failure(ErrorCode::IoError, "write manifest");
}

Status load_manifest(const std::filesystem::path& path, DatasetManifest& manifest) {
  std::ifstream in(path);
  if (!in) return Status::failure(ErrorCode::IoError, "open manifest");
  DatasetManifest parsed;
  std::string line;
  while (std::getline(in, line)) {
    const auto split = line.find('=');
    if (split == std::string::npos) return Status::failure(ErrorCode::CorruptData, "bad manifest line");
    const auto key = line.substr(0, split);
    const auto value = line.substr(split + 1);
    if (key == "version") parsed.version = value;
    else if (key == "scene") parsed.scene = value;
    else if (key == "route") parsed.route = value;
    else if (key == "acquisition_time") parsed.acquisition_time = value;
    else if (key == "weather") parsed.weather = value;
    else if (key == "label_version") parsed.label_version = value;
    else if (key == "split" && !parse_split(value, parsed.split)) return Status::failure(ErrorCode::CorruptData, "bad split");
    else if (key == "anonymized") parsed.anonymized = value == "1";
    else if (key == "task_ids") {
      size_t begin = 0;
      while (begin <= value.size()) {
        const auto end = value.find(',', begin);
        const auto item = value.substr(begin, end == std::string::npos ? end : end - begin);
        if (!item.empty()) parsed.task_ids.push_back(item);
        if (end == std::string::npos) break;
        begin = end + 1;
      }
    }
  }
  if (!valid(parsed)) return Status::failure(ErrorCode::CorruptData, "manifest incomplete");
  manifest = std::move(parsed);
  return Status::success();
}

}  // namespace rail
