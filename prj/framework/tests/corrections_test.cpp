#include "test_support.hpp"
#include "linux/corrections/forwarder.hpp"
#include "linux/corrections/ntrip_client.hpp"
#include "adapters/simulation/gnss_session.hpp"
using namespace rail;
static void run() {
  SimulationCorrectionSource source({1,2,3});
  SimulationGnssSession sink;
  CorrectionForwarder f(source,sink,1000000000);
  CHECK(f.step(1).ok());
  CHECK_EQ(sink.bytes().size(),3u);

  NtripClient ntrip;
  CHECK(ntrip.connect({"caster.example", 2101, "MOUNT", "user", "secret"}).ok());
  ntrip.inject({{9,8,7}, 100});
  CorrectionForwarder ntrip_forwarder(ntrip, sink, 20);
  CHECK(ntrip_forwarder.step(110).ok());
  CHECK_EQ(ntrip.health().received_bytes, 3u);
  ntrip.inject({{1}, 10});
  CHECK_EQ(ntrip_forwarder.step(100).code, ErrorCode::Timeout);
}
int main(){return test_main("corrections", run);}
