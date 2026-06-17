/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef GroundOutput_HPP
#define GroundOutput_HPP

#include "Foundation.hpp"

namespace Kiva {

class GroundOutput {
public:
  // Outputs
  enum OutputType {
    OT_FLUX,
    OT_TEMP,     // Temperature that satisfies heat transfer equation (Qavg = havg * (Tin - Tthis))
    OT_AVG_TEMP, // Actual average temperature of the surface (the sign of (Tin - Tthis) may not
                 // agree with sign of Qavg)
    OT_EFF_TEMP, // Effective temperature used as a boundary condition on the other side of
                 // floor/wall constructions (if Kiva is used as a pre-processor)
    OT_RATE,
    OT_CONV,
    OT_RAD
  };

  typedef std::vector<Surface::SurfaceType> OutputMap;

  GroundOutput(OutputMap oM) : outputMap(oM){};

  GroundOutput(){};

  OutputMap outputMap;

  std::map<std::pair<Surface::SurfaceType, OutputType>, double> outputValues;
};

} // namespace Kiva

#endif // GroundOutput_HPP
