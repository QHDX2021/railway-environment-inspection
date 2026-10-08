#include "linux/acquisition/pipeline.hpp"

namespace rail {

RunReport Pipeline::run(ISampleSource& source, IRecordingWriter& writer, const PipelineOptions& options) {
  RunReport report{true, 0, 0, 0, Status::success()};
  bool completed = false;
  for (;;) {
    if (options.cancel && options.cancel->load()) {
      source.stop();
      report.ok = false;
      report.status = Status::failure(ErrorCode::Cancelled, "cancelled");
      break;
    }
    Record record;
    auto next = source.next(record);
    if (next.eof) {
      completed = true;
      break;
    }
    if (!next.status.ok()) {
      report.ok = false;
      report.status = next.status;
      break;
    }
    ++report.produced;
    auto append = writer.append(record);
    if (!append.ok()) {
      report.ok = false;
      report.status = append;
      break;
    }
    ++report.saved;
  }

  auto final_status = completed ? writer.finish() : writer.abort(report.status.code);
  if (!final_status.ok()) {
    report.ok = false;
    report.status = final_status;
  }
  return report;
}

}  // namespace rail
