#ifndef SENSESP_TRANSFORMS_RADIANS_H_
#define SENSESP_TRANSFORMS_RADIANS_H_

#include <cmath>
#include "sensesp/transforms/transform.h"

namespace sensesp {

 // Conveneince function to convert degrees to radians
 inline const double convertDegreesToRadians(const double& degrees) {
    return degrees * (M_PI / 180.0);
 }

 inline const double convertRadiansToDegress(const double& radians) {
    return radians * (180.0 / M_PI);
 }

class RadiansTransform : public LambdaTransform<float, float> {
 public:
  RadiansTransform() : LambdaTransform<float, float>(convertDegreesToRadians) {}
};


}  // namespace sensesp

#endif // SENSESP_TRANSFORMS_RADIANS_H_