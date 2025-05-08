#ifndef SENSESP_TRANSFORMS_MY_LINEAR_H_
#define SENSESP_TRANSFORMS_MY_LINEAR_H_

#include "sensesp/transforms/linear.h"

namespace sensesp {

// Two points, where point.first is 'x' and point.second is 'y', or "f(x)"
// are transformed to slope intercept form (f(x) = m*x + b) to get a `Linear` instance.
// e.g. A sensor with resistance range of 0 to 190 Ohms might
// correspond to an angles -35 to +35 degrees, then that means
// a the first point is x=0, f(x)=-35 and end point at x=190, f(x)=+35
std::shared_ptr<Linear> linearTransformOf(
  std::pair<float, float> point1,
  std::pair<float, float> point2,
  const String& config_path = ""
) {
    const float m = (point2.second - point1.second) / (point2.first - point1.first);
    const float b = point1.second - (m * point1.first);
    return std::make_shared<Linear>(m, b, config_path);
  }

}  // namespace sensesp

#endif // SENSESP_TRANSFORMS_MY_LINEAR_H_