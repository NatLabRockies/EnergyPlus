/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef ConvectionAlgorithms_CPP
#define ConvectionAlgorithms_CPP

#include <map>

#include "Algorithms.hpp"

#define MEMOIZE_DEF(FCN, NAME)                                                                     \
  namespace Memo {                                                                                 \
  double NAME(double x) {                                                                          \
    if (NAME##_map.count(x) > 0) {                                                                 \
      return NAME##_map[x];                                                                        \
    } else {                                                                                       \
      return NAME##_map[x] = FCN;                                                                  \
    }                                                                                              \
  }                                                                                                \
  }

namespace Kiva {

static const double PI = 4.0 * atan(1.0);
static const double SIGMA = 5.67 * pow(10.0, -8); // [W/m2-K4]

MEMOIZE_DEF(std::cos(x), cos)
MEMOIZE_DEF(std::sqrt(std::sqrt(x)), pow025)
MEMOIZE_DEF(pow(x, 0.89), pow089)
MEMOIZE_DEF(pow(x, 0.617), pow0617)

double cbrt_a(double x) {
  // faster approximation of pow(x, 1.0/3.0)
  // from http://web.archive.org/web/20120620192258/http://metamerist.com/cbrt/cbrt.htm
  // using Kahan's method as an initail guess for Halley's method.
  const unsigned int B1 = 715094163;
  double t = 0.0;
  unsigned int *pt = (unsigned int *)&t;
  unsigned int *px = (unsigned int *)&x;
  pt[1] = px[1] / 3 + B1;

  const double a3 = t * t * t;
  const double b = t * (a3 + x + x) / (a3 + a3 + x);
  return b;
}

double getMoWiTTForcedTerm(double cosTilt, double azimuth, double windDir, double windSpeed) {
  if (isWindward(cosTilt, azimuth, windDir)) {
    return 3.26 * Memo::pow089(windSpeed);
  } else {
    return 3.55 * Memo::pow0617(windSpeed);
  }
}

double getDOE2ConvectionCoeff(double Tsurf, double Tamb, double hfTerm, double roughness,
                              double cosTilt) {
  /* Based on the DOE-2 convection model as used in EnergyPlus
   *
   * Roughness factors:
   * Very Rough = 2.17
   * Rough = 1.67
   * Medium Rough = 1.52
   * Medium Smooth = 1.13
   * Smooth = 1.11
   * Very Smooth = 1.0
   *
   * These values correspond roughly to the relief in milimeters. We ask
   * for rougness in meters instead, so we multiply by 100.
   */

  double hn;
  double dT3rd = cbrt_a(fabs(Tsurf - Tamb));

  if (cosTilt == 0.0) {
    hn = 1.31 * dT3rd;
  } else if ((cosTilt < 0.0 && Tsurf < Tamb) || (cosTilt > 0.0 && Tsurf > Tamb)) {
    hn = 9.482 * dT3rd / (7.283 - fabs(cosTilt));
  } else /*if ((cosTilt < 0.0 && Tsurf > Tamb) ||
         (cosTilt > 0.0 && Tsurf < Tamb)) */
  {
    hn = 1.810 * dT3rd / (1.382 + fabs(cosTilt));
  }

  double hcGlass = sqrt(hn * hn + hfTerm * hfTerm);

  double rf = 1 + roughness / 0.004; // convert meters to milimeters

  double hf = rf * (hcGlass - hn);
  return hn + hf;
}

bool isWindward(double cosTilt, double azimuth, double windDirection) {
  if (fabs(cosTilt) < 0.98) {
    double diff = fabs(windDirection - azimuth);
    if ((diff - PI) > 0.001) {
      diff -= 2 * PI;
    }
    if (fabs((diff)-100.0 * PI / 180.0) > 0.001) {
      return false;
    } else
      return true;
  } else
    return true;
}

double getExteriorIRCoeff(double eSurf, double Tsurf, double Tamb, double Fqtr) {
  return eSurf * SIGMA * (Tamb * Tamb * Fqtr * Fqtr + Tsurf * Tsurf) * (Tamb * Fqtr + Tsurf);
}

double getEffectiveExteriorViewFactor(double eSky, double tilt) {
  double Fsky = 0.5 * (1.0 + Memo::cos(tilt));
  double beta = Memo::cos(tilt * 0.5);
  return Fsky * beta * (eSky - 1.0) + 1.0;
}

double getSimpleInteriorIRCoeff(double eSurf, double Tsurf, double Trad) {
  return eSurf * SIGMA * (Trad * Trad + Tsurf * Tsurf) * (Trad + Tsurf);
}

} // namespace Kiva
#endif
