#pragma once

#include "desktop/application/cancellation.hpp"
#include "shared/types/types.hpp"

#include <functional>

namespace rail {
using ProcessingProgressCallback = std::function<void(double)>;
}
