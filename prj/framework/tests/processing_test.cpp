#include "test_support.hpp"
#include "desktop/processing/interfaces.hpp"
using namespace rail;
static void run(){ProcessingRequest req; auto result=run_not_supported(req); CHECK(result.status.code==ErrorCode::NotSupported);}
int main(){return test_main("processing",run);}
