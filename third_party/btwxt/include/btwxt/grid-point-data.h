/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

// Standard
#include <algorithm>
#include <cfloat>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

// btwxt
#include "logging.h"

namespace Btwxt {

class GridPointDataSet {
    // Data corresponding to all points within a collection of grid axes. Length of data should
    // equal the total number of permutations of grid axes points.
  public:
    // Constructors
    GridPointDataSet() = default;

    explicit GridPointDataSet(std::vector<double> data, std::string name = "")
        : data(std::move(data)), name(std::move(name))
    {
    }
    std::vector<double> data;
    std::string name;
};

} // namespace Btwxt
