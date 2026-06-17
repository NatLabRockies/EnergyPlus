/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef AGGREGATOR_FIXTURE_HPP_
#define AGGREGATOR_FIXTURE_HPP_

#include "Aggregator.hpp"
#include "Instance.hpp"
#include "base-fixture.hpp"
#include "foundation-fixture.hpp"

using namespace Kiva;

class AggregatorFixture : public BaseFixture {
public:
  void SetUp() {
    fndSlab = typical_fnd();
    fndBsmt = typical_fnd();
    fndBsmt.foundationDepth = 2; // m

    instances.emplace_back(fndSlab);
    instances.emplace_back(fndBsmt);

    bcs.localWindSpeed = 0;
    bcs.outdoorTemp = 283.15;
    bcs.slabConvectiveTemp = bcs.wallConvectiveTemp = bcs.slabRadiantTemp = bcs.wallRadiantTemp =
        310.15;

    for (auto &instance : instances) {
      Foundation::NumericalScheme tempNS = instance.ground->foundation.numericalScheme;
      instance.ground->foundation.numericalScheme = Foundation::NS_STEADY_STATE;
      instance.ground->calculate(bcs);
      instance.ground->foundation.numericalScheme = tempNS;
      instance.ground->calculateSurfaceAverages();
    }
  }

  Foundation fndSlab;
  Foundation fndBsmt;
  std::vector<Instance> instances;
};

#endif /* AGGREGATOR_FIXTURE_HPP_ */
