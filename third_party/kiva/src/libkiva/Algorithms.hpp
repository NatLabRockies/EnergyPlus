/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef ConvectionAlgorithms_HPP
#define ConvectionAlgorithms_HPP

#include <cmath>
#include <functional>
#include <map>

#include "libkiva_export.h"

#define MEMOIZE_DECL(NAME)                                                                         \
  namespace Memo {                                                                                 \
  static std::map<double, double> NAME##_map;                                                      \
  double NAME(double x);                                                                           \
  }

#define KIVA_CONST_CONV(HC) [=](double, double, double, double, double) -> double { return HC; }
#define KIVA_HF_DEF [=](double, double, double, double windSpeed) -> double { return windSpeed; }
#define KIVA_HF_ZERO [=](double, double, double, double) -> double { return 0.0; }

namespace Kiva {

MEMOIZE_DECL(cos)
MEMOIZE_DECL(pow025)
MEMOIZE_DECL(pow089)
MEMOIZE_DECL(pow0617)

typedef std::function<double(double, double, double, double, double)> ConvectionAlgorithm;

typedef std::function<double(double, double, double, double)> ForcedConvectionTerm;

double cbrt_a(double x);

double LIBKIVA_EXPORT getMoWiTTForcedTerm(double cosTilt, double azimuth, double windDir,
                                          double windSpeed);

double LIBKIVA_EXPORT getDOE2ConvectionCoeff(double Tsurf, double Tamb, double hfTerm,
                                             double roughness, double cosTilt);

bool isWindward(double cosTilt, double azimuth, double windDirection);

double getExteriorIRCoeff(double eSurf, double Tsurf, double Tamb, double Fqtr);

double getEffectiveExteriorViewFactor(double eSky, double tilt);

double getSimpleInteriorIRCoeff(double eSurf, double Tsurf, double Trad);

} // namespace Kiva

#endif
