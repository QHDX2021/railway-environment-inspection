#include "test_support.hpp"
#include "shared/recording/reader.hpp"
#include "linux/storage/task_writer.hpp"
#include <filesystem>
using namespace rail;
static void run() {
  auto dir = std::filesystem::temp_directory_path()/"rail_recording_test"; std::filesystem::remove_all(dir);
  TaskWriter w; CHECK(w.create(dir, {"t1", true}).ok());
  Record r; r.type=1; r.stream=2; r.sequence=3; r.time={99,1000,7,TimeQuality::Locked}; r.payload={0,1,0xff}; r.simulated=true;
  CHECK(w.append(r).ok()); CHECK(w.finish().ok());
  RecordReader rd; int n=0; auto report=rd.read(dir/"data.rrec", ReadMode::Strict, [&](const Record& x){++n; CHECK_EQ(x.sequence,3u); CHECK(x.simulated);}); CHECK(report.ok && n==1);
}
int main(){return test_main("recording", run);}
