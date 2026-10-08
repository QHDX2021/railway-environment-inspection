#include "test_support.hpp"
#include "desktop/processing/change_analyzer.hpp"

using namespace rail;

static void run() {
  ChangeRequest request;
  request.baseline_task_id = "before";
  request.current_task_id = "after";
  request.registration_quality = 0.99;
  request.change_threshold = 0.5;
  request.baseline_points = {{0, 0, 0}, {1, 0, 0}};
  request.current_points = {{0, 0, 0}, {2, 0, 0}};
  auto result = analyze_change(request, {}, {});
  CHECK(result.status.ok());
  CHECK_EQ(result.candidates.size(), static_cast<size_t>(1));
  CHECK_EQ(result.candidates[0].baseline_task_id, std::string("before"));
  CHECK_EQ(result.candidates[0].metric, 1.0);

  request.registration_quality = 0.4;
  auto invalid = analyze_change(request, {}, {});
  CHECK_EQ(invalid.status.code, ErrorCode::InvalidArgument);
}

int main() { return test_main("change_analyzer", run); }
