#include "test_support.hpp"
#include "firmware/core/host_protocol.hpp"
#include "firmware/core/imu_capture.hpp"
#include "firmware/hal/time_hal.hpp"

#include <deque>

using namespace rail;

class FakeImu final : public IImuHal {
 public:
  std::deque<ImuSample> samples;
  bool configured{};

  Status configure(const ImuConfig&) override {
    configured = true;
    return Status::success();
  }
  Status read_burst(ImuSample& sample) override {
    if (samples.empty()) return Status::failure(ErrorCode::Timeout, "no sample");
    sample = samples.front();
    samples.pop_front();
    return Status::success();
  }
  bool data_ready() const override { return !samples.empty(); }
};

class FakeTime final : public ITimeHal {
 public:
  uint64_t pps{};
  uint64_t mapped_ticks{};
  Status mapping_status{Status::success()};

  uint64_t capture_pps() override { return pps; }
  Status map_second(uint64_t pps_tick, uint64_t device_tick) override {
    pps = pps_tick;
    mapped_ticks = device_tick;
    return mapping_status;
  }
};

class FakeTransport final : public IHostTransport {
 public:
  std::deque<Message> incoming;
  std::deque<Message> outgoing;

  Status send(const Message& message) override {
    outgoing.push_back(message);
    return Status::success();
  }
  Status receive(Message& message) override {
    if (incoming.empty()) return Status::failure(ErrorCode::Timeout, "no message");
    message = incoming.front();
    incoming.pop_front();
    return Status::success();
  }
};

static ImuSample sample(uint64_t ticks) {
  ImuSample value;
  value.device_ticks = ticks;
  value.gyro[0] = static_cast<int32_t>(ticks);
  value.accel[0] = -static_cast<int32_t>(ticks);
  value.quality = TimeQuality::Locked;
  return value;
}

static void run() {
  FakeImu imu;
  CHECK(imu.configure({1000, 4}).ok());
  imu.samples.push_back(sample(10));
  imu.samples.push_back(sample(11));
  imu.samples.push_back(sample(12));
  imu.samples.push_back(sample(13));
  imu.samples.push_back(sample(14));
  ImuCapture capture(imu, 4);
  CHECK(capture.on_data_ready().ok());
  CHECK(capture.on_data_ready().ok());
  CHECK(capture.on_data_ready().ok());
  CHECK(capture.on_data_ready().ok());
  CHECK_EQ(capture.on_data_ready().code, ErrorCode::Overflow);
  CHECK_EQ(capture.stats().overflow_count, 1u);

  ImuSample output;
  for (uint64_t expected = 0; expected < 4; ++expected) {
    CHECK(capture.pop(output));
    CHECK_EQ(output.sequence, expected);
  }
  CHECK(!capture.pop(output));

  FakeTime time;
  CHECK(time.map_second(100, 999).ok());
  CHECK_EQ(time.capture_pps(), 100u);
  CHECK_EQ(time.mapped_ticks, 999u);

  FakeTransport transport;
  transport.incoming.push_back({MessageType::TaskStart, 77, {}});
  HostProtocol protocol(transport);
  CHECK(protocol.poll().ok());
  CHECK_EQ(transport.outgoing.size(), static_cast<size_t>(1));
  CHECK_EQ(transport.outgoing.front().type, MessageType::Status);
  CHECK_EQ(transport.outgoing.front().request_id, 77u);

  PowerStateMachine power;
  CHECK_EQ(power.dispatch(PowerEvent::ExternalLost), PowerAction::StopAcquisition);
  CHECK_EQ(power.dispatch(PowerEvent::DataFlushed), PowerAction::RequestShutdown);
  CHECK_EQ(power.dispatch(PowerEvent::OsShutdownComplete), PowerAction::CutPower);
  CHECK_EQ(power.dispatch(PowerEvent::Timeout), PowerAction::Fault);
}

int main() { return test_main("firmware_core", run); }
