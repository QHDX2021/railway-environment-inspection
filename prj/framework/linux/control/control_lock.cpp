#include "linux/control/control_lock.hpp"

namespace rail {

Status ControlLock::acquire(uint64_t owner) {
  if (owner == 0) return Status::failure(ErrorCode::InvalidArgument, "invalid owner");
  if (locked_ && owner_ != owner) return Status::failure(ErrorCode::Conflict, "control lock held");
  locked_ = true;
  owner_ = owner;
  return Status::success();
}

Status ControlLock::release(uint64_t owner) {
  if (!locked_) return Status::failure(ErrorCode::InvalidState, "control lock is free");
  if (owner_ != owner) return Status::failure(ErrorCode::Conflict, "control lock owner mismatch");
  locked_ = false;
  owner_ = 0;
  return Status::success();
}

}  // namespace rail
