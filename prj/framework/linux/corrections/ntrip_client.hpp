#pragma once

#include "linux/corrections/forwarder.hpp"

#include <deque>

namespace rail {

struct NtripConfig {
  std::string host;
  uint16_t port{2101};
  std::string mountpoint;
  std::string username;
  std::string password;
};

struct NtripHealth {
  bool connected{};
  uint64_t received_bytes{};
  uint64_t last_received_monotonic_ns{};
  ErrorCode last_error{ErrorCode::Ok};
};

class INtripClient {
 public:
  virtual ~INtripClient() = default;
  virtual Status connect(const NtripConfig&) = 0;
  virtual SourceResult next(CorrectionChunk&) = 0;
  virtual Status reconnect() = 0;
  virtual NtripHealth health() const = 0;
};

class NtripClient final : public INtripClient, public ICorrectionSource {
 public:
  Status connect(const NtripConfig&) override;
  SourceResult next(CorrectionChunk&) override;
  Status reconnect() override;
  NtripHealth health() const override { return health_; }

  void inject(CorrectionChunk chunk) { chunks_.push_back(std::move(chunk)); }

 private:
  NtripConfig config_;
  std::deque<CorrectionChunk> chunks_;
  NtripHealth health_{};
};

}  // namespace rail
