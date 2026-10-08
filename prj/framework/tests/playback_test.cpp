#include "test_support.hpp"
#include "desktop/playback/playback.hpp"
#include "linux/storage/task_writer.hpp"
#include <filesystem>
using namespace rail;
static void run(){auto d=std::filesystem::temp_directory_path()/"rail_playback_test";std::filesystem::remove_all(d);TaskWriter w;CHECK(w.create(d,{"p",true}).ok());Record r;r.type=1;r.sequence=1;r.time={1,1,1,TimeQuality::Locked};CHECK(w.append(r).ok());CHECK(w.finish().ok());Playback p;CHECK(p.open(d/"data.rrec",ReadMode::Strict).ok());Record out;CHECK(p.next(out).ok());CHECK_EQ(out.sequence,1u);}
int main(){return test_main("playback",run);}
