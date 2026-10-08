#include "desktop/processing/ai_analyzer.hpp"

namespace rail {

AiResult run_ai(const AiRequest& request,
                const CancellationToken& token,
                const ProcessingProgressCallback& progress) {
  AiResult result;
  result.input_task_id = request.input_task_id;
  result.model_version = request.model_version;
  if (request.input_task_id.empty() || request.input.empty() || request.output.empty() || request.model_version.empty()) {
    result.status = Status::failure(ErrorCode::InvalidArgument, "AI input incomplete");
    return result;
  }
  if (token.cancelled()) {
    result.status = Status::failure(ErrorCode::Cancelled, "AI cancelled");
    return result;
  }
  if (!request.model_available) {
    result.status = Status::failure(ErrorCode::NotSupported, "AI model not installed");
    return result;
  }
  if (progress) progress(1.0);
  result.status = Status::success();
  return result;
}

}  // namespace rail
