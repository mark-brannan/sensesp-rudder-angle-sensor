#ifndef RUDDER_ANGLE_SENSOR_LINEAR_H_
#define RUDDER_ANGLE_SENSOR_LINEAR_H_

#include <memory>
#include <utility>

#include "sensesp/transforms/linear.h"

namespace sensesp {

/// An (x, f(x)) point on a line.
using XYPair = std::pair<float, float>;

/**
 * @brief Build a `Linear` transform from two points on the line.
 *
 * `Linear` is defined in slope-intercept form (f(x) = m*x + b). This helper
 * derives m and b from two points, which is usually how a sensor is
 * specified: e.g. a resistance of 0 Ohms corresponds to -35 degrees and
 * 190 Ohms to +35 degrees, so point1 = (0, -35) and point2 = (190, 35).
 */
inline std::shared_ptr<Linear> linear_transform_of(
    const XYPair& point1, const XYPair& point2,
    const String& config_path = "") {
  const float m = (point2.second - point1.second) / (point2.first - point1.first);
  const float b = point1.second - (m * point1.first);
  return std::make_shared<Linear>(m, b, config_path);
}

}  // namespace sensesp

#endif  // RUDDER_ANGLE_SENSOR_LINEAR_H_
