/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#include <cmath>

#include "Aggregator.hpp"
#include "Errors.hpp"
#include "Foundation.hpp"

namespace Kiva {

Aggregator::Aggregator() : surface_type_set(false), validated(false) {}

Aggregator::Aggregator(Surface::SurfaceType st)
    : surface_type(st), surface_type_set(true), validated(false) {}

void Aggregator::add_instance(Surface::SurfaceType st, Ground *grnd, double weight) {
  // Check if surface type is the same as already set. If not yet set, set it now.
  if (!surface_type_set) {
    surface_type = st;
  } else if (st != surface_type) {
    showMessage(MSG_ERR, "Inconsistent surface type added to aggregator.");
  }
  add_instance(grnd, weight);
}

void Aggregator::add_instance(Ground *grnd, double weight) { instances.push_back({grnd, weight}); }

std::size_t Aggregator::size() { return instances.size(); }

void Aggregator::validate() {
  // Check if surface type exists in all instances
  // Check if weights add up to 1.0
  double check_weights = 0.0;
  for (auto &instance : instances) {
    Ground *grnd = instance.first;
    check_weights += instance.second;
    if (!grnd->foundation.hasSurface[surface_type]) {
      showMessage(MSG_ERR,
                  "Aggregation requested for surface that is not part of foundation instance.");
    }
  }
  if (!isEqual(check_weights, 1.0)) {
    if (isEqual(check_weights, 1.0, 0.01)) {
      showMessage(MSG_WARN,
                  "The weights of associated Kiva instances do not quite add to unity--check "
                  "exposed perimeter values. Weights will be slightly modified to add to unity.");
      for (auto &instance : instances) {
        instance.second /= check_weights;
      }
    } else {
      showMessage(MSG_ERR, "The weights of associated Kiva instances do not add to unity--check "
                           "exposed perimeter values.");
    }
  }
  validated = true;
}

void Aggregator::calc_weighted_results() {
  if (!validated) {
    validate();
  }
  results.reset();
  double Tz{293.15}, Tr{293.15};
  for (auto &instance : instances) {
    Ground *grnd = instance.first;
    Tz = surface_type == Surface::ST_WALL_INT ? grnd->bcs.wallConvectiveTemp
                                              : grnd->bcs.slabConvectiveTemp;
    Tr = surface_type == Surface::ST_WALL_INT ? grnd->bcs.wallRadiantTemp
                                              : grnd->bcs.slabRadiantTemp;
    double p = instance.second;
    double hci = grnd->getSurfaceAverageValue({surface_type, Kiva::GroundOutput::OT_CONV});
    double hri = grnd->getSurfaceAverageValue({surface_type, Kiva::GroundOutput::OT_RAD});
    double Ts = grnd->getSurfaceAverageValue({surface_type, Kiva::GroundOutput::OT_TEMP});
    double Ta = grnd->getSurfaceAverageValue({surface_type, Kiva::GroundOutput::OT_AVG_TEMP});
    double qi = -grnd->getSurfaceAverageValue({surface_type, Kiva::GroundOutput::OT_FLUX});

    if (!std::isfinite(Ts)) {
      showMessage(MSG_ERR, "Kiva is not giving realistic results!");
    }

    results.qconv += p * hci * (Tz - Ts);
    results.qrad += p * hri * (Tr - Ts);
    results.qtot += p * qi;
    results.hconv += p * hci;
    results.hrad += p * hri;
    results.Tavg += p * Ta;
  }

  results.Tconv = results.hconv == 0 ? Tz : Tz - results.qconv / results.hconv;
  results.Trad = results.hrad == 0 ? Tr : Tr - results.qrad / results.hrad;

  return;
}

std::pair<Ground *, double> Aggregator::get_instance(std::size_t index) { return instances[index]; }

} // namespace Kiva
