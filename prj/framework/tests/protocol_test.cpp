#include "test_support.hpp"
#include "shared/protocol/frame.hpp"
#include <array>
#include <cstring>
using namespace rail;
static void run() {
  CHECK_EQ(crc32(reinterpret_cast<const uint8_t*>("123456789"), 9), 0xCBF43926u);
  std::array<uint8_t, 128> encoded{};
  const uint8_t p[] = {0, 1, 2, 0xff};
  size_t written = 0;
  CHECK_EQ(encode_frame({7, 42, {p, sizeof(p)}}, {encoded.data(), encoded.size()}, written), ErrorCode::Ok);
  FrameDecoder d; int count = 0; d.feed({encoded.data(), 3}, &count, [](void* c, FrameView f){ CHECK_EQ(f.type, 7); CHECK_EQ(f.sequence, 42u); ++*static_cast<int*>(c); });
  d.feed({encoded.data()+3, written-3}, &count, [](void* c, FrameView f){ CHECK_EQ(f.type, 7); CHECK_EQ(f.sequence, 42u); ++*static_cast<int*>(c); }); CHECK_EQ(count, 1);
  encoded[written-1] ^= 0x01; d.feed({encoded.data(), written}, &count, [](void*, FrameView){}); CHECK(d.crc_errors() > 0);
}
int main(){return test_main("protocol", run);}
