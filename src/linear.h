#ifndef SENSESP_TRANSFORMS_MY_LINEAR_H_
#define SENSESP_TRANSFORMS_MY_LINEAR_H_

#include "sensesp/transforms/linear.h"

namespace sensesp {

// Two ranges, where range.first is 'x' and range.second is 'y';
// e.g. A voltage range of 0 to 3.3 converted to an angle range of -35 to +35,
std::shared_ptr<Linear> linearTransformOf(std::pair<float, float> range1, std::pair<float, float> range2) {
    const float m = (range2.second - range1.second) / (range2.first - range1.first);
    const float b = range1.second - (m * range1.first);
    return std::make_shared<Linear>(m, b);
  }

}  // namespace sensesp

#endif // SENSESP_TRANSFORMS_MY_LINEAR_H_