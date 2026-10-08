#pragma once

#include <array>

namespace rail {

struct Vec3 {
  double x{};
  double y{};
  double z{};
};

struct Transform {
  std::array<double, 9> rotation{1, 0, 0, 0, 1, 0, 0, 0, 1};
  Vec3 translation{};

  Vec3 apply(const Vec3&) const;
};

Transform compose(const Transform& first, const Transform& second);

}  // namespace rail
