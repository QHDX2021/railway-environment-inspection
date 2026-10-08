#include "linux/corrections/ntrip_client.hpp"

namespace rail {

Status NtripClient::connect(const NtripConfig& config) {
  if (config.host.empty() || config.mountpoint.empty() || config.port == 0) {
    health_.last_error = ErrorCode::InvalidArgument;
    return Status::failure(ErrorCode::InvalidArgument, "incomplete NTRIP configuration");
  }
  config_ = config;
  health_.connected = true;
  health_.last_error = ErrorCode::Ok;
  return Status::success();
}

SourceResult NtripClient::next(CorrectionChunk& chunk) {
  if (!health_.connected) {
    return {Status::failure(ErrorCode::InvalidState, "NTRIP disconnected"), false};
  }
  if (chunks_.empty()) return {Status::failure(ErrorCode::Timeout, "NTRIP no data"), false};
  chunk = std::move(chunks_.front());
  chunks_.pop_front();
  health_.received_bytes += chunk.data.size();
  health_.last_received_monotonic_ns = chunk.received_monotonic_ns;
  return {Status::success(), false};
}

Status NtripClient::reconnect() {
  if (config_.host.empty()) return Status::failure(ErrorCode::InvalidState, "NTRIP not configured");
  health_.connected = true;
  health_.last_error = ErrorCode::Ok;
  return Status::success();
}

}  // namespace rail
