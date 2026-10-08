#include "test_support.hpp"
#include "desktop/application/job.hpp"

#include <filesystem>

using namespace rail;

static void run() {
  ProcessingJob jobs([](const ProcessingRequest& request, const CancellationToken& token) {
    if (token.cancelled()) return ProcessingResult{Status::failure(ErrorCode::Cancelled, "cancelled"), {}};
    return ProcessingResult{Status::success(), {request.output / "result.bin"}};
  });
  ProcessingRequest request;
  request.task_id = "task-1";
  request.input = "input.rrec";
  request.config_snapshot = "config-v1";
  request.calibration_ref = "calib-v1";
  request.output = std::filesystem::temp_directory_path() / "rail_job_output";

  const JobId complete_id = jobs.submit(request);
  CHECK_EQ(jobs.status(complete_id).state, JobState::Queued);
  CHECK(jobs.run_next().ok());
  CHECK_EQ(jobs.status(complete_id).state, JobState::Completed);
  CHECK(jobs.status(complete_id).result.status.ok());

  const JobId cancelled_id = jobs.submit(request);
  CHECK(jobs.cancel(cancelled_id).ok());
  CHECK_EQ(jobs.status(cancelled_id).state, JobState::Cancelled);
  CHECK_EQ(jobs.status(cancelled_id).result.status.code, ErrorCode::Cancelled);

  const JobId missing_id = jobs.submit({});
  CHECK_EQ(jobs.status(missing_id).state, JobState::Failed);
  CHECK_EQ(jobs.status(missing_id).result.status.code, ErrorCode::InvalidArgument);
}

int main() { return test_main("job", run); }
