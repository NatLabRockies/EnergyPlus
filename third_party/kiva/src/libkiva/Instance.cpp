/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#include "Instance.hpp"

namespace Kiva {

Instance::Instance(Foundation fnd) {
  foundation = std::make_shared<Foundation>(fnd);
  create();
}

void Instance::create() {
  GroundOutput::OutputMap outputMap;

  outputMap.push_back(Surface::ST_SLAB_CORE);

  if (foundation->hasPerimeterSurface) {
    outputMap.push_back(Surface::ST_SLAB_PERIM);
  }

  if (foundation->foundationDepth) {
    outputMap.push_back(Surface::ST_WALL_INT);
  }

  if (!foundation->useDetailedExposedPerimeter || !isConvex(foundation->polygon) ||
      foundation->exposedFraction == 0) {
    if (foundation->reductionStrategy == Foundation::RS_BOUNDARY) {
      foundation->reductionStrategy = Foundation::RS_AP;
    }
  }

  ground = std::make_shared<Ground>(*foundation.get(), outputMap);

  if (foundation->reductionStrategy == Foundation::RS_BOUNDARY) {
    ground->calculateBoundaryLayer();
    ground->setNewBoundaryGeometry();
  }

  ground->buildDomain();
}

void Instance::calculate(double ts) { ground->calculate(*bcs, ts); }

void Instance::calculate_surface_averages() { ground->calculateSurfaceAverages(); }

} // namespace Kiva
