#include "test_support.hpp"
#include "linux/acquisition/device_manager.hpp"
#include "linux/devices/byte_transport.hpp"
#include "linux/devices/camera_device.hpp"
#include "linux/devices/gnss_device.hpp"
#include "linux/devices/lidar_device.hpp"
#include "linux/devices/pcb_device.hpp"

#include <deque>

using namespace rail;

class FakeTransport final : public IByteTransport {
 public:
  bool opened{};
  std::deque<uint8_t> bytes;

  Status open(const TransportConfig&) override { opened = true; return Status::success(); }
  Status read(MutableByteView output) override {
    if (!opened) return Status::failure(ErrorCode::InvalidState, "transport closed");
    const size_t count = std::min(output.size, bytes.size());
    for (size_t i = 0; i < count; ++i) {
      output.data[i] = bytes.front();
      bytes.pop_front();
    }
    return Status::success();
  }
  Status write(ByteView input) override {
    if (!opened) return Status::failure(ErrorCode::InvalidState, "transport closed");
    for (size_t i = 0; i < input.size; ++i) bytes.push_back(input.data[i]);
    return Status::success();
  }
  void close() override { opened = false; }
};

static void run() {
  FakeTransport pcb_transport;
  PcbDevice pcb(pcb_transport);
  CHECK_EQ(pcb.capabilities().device_id, std::string("pcb"));
  CHECK(pcb.configure({"pcb", 1000, "imu=adis16477"}).ok());
  CHECK(pcb.start().ok());
  CHECK(pcb.health().connected);
  CHECK_EQ(pcb.start().code, ErrorCode::Conflict);
  CHECK(pcb.stop().ok());
  CHECK(!pcb.health().connected);

  FakeTransport gnss_transport;
  FakeTransport lidar_transport;
  FakeTransport camera_transport;
  GnssDevice gnss(gnss_transport);
  LidarDevice lidar(lidar_transport);
  CameraDevice camera(camera_transport);
  DeviceManager manager;
  CHECK(manager.add(pcb).ok());
  CHECK(manager.add(gnss).ok());
  CHECK(manager.add(lidar).ok());
  CHECK(manager.add(camera).ok());
  CHECK(manager.start_task({"task-1", {"pcb", "gnss"}}).ok());
  CHECK(gnss.health().connected);
  CHECK(!lidar.health().connected);
  CHECK_EQ(manager.health().size(), static_cast<size_t>(4));
  CHECK(manager.stop_task().ok());
  CHECK(!gnss.health().connected);
}

int main() { return test_main("device_adapter", run); }
