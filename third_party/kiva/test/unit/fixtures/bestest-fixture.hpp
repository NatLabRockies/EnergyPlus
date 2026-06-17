/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef BESTEST_FIXTURE_HPP_
#define BESTEST_FIXTURE_HPP_

#include "base-fixture.hpp"

using namespace Kiva;

class BESTESTFixture : public BaseFixture {
protected:
  virtual void SetUp() { specifySystem(); };
  void specifySystem() {
    fnd.reductionStrategy = Foundation::RS_AP;
    Material soil(1.9, 1490.0, 1800.0);
    double length(12.0);
    double width(12.0);

    fnd.deepGroundBoundary = Foundation::DGB_FIXED_TEMPERATURE;

    fnd.soil = soil;
    fnd.grade.absorptivity = 0.0;
    fnd.grade.emissivity = 0.0;

    fnd.hasSlab = false;
    fnd.slab.interior.emissivity = 0.0;

    fnd.polygon.outer().push_back(Point(-length / 2.0, -width / 2.0));
    fnd.polygon.outer().push_back(Point(-length / 2.0, width / 2.0));
    fnd.polygon.outer().push_back(Point(length / 2.0, width / 2.0));
    fnd.polygon.outer().push_back(Point(length / 2.0, -width / 2.0));

    Layer tempLayer;
    tempLayer.thickness = 0.24;
    tempLayer.material = soil;

    fnd.wall.layers.push_back(tempLayer);

    fnd.wall.heightAboveGrade = 0.0;
    fnd.wall.depthBelowSlab = 0.0;
    fnd.wall.interior.emissivity = 0.0;
    fnd.wall.exterior.emissivity = 0.0;
    fnd.wall.interior.absorptivity = 0.0;
    fnd.wall.exterior.absorptivity = 0.0;

    fnd.numericalScheme = Foundation::NS_STEADY_STATE;
    fnd.mesh.maxNearGrowthCoeff = 1.0;

    bcs.localWindSpeed = 0;
    bcs.outdoorTemp = 283.15;
    bcs.slabConvectiveTemp = bcs.wallConvectiveTemp = bcs.slabRadiantTemp = bcs.wallRadiantTemp =
        303.15;
    bcs.deepGroundTemperature = 283.15;

    bcs.slabConvectionAlgorithm = KIVA_CONST_CONV(99999.);
    bcs.intWallConvectionAlgorithm = KIVA_CONST_CONV(99999.);
    bcs.extWallConvectionAlgorithm = KIVA_CONST_CONV(99999.);
    bcs.gradeConvectionAlgorithm = KIVA_CONST_CONV(99999.);

    outputMap = {Surface::ST_SLAB_CORE};
  };
};

class GC10aFixture : public BESTESTFixture {
protected:
  virtual void SetUp() {
    specifySystem();
    specifyGC10a();
  };

  void specifyGC10a() {
    fnd.wallTopBoundary = Foundation::WTB_LINEAR_DT;
    fnd.wallTopInteriorTemperature = 303.15;
    fnd.wallTopExteriorTemperature = 283.15;
    fnd.numberOfDimensions = 2;
  }
};

#endif /* BESTEST_FIXTURE_HPP_ */
