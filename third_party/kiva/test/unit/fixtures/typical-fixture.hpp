/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef TYPICAL_FIXTURE_HPP_
#define TYPICAL_FIXTURE_HPP_

#include "base-fixture.hpp"
#include "foundation-fixture.hpp"

using namespace Kiva;

class TypicalFixture : public BaseFixture {
protected:
  void SetUp() { fnd = typical_fnd(); }

  double calcQ() {
    init();
    bcs.localWindSpeed = 0;
    bcs.outdoorTemp = 283.15;
    bcs.slabConvectiveTemp = bcs.wallConvectiveTemp = bcs.slabRadiantTemp = bcs.wallRadiantTemp =
        310.15;
    ground->calculate(bcs, 3600.0);
    ground->calculateSurfaceAverages();
    return ground->getSurfaceAverageValue({Surface::ST_SLAB_CORE, GroundOutput::OT_RATE});
  }
};

#endif /* TYPICAL_FIXTURE_HPP_ */
