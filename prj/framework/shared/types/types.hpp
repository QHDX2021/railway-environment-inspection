#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace rail {
enum class ErrorCode { Ok, InvalidArgument, InvalidState, Conflict, IoError, CorruptData, UnsupportedVersion, NotSupported, Overflow, Cancelled, Timeout };
struct Status { ErrorCode code{ErrorCode::Ok}; std::string message; bool ok() const { return code == ErrorCode::Ok; } static Status success(){return {}; } static Status failure(ErrorCode c,std::string m){return {c,std::move(m)};} };
enum class TimeQuality : uint8_t { Unsynchronized, Locked, Holdover, Invalid };
enum class ClockDomain : uint8_t { DeviceTicks, GnssTime, Utc, HostMonotonic };
struct TimeTag {
  ClockDomain domain{ClockDomain::DeviceTicks};
  uint64_t ticks{};
  uint32_t ticks_per_second{};
  uint32_t boot_id{};
  TimeQuality quality{TimeQuality::Unsynchronized};

  TimeTag() = default;
  TimeTag(uint64_t device_ticks,
          uint32_t rate,
          uint32_t boot,
          TimeQuality sync_quality,
          ClockDomain clock_domain = ClockDomain::DeviceTicks)
      : domain(clock_domain), ticks(device_ticks), ticks_per_second(rate), boot_id(boot), quality(sync_quality) {}
  TimeTag(ClockDomain clock_domain,
          uint64_t time_ticks,
          uint32_t rate,
          uint32_t boot,
          TimeQuality sync_quality)
      : domain(clock_domain), ticks(time_ticks), ticks_per_second(rate), boot_id(boot), quality(sync_quality) {}
};
struct ByteView { const uint8_t* data{}; size_t size{}; };
struct MutableByteView { uint8_t* data{}; size_t size{}; };
using Bytes = std::vector<uint8_t>;
}
