#include "test_support.hpp"
#include "linux/acquisition/pipeline.hpp"
#include "adapters/simulation/sample_source.hpp"
#include "linux/storage/task_writer.hpp"
#include <filesystem>
#include <vector>
using namespace rail;
class OversizedSource final : public ISampleSource {
  bool sent_{};
 public:
  SourceResult next(Record& record) override {
    if (sent_) return {Status::success(), true};
    sent_ = true;
    record.payload.resize(16 * 1024 * 1024 + 1);
    return {Status::success(), false};
  }
  void stop() override {}
};
static void run() {
  auto dir=std::filesystem::temp_directory_path()/"rail_acquisition_test"; std::filesystem::remove_all(dir); TaskWriter w; CHECK(w.create(dir,{"a1",true}).ok());
  SimulationSource src(10); Pipeline p; auto report=p.run(src,w,{}); CHECK(report.ok); CHECK_EQ(report.produced,40u); CHECK(w.finished());

  auto failed_dir=std::filesystem::temp_directory_path()/"rail_acquisition_failed"; std::filesystem::remove_all(failed_dir);
  TaskWriter failed_writer; CHECK(failed_writer.create(failed_dir,{"failed",true}).ok());
  OversizedSource failed_source; auto failed=p.run(failed_source,failed_writer,{});
  CHECK(!failed.ok); CHECK(!std::filesystem::exists(failed_dir/"complete"));
  TaskMetadata failed_metadata; CHECK(read_task_metadata(failed_dir,failed_metadata).ok());
  CHECK_EQ(failed_metadata.state,TaskState::Failed);
}
int main(){return test_main("acquisition", run);}
