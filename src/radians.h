#ifndef RUDDER_ANGLE_SENSOR_RADIANS_H_
#define RUDDER_ANGLE_SENSOR_RADIANS_H_

#include <cmath>

#include "sensesp/transforms/lambda_transform.h"

namespace sensesp {

inline float degrees_to_radians(float degrees) {
  return degrees * (static_cast<float>(M_PI) / 180.0f);
}

inline float radians_to_degrees(float radians) {
  return radians * (180.0f / static_cast<float>(M_PI));
}

/// Transform that converts a value in degrees to radians.
class DegreesToRadians : public LambdaTransform<float, float> {
 public:
  DegreesToRadians() : LambdaTransform<float, float>(degrees_to_radians) {}
};

}  // namespace sensesp

#endif  // RUDDER_ANGLE_SENSOR_RADIANS_H_
