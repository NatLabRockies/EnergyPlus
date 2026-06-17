/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Instance_HPP
#define Instance_HPP

#include <memory>

#include "Ground.hpp"

namespace Kiva {

class LIBKIVA_EXPORT Instance {
public:
  Instance() = default;
  Instance(Foundation fnd);
  void create();
  void calculate(double ts = 0.0);
  void calculate_surface_averages();
  std::shared_ptr<Ground> ground;
  std::shared_ptr<Foundation> foundation;
  std::shared_ptr<BoundaryConditions> bcs;
};

} // namespace Kiva

#endif
