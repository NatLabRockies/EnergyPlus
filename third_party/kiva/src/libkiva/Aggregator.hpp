/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Aggregator_HPP
#define Aggregator_HPP

#include "Ground.hpp"

namespace Kiva {

class LIBKIVA_EXPORT Aggregator {
public:
  Aggregator();
  Aggregator(Surface::SurfaceType st);
  void add_instance(Surface::SurfaceType st, Ground *grnd, double weight);
  void add_instance(Ground *grnd, double weight);
  std::size_t size();
  void calc_weighted_results();
  std::pair<Ground *, double> get_instance(std::size_t index);

  struct Results {
    double hconv, hrad, qtot, qconv, qrad, Tconv, Tavg, Trad;
    void reset() { hconv = hrad = qtot = qconv = qrad = Tconv = Tavg = Trad = 0.0; }
  };

  Results results;

private:
  void validate();
  std::vector<std::pair<Ground *, double>> instances;
  Surface::SurfaceType surface_type;
  bool surface_type_set, validated;
};

} // namespace Kiva

#endif
