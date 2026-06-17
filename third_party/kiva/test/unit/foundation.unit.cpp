/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#include "fixtures/aggregator-fixture.hpp"
#include "fixtures/bestest-fixture.hpp"
#include "fixtures/foundation-fixture.hpp"
#include "fixtures/typical-fixture.hpp"

#include "Errors.hpp"

using namespace Kiva;

std::string dbl_to_string(double dbl) {
  std::ostringstream strs;
  strs << dbl;
  std::string str = strs.str();
  return str;
}

Foundation typical_fnd() {
  Foundation fnd;
  fnd.reductionStrategy = Foundation::RS_AP;

  Material concrete(1.95, 2400.0, 900.0);

  Layer tempLayer;
  tempLayer.thickness = 0.10;
  tempLayer.material = concrete;

  fnd.slab.interior.emissivity = 0.8;
  fnd.slab.layers.push_back(tempLayer);

  tempLayer.thickness = 0.2;
  tempLayer.material = concrete;

  fnd.wall.layers.push_back(tempLayer);

  fnd.wall.heightAboveGrade = 0.1;
  fnd.wall.depthBelowSlab = 0.2;
  fnd.wall.interior.emissivity = 0.8;
  fnd.wall.exterior.emissivity = 0.8;
  fnd.wall.interior.absorptivity = 0.8;
  fnd.wall.exterior.absorptivity = 0.8;

  fnd.foundationDepth = 0.0;
  fnd.numericalScheme = Foundation::NS_ADI;

  fnd.polygon.outer().push_back(Point(-6.0, -6.0));
  fnd.polygon.outer().push_back(Point(-6.0, 6.0));
  fnd.polygon.outer().push_back(Point(6.0, 6.0));
  fnd.polygon.outer().push_back(Point(6.0, -6.0));

  return fnd;
}

TEST_F(GC10aFixture, GC10a) {
  double analyticalQ = 2432.597;
  // double trnsysQ = 2427;
  // double fluentQ = 2425;
  // double matlabQ = 2432;

  EXPECT_NEAR(calcQ(), analyticalQ, analyticalQ * 0.03);

  // kiva: 2480.2
}

TEST_F(BESTESTFixture, GC30a) {
  fnd.deepGroundDepth = 30.0;
  fnd.farFieldWidth = 20.0;

  double trnsysQ = 2642;
  double fluentQ = 2585;
  double matlabQ = 2695;

  double average = (trnsysQ + fluentQ + matlabQ) / 3.0;

  EXPECT_NEAR(calcQ(), average, average * 0.05);

  // average: 2640.7
  // kiva: 2665.0
}

TEST_F(BESTESTFixture, GC30b) {
  fnd.deepGroundDepth = 15.0;
  fnd.farFieldWidth = 15.0;

  bcs.slabConvectionAlgorithm = KIVA_CONST_CONV(100.);
  bcs.intWallConvectionAlgorithm = KIVA_CONST_CONV(100.);
  bcs.extWallConvectionAlgorithm = KIVA_CONST_CONV(100.);
  bcs.gradeConvectionAlgorithm = KIVA_CONST_CONV(100.);

  double trnsysQ = 2533;
  double fluentQ = 2504;
  double matlabQ = 2570;

  double average = (trnsysQ + fluentQ + matlabQ) / 3.0;

  EXPECT_NEAR(calcQ(), average, average * 0.05);

  // average: 2535.7
  // kiva: 2576.8
}

TEST_F(BESTESTFixture, GC30c) {
  fnd.deepGroundDepth = 15.0;
  fnd.farFieldWidth = 8.0;

  bcs.slabConvectionAlgorithm = KIVA_CONST_CONV(7.95);
  bcs.intWallConvectionAlgorithm = KIVA_CONST_CONV(7.95);

  double trnsysQ = 2137;
  double fluentQ = 2123;
  double matlabQ = 2154;

  double average = (trnsysQ + fluentQ + matlabQ) / 3.0;

  EXPECT_NEAR(calcQ(), average, average * 0.05);

  // average: 2138
  // kiva: 2229.3
}

TEST_F(BESTESTFixture, GC60b) {
  fnd.deepGroundDepth = 15.0;
  fnd.farFieldWidth = 15.0;

  bcs.slabConvectionAlgorithm = KIVA_CONST_CONV(7.95);
  bcs.intWallConvectionAlgorithm = KIVA_CONST_CONV(7.95);
  bcs.extWallConvectionAlgorithm = KIVA_CONST_CONV(100.);
  bcs.gradeConvectionAlgorithm = KIVA_CONST_CONV(100.);

  double trnsysQ = 2113;
  double fluentQ = 2104;
  double matlabQ = 2128;

  double average = (trnsysQ + fluentQ + matlabQ) / 3.0;

  EXPECT_NEAR(calcQ(), average, average * 0.05);

  // average: 2115
  // kiva: 2208.7
}

TEST_F(BESTESTFixture, GC65b) {
  fnd.deepGroundDepth = 15.0;
  fnd.farFieldWidth = 15.0;

  bcs.slabConvectionAlgorithm = KIVA_CONST_CONV(7.95);
  bcs.intWallConvectionAlgorithm = KIVA_CONST_CONV(7.95);
  bcs.extWallConvectionAlgorithm = KIVA_CONST_CONV(11.95);
  bcs.gradeConvectionAlgorithm = KIVA_CONST_CONV(11.95);

  double trnsysQ = 1994;
  double fluentQ = 1991;
  double matlabQ = 2004;

  double average = (trnsysQ + fluentQ + matlabQ) / 3.0;

  EXPECT_NEAR(calcQ(), average, average * 0.05);

  // average: 1996.3
  // kiva: 2073.3
}

// Not an actual BESTEST Case, though recommended by TRNSYS report
TEST_F(BESTESTFixture, 1D) {
  fnd.exposedFraction = 0.0;
  fnd.deepGroundDepth = 1.0;
  fnd.useDetailedExposedPerimeter = false;

  double area = 144; // m2
  double expectedQ = fnd.soil.conductivity / fnd.deepGroundDepth * area *
                     (bcs.slabConvectiveTemp - bcs.deepGroundTemperature);
  EXPECT_NEAR(calcQ(), expectedQ, 1.0);
}

/*
TEST_F( TypicalFixture, Slab)
{
  bcs.localWindSpeed = 0;
  bcs.outdoorTemp = 283.15;
  bcs.indoorTemp = 303.15;

  outputMap[Surface::ST_SLAB_CORE] = {
    GroundOutput::OT_RATE
  };

  EXPECT_NEAR(1.0, 1.0, 1.0);
}*/

TEST_F(GC10aFixture, calculateADI) {
  fnd.numericalScheme = Foundation::NS_ADI;
  bool fullyear = false;
  //  fullyear = true;
  if (fullyear) {
    double surface_avg = calculate(8760);
    Kiva::showMessage(MSG_INFO, dbl_to_string(surface_avg));
    EXPECT_NEAR(surface_avg, 2888.473, 0.01);
  } else {
    double surface_avg = calculate();
    Kiva::showMessage(MSG_INFO, dbl_to_string(surface_avg));
    EXPECT_NEAR(surface_avg, 2607.32, 0.01);
  }
}

TEST_F(GC10aFixture, calculateImplicit) {
  fnd.numericalScheme = Foundation::NS_IMPLICIT;

  double surface_avg = calculate();
  Kiva::showMessage(MSG_INFO, dbl_to_string(surface_avg));
  EXPECT_NEAR(surface_avg, 2601.25, 0.01);
}

TEST_F(GC10aFixture, calculateCrankN) {
  fnd.numericalScheme = Foundation::NS_CRANK_NICOLSON;

  double surface_avg = calculate();
  Kiva::showMessage(MSG_INFO, dbl_to_string(surface_avg));
  EXPECT_NEAR(surface_avg, 2600.87, 0.01);
}

TEST_F(GC10aFixture, calculateADE) {
  fnd.numericalScheme = Foundation::NS_ADE;

  double surface_avg = calculate();
  Kiva::showMessage(MSG_INFO, dbl_to_string(surface_avg));
  EXPECT_NEAR(surface_avg, 2615.19, 0.01);
}

TEST_F(GC10aFixture, GC10a_calculateSteadyState) {
  fnd.numericalScheme = Foundation::NS_STEADY_STATE;

  double surface_avg = calculate();
  Kiva::showMessage(MSG_INFO, dbl_to_string(surface_avg));
  EXPECT_NEAR(surface_avg, 3107.57, 0.01);
}

TEST_F(AggregatorFixture, validation) {

  Aggregator floor_results;

  floor_results = Aggregator(Surface::SurfaceType::ST_SLAB_CORE);

  floor_results.add_instance(instances[0].ground.get(), 0.10);
  floor_results.add_instance(instances[1].ground.get(), 0.75);

  EXPECT_DEATH(floor_results.calc_weighted_results(),
               "The weights of associated Kiva instances do not add to unity.");

  floor_results = Aggregator(Surface::SurfaceType::ST_SLAB_CORE);

  EXPECT_DEATH(floor_results.add_instance(Surface::SurfaceType::ST_SLAB_PERIM,
                                          instances[0].ground.get(), 0.25);
               , "Inconsistent surface type added to aggregator.");

  floor_results = Aggregator(Surface::SurfaceType::ST_WALL_INT);

  floor_results.add_instance(instances[0].ground.get(), 0.25);
  floor_results.add_instance(instances[1].ground.get(), 0.75);

  EXPECT_DEATH(floor_results.calc_weighted_results(),
               "Aggregation requested for surface that is not part of foundation instance.");

  floor_results = Aggregator(Surface::SurfaceType::ST_SLAB_CORE);

  floor_results.add_instance(instances[0].ground.get(), 0.25);
  floor_results.add_instance(instances[1].ground.get(), 0.753);
  floor_results.calc_weighted_results();

  // Aggregator will re-weight to make totals add to 1.0--
  // indiviudal weights will now be different.
  EXPECT_NE(floor_results.get_instance(1).second, 0.753);

  floor_results = Aggregator(Surface::SurfaceType::ST_SLAB_CORE);

  floor_results.add_instance(instances[0].ground.get(), 0.25);
  floor_results.add_instance(instances[1].ground.get(), 0.75);
  floor_results.calc_weighted_results(); // Expect success
}

// Test for zero convection from foundation
TEST_F(AggregatorFixture, zeroConvection) {

  Aggregator floor_results;

  floor_results = Aggregator(Surface::SurfaceType::ST_SLAB_CORE);
  floor_results.add_instance(instances[0].ground.get(), 1.0);
  for (auto &surface : instances[0].foundation->surfaces) {
    for (auto index : surface.indices) {
      // Set slab temperature to match air temperature
      instances[0].ground->TNew[index] = 310.15;
    }
  }
  instances[0].ground->calculateSurfaceAverages();
  double Tavg = instances[0].ground->groundOutput.outputValues[{Surface::SurfaceType::ST_SLAB_CORE,
                                                                GroundOutput::OT_TEMP}];
  EXPECT_NEAR(Tavg, 310.15,
              0.01); // Check that Tavg equals slab convective temperature when convection is zero

  instances[0].ground->groundOutput.outputValues[{Surface::SurfaceType::ST_SLAB_CORE,
                                                  GroundOutput::OT_CONV}] = 0;

  floor_results.calc_weighted_results();
  EXPECT_EQ(floor_results.results.Tconv,
            310.15); // Check that Tconv equals slab convective temperature when convection is zero
}

TEST_F(TypicalFixture, convectionCallback) {
  double hc1 = bcs.slabConvectionAlgorithm(290., 295., 0., 0., 0.);
  bcs.slabConvectionAlgorithm = KIVA_CONST_CONV(2.0);
  double hc2 = bcs.slabConvectionAlgorithm(290., 295., 0., 0., 0.);
  EXPECT_NE(hc1, hc2);
  EXPECT_EQ(2.0, hc2);
  bcs.slabConvectionAlgorithm = [=](double Tsurf, double Tamb, double HfTerm, double roughness,
                                    double cosTilt) -> double {
    double deltaT = Tsurf - Tamb;
    return hc2 + deltaT * deltaT + hc1 -
           getDOE2ConvectionCoeff(Tsurf, Tamb, HfTerm, roughness, cosTilt);
  };
  double hc3 = bcs.slabConvectionAlgorithm(290., 295., 0., 0., 0.);

  EXPECT_NEAR(hc3, 27.0, 0.00001);
}

TEST_F(FoundationFixture, foundationSurfaces) {
  fnd.foundationDepth = 1.0;
  fnd.wall.heightAboveGrade = 0.0;
  Material insulation(0.0288, 28.0, 1450.0);
  InputBlock extIns;
  extIns.z = 0;
  extIns.x = fnd.wall.totalWidth();
  extIns.depth = 1.0;
  extIns.width = 0.05;
  extIns.material = insulation;
  fnd.inputBlocks.push_back(extIns);
  fnd.createMeshData();
  Domain domain(fnd);
  EXPECT_EQ(fnd.surfaces[6].type, Surface::ST_GRADE);
  EXPECT_EQ(fnd.surfaces[8].type, Surface::ST_WALL_TOP);
  EXPECT_NEAR(fnd.surfaces[8].xMax, fnd.surfaces[6].xMin, 0.00001);
}

TEST_F(FoundationFixture, foundationSurfaces2) {
  Material insulation(0.0288, 28.0, 1450.0);
  InputBlock intIns;
  intIns.z = 0.0;
  intIns.x = 0.0;
  intIns.depth = 1.0;
  intIns.width = -0.05;
  intIns.material = insulation;
  fnd.inputBlocks.push_back(intIns);
  fnd.createMeshData();
  Domain domain(fnd);
  EXPECT_EQ(fnd.surfaces[5].type, Surface::ST_SLAB_CORE);
  EXPECT_EQ(fnd.surfaces[8].type, Surface::ST_WALL_TOP);
  EXPECT_NEAR(fnd.surfaces[5].xMax, fnd.surfaces[8].xMin, 0.00001);
}

TEST_F(FoundationFixture, clockwiseSurface) {
  Material insulation(0.0288, 28.0, 1450.0);
  InputBlock intIns;
  intIns.z = 0.0;
  intIns.x = 0.0;
  intIns.depth = 1.0;
  intIns.width = -0.05;
  intIns.material = insulation;
  fnd.inputBlocks.push_back(intIns);
  fnd.polygon.clear();
  fnd.polygon.outer().push_back(Point(-6.0, -6.0));
  fnd.polygon.outer().push_back(Point(6.0, -6.0));
  fnd.polygon.outer().push_back(Point(6.0, 6.0));
  fnd.polygon.outer().push_back(Point(-6.0, 6.0));
  EXPECT_FALSE(isCounterClockWise(fnd.polygon));
  fnd.createMeshData();
  EXPECT_TRUE(isCounterClockWise(fnd.polygon));
}

TEST_F(FoundationFixture, verticalSurfaceAreas) {
  fnd.foundationDepth = 2.0;
  fnd.useDetailedExposedPerimeter = true;
  Material insulation(0.0288, 28.0, 1450.0);
  InputBlock intIns;
  intIns.z = 0.0;
  intIns.x = 0.0;
  intIns.depth = 2.0;
  intIns.width = -1.0;
  intIns.material = insulation;
  fnd.inputBlocks.push_back(intIns);
  fnd.polygon.clear();
  fnd.polygon.outer().push_back(Point(-6.0, -6.0));
  fnd.polygon.outer().push_back(Point(-6.0, 3.0));
  fnd.polygon.outer().push_back(Point(3.0, 3.0));
  fnd.polygon.outer().push_back(Point(3.0, 6.0));
  fnd.polygon.outer().push_back(Point(6.0, 6.0));
  fnd.polygon.outer().push_back(Point(6.0, -6.0));
  fnd.isExposedPerimeter.clear();
  fnd.isExposedPerimeter.push_back(true);
  fnd.isExposedPerimeter.push_back(true);
  fnd.isExposedPerimeter.push_back(true);
  fnd.isExposedPerimeter.push_back(false);
  fnd.isExposedPerimeter.push_back(false);
  fnd.isExposedPerimeter.push_back(true);
  fnd.createMeshData();
  Polygon offset_polygon = offset(fnd.polygon, intIns.width);
  EXPECT_NEAR(fnd.surfaceAreas[Surface::SurfaceType::ST_WALL_INT],
              fnd.foundationDepth * boost::geometry::perimeter(offset_polygon) *
                  fnd.exposedFraction,
              0.00001);
}

// Google Test main
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
