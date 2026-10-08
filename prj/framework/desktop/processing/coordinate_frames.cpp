#include "desktop/processing/coordinate_frames.hpp"

namespace rail {

Vec3 Transform::apply(const Vec3& value) const {
  return {
      rotation[0] * value.x + rotation[1] * value.y + rotation[2] * value.z + translation.x,
      rotation[3] * value.x + rotation[4] * value.y + rotation[5] * value.z + translation.y,
      rotation[6] * value.x + rotation[7] * value.y + rotation[8] * value.z + translation.z,
  };
}

Transform compose(const Transform& first, const Transform& second) {
  Transform result;
  for (size_t row = 0; row < 3; ++row) {
    for (size_t col = 0; col < 3; ++col) {
      result.rotation[row * 3 + col] =
          first.rotation[row * 3] * second.rotation[col] +
          first.rotation[row * 3 + 1] * second.rotation[3 + col] +
          first.rotation[row * 3 + 2] * second.rotation[6 + col];
    }
  }
  result.translation = first.apply(second.translation);
  return result;
}

}  // namespace rail
