/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <memory>

#ifndef NDEBUG
#ifdef __unix__
#include <cfenv>
#endif
#endif

// Penumbra
#include "penumbra-implementation.h"

namespace Penumbra {

PenumbraImplementation::PenumbraImplementation(int size,
                                               const std::shared_ptr<Courierr::Courierr> &logger_in)
    : context(size, logger_in.get()), logger(logger_in) {}

void PenumbraImplementation::add_surface(const Surface &surface) {
  surface.surface->logger = logger;
  if (surface.surface->name.empty()) {
    surface.surface->name = std::format("Surface {}", surfaces.size());
  }
  surfaces.push_back(*surface.surface);
}

void PenumbraImplementation::check_surface(const unsigned int surface_index,
                                           const std::string_view &surface_context) const {
  if (surface_index >= surfaces.size()) {
    throw SurfaceException(surface_index, surface_context, *(logger));
  }
}

} // namespace Penumbra
