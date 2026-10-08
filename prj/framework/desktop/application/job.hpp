#pragma once

#include "desktop/application/cancellation.hpp"
#include "desktop/processing/interfaces.hpp"

#include <functional>
#include <map>

namespace rail {

using JobId = uint64_t;
enum class JobState : uint8_t { Queued, Running, Completed, Failed, Cancelled };

struct JobStatus {
  JobState state{JobState::Failed};
  ProcessingResult result;
};

using JobProcessor = std::function<ProcessingResult(const ProcessingRequest&, const CancellationToken&)>;

class ProcessingJob {
 public:
  explicit ProcessingJob(JobProcessor processor = {});

  JobId submit(const ProcessingRequest&);
  JobStatus status(JobId) const;
  Status cancel(JobId);
  Status run_next();
  Status run_all();

 private:
  struct Entry {
    ProcessingRequest request;
    JobStatus status;
    bool cancel_requested{};
  };

  JobProcessor processor_;
  std::map<JobId, Entry> jobs_;
  JobId next_id_{1};
};

}  // namespace rail
