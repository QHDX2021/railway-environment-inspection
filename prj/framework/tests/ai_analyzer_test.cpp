#include "test_support.hpp"
#include "desktop/processing/ai_analyzer.hpp"
#include "desktop/processing/dataset_manifest.hpp"

#include <filesystem>

using namespace rail;

static void run() {
  DatasetManifest manifest;
  manifest.version = "manifest-v1";
  manifest.scene = "rail-yard";
  manifest.route = "route-1";
  manifest.acquisition_time = "2026-10-08T12:00:00Z";
  manifest.weather = "clear";
  manifest.label_version = "labels-v2";
  manifest.split = DatasetSplit::Validation;
  manifest.anonymized = true;
  manifest.task_ids = {"task-1"};
  const auto path = std::filesystem::temp_directory_path() / "rail_manifest.txt";
  CHECK(save_manifest(path, manifest).ok());
  DatasetManifest loaded;
  CHECK(load_manifest(path, loaded).ok());
  CHECK_EQ(loaded.route, std::string("route-1"));
  CHECK_EQ(loaded.split, DatasetSplit::Validation);

  AiRequest request;
  request.input_task_id = "task-1";
  request.input = "task-1/data.rrec";
  request.output = "task-1/ai";
  request.model_version = "model-v1";
  request.model_available = false;
  auto unsupported = run_ai(request, {}, {});
  CHECK_EQ(unsupported.status.code, ErrorCode::NotSupported);
}

int main() { return test_main("ai_analyzer", run); }
