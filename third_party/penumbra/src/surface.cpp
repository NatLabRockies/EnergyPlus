/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <vector>
#include <array>

// Penumbra
#include <penumbra/surface.h>
#include "surface-implementation.h"

namespace Penumbra {

Surface::Surface() {
  surface = std::make_shared<SurfaceImplementation>();
}

Surface::Surface(const Polygon &polygon, const std::string &name_in) {
  surface = std::make_shared<SurfaceImplementation>(polygon);
  surface->name = name_in;
}

Surface::Surface(const Surface &surface_in) {
  surface = surface_in.surface;
}

Surface::~Surface() = default;

void Surface::add_hole(const Polygon &hole) {
  surface->holes.push_back(hole);
}

} // namespace Penumbra
