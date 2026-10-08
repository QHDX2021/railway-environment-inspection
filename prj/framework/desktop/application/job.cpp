#include "desktop/application/job.hpp"

namespace rail {

ProcessingJob::ProcessingJob(JobProcessor processor) : processor_(std::move(processor)) {
  if (!processor_) {
    processor_ = [](const ProcessingRequest& request, const CancellationToken&) {
      return run_not_supported(request);
    };
  }
}

JobId ProcessingJob::submit(const ProcessingRequest& request) {
  const JobId id = next_id_++;
  Entry entry;
  entry.request = request;
  entry.status.state = JobState::Queued;
  if (request.input.empty() || request.output.empty() || request.task_id.empty() ||
      request.config_snapshot.empty() || request.calibration_ref.empty()) {
    entry.status.state = JobState::Failed;
    entry.status.result.status = Status::failure(ErrorCode::InvalidArgument, "processing input incomplete");
  }
  jobs_.emplace(id, std::move(entry));
  return id;
}

JobStatus ProcessingJob::status(JobId id) const {
  const auto it = jobs_.find(id);
  if (it == jobs_.end()) {
    return {JobState::Failed,
            {Status::failure(ErrorCode::InvalidArgument, "unknown job"), {}}};
  }
  return it->second.status;
}

Status ProcessingJob::cancel(JobId id) {
  const auto it = jobs_.find(id);
  if (it == jobs_.end()) return Status::failure(ErrorCode::InvalidArgument, "unknown job");
  auto& entry = it->second;
  if (entry.status.state == JobState::Queued) {
    entry.cancel_requested = true;
    entry.status.state = JobState::Cancelled;
    entry.status.result = {Status::failure(ErrorCode::Cancelled, "job cancelled"), {}};
    return Status::success();
  }
  if (entry.status.state == JobState::Running) {
    entry.cancel_requested = true;
    return Status::success();
  }
  return Status::failure(ErrorCode::InvalidState, "job is not cancellable");
}

Status ProcessingJob::run_next() {
  for (auto& pair : jobs_) {
    auto& entry = pair.second;
    if (entry.status.state != JobState::Queued) continue;
    if (entry.cancel_requested) {
      entry.status.state = JobState::Cancelled;
      entry.status.result = {Status::failure(ErrorCode::Cancelled, "job cancelled"), {}};
      return Status::success();
    }
    entry.status.state = JobState::Running;
    const CancellationToken token(&entry.cancel_requested);
    entry.status.result = processor_(entry.request, token);
    if (entry.cancel_requested || entry.status.result.status.code == ErrorCode::Cancelled) {
      entry.status.state = JobState::Cancelled;
      if (entry.status.result.status.ok()) {
        entry.status.result.status = Status::failure(ErrorCode::Cancelled, "job cancelled");
      }
    } else {
      entry.status.state = entry.status.result.status.ok() ? JobState::Completed : JobState::Failed;
    }
    return Status::success();
  }
  return Status::failure(ErrorCode::InvalidState, "no queued job");
}

Status ProcessingJob::run_all() {
  for (;;) {
    auto status = run_next();
    if (!status.ok()) {
      return status.code == ErrorCode::InvalidState ? Status::success() : status;
    }
  }
}

}  // namespace rail
