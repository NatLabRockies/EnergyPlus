/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Domain_HPP
#define Domain_HPP

#include "Cell.hpp"
#include "Errors.hpp"
#include "Foundation.hpp"
#include "Functions.hpp"
#include "Mesher.hpp"
#include "libkiva_export.h"

#include <fstream>
#include <memory>
#include <numeric>

namespace Kiva {

class LIBKIVA_EXPORT Domain {
public:
  // mesh
  Mesher mesh[3];
  std::size_t dim_lengths[3];
  std::size_t stepsize[3];

  std::vector<std::shared_ptr<Cell>> cell;
  std::vector<std::vector<std::size_t>> dest_index_vector;

public:
  Domain();
  Domain(Foundation &foundation);
  void setDomain(Foundation &foundation);
  int getNumZeroDims(std::size_t i, std::size_t j, std::size_t k);
  double getDistances(std::size_t i, std::size_t dim, std::size_t dir);
  void set2DZeroThicknessCellProperties(std::size_t index);
  void set3DZeroThicknessCellProperties(std::size_t index);
  void printCellTypes();
  std::tuple<std::size_t, std::size_t, std::size_t> getCoordinates(std::size_t index);
  std::vector<std::size_t> getDestIndex(std::size_t i, std::size_t j, std::size_t k);
  std::size_t getIndex(std::size_t i, std::size_t j, std::size_t k);
};

} // namespace Kiva

#endif
