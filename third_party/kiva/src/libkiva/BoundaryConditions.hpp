/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef BoundaryConditions_HPP
#define BoundaryConditions_HPP

#include "Algorithms.hpp"

namespace Kiva {

class BoundaryConditions {
public:
  double slabConvectiveTemp;
  double wallConvectiveTemp;
  double slabRadiantTemp;
  double wallRadiantTemp;
  double outdoorTemp;
  double localWindSpeed;
  double windDirection;
  double solarAzimuth;
  double solarAltitude;
  double directNormalFlux;
  double diffuseHorizontalFlux;
  double skyEmissivity;
  double slabAbsRadiation;
  double wallAbsRadiation;
  double deepGroundTemperature;

  ConvectionAlgorithm slabConvectionAlgorithm;
  ConvectionAlgorithm intWallConvectionAlgorithm;
  ConvectionAlgorithm extWallConvectionAlgorithm;
  ConvectionAlgorithm gradeConvectionAlgorithm;

  ForcedConvectionTerm extWallForcedTerm;
  ForcedConvectionTerm gradeForcedTerm;

  BoundaryConditions()
      : slabConvectiveTemp(293.15), wallConvectiveTemp(293.15), slabRadiantTemp(293.15),
        wallRadiantTemp(293.15), outdoorTemp(273.15), localWindSpeed(0.0), windDirection(0.0),
        solarAzimuth(3.14), solarAltitude(0.0), directNormalFlux(0.0), diffuseHorizontalFlux(0.0),
        skyEmissivity(0.0), slabAbsRadiation(0.0), wallAbsRadiation(0.0),
        deepGroundTemperature(283.15), slabConvectionAlgorithm(&getDOE2ConvectionCoeff),
        intWallConvectionAlgorithm(&getDOE2ConvectionCoeff),
        extWallConvectionAlgorithm(&getDOE2ConvectionCoeff),
        gradeConvectionAlgorithm(&getDOE2ConvectionCoeff), extWallForcedTerm(&getMoWiTTForcedTerm),
        gradeForcedTerm(&getMoWiTTForcedTerm) {}
};

} // namespace Kiva
#endif // BoundaryConditions_HPP
