#pragma once

#include "shared/types/types.hpp"

#include <filesystem>

namespace rail {

enum class DatasetSplit : uint8_t { Train, Validation, Test };

struct DatasetManifest {
  std::string version;
  std::string scene;
  std::string route;
  std::string acquisition_time;
  std::string weather;
  std::string label_version;
  DatasetSplit split{DatasetSplit::Train};
  bool anonymized{};
  std::vector<std::string> task_ids;
};

Status save_manifest(const std::filesystem::path&, const DatasetManifest&);
Status load_manifest(const std::filesystem::path&, DatasetManifest&);

}  // namespace rail
