#include "test_support.hpp"
#include "shared/configuration/parameters.hpp"
#include "firmware/core/state_machine.hpp"
using namespace rail;
static void run() {
  ParameterService s; s.define({"trigger_hz", int64_t(10), 1, 100, "Hz", false, ApplyMode::IdleOnly, false});
  CHECK(s.set("trigger_hz", int64_t(20), 0).ok()); CHECK_EQ(s.revision(), 1u);
  CHECK(!s.set("trigger_hz", int64_t(200), 1).ok()); CHECK(s.set("trigger_hz", int64_t(30), 0).code == ErrorCode::Conflict);
  AcquisitionStateMachine m; CHECK_EQ(m.dispatch(AcquisitionEvent::Start), ErrorCode::Ok); CHECK_EQ(m.dispatch(AcquisitionEvent::Started), ErrorCode::Ok); CHECK_EQ(m.state(), AcquisitionState::Recording); CHECK_EQ(m.dispatch(AcquisitionEvent::Start), ErrorCode::InvalidState);
}
int main(){return test_main("control", run);}
