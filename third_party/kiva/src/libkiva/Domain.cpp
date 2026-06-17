/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Domain_CPP
#define Domain_CPP

#include "Domain.hpp"

namespace Kiva {

static const double PI = 4.0 * atan(1.0);

Domain::Domain() {}

Domain::Domain(Foundation &foundation) { setDomain(foundation); }

// Having this separate from the constructor allows the correct resizing of
// multidimensional arrays on pre-existing initialized instances.
void Domain::setDomain(Foundation &foundation) {

  {
    mesh[0] = Mesher(foundation.xMeshData);
    mesh[1] = Mesher(foundation.yMeshData);
    mesh[2] = Mesher(foundation.zMeshData);
  }

  for (std::size_t dim = 0; dim < 3; ++dim) {
    dim_lengths[dim] = mesh[dim].centers.size();
  }

  std::vector<double> dxp_vector, dxm_vector, dyp_vector, dym_vector, dzp_vector, dzm_vector;
  for (std::size_t i = 0; i < dim_lengths[0]; i++) {
    dxp_vector.emplace_back(getDistances(i, 0, 1));
    dxm_vector.emplace_back(getDistances(i, 0, 0));
  }
  for (std::size_t j = 0; j < dim_lengths[1]; j++) {
    dyp_vector.emplace_back(getDistances(j, 1, 1));
    dym_vector.emplace_back(getDistances(j, 1, 0));
  }
  for (std::size_t k = 0; k < dim_lengths[2]; k++) {
    dzp_vector.emplace_back(getDistances(k, 2, 1));
    dzm_vector.emplace_back(getDistances(k, 2, 0));
  }

  stepsize[0] = 1;
  stepsize[1] = dim_lengths[0];
  stepsize[2] = dim_lengths[0] * dim_lengths[1];
  std::size_t num_cells = dim_lengths[0] * dim_lengths[1] * dim_lengths[2];
  cell.reserve(num_cells);
  dest_index_vector.resize(3, std::vector<std::size_t>(num_cells));
  std::vector<std::size_t> temp_di(3);
  std::size_t i, j, k;
  CellType cellType;

  for (std::size_t index = 0; index < num_cells; index++) {
    std::tie(i, j, k) = getCoordinates(index);
    temp_di = getDestIndex(i, j, k);
    for (std::size_t d = 0; d < 3; d++) {
      dest_index_vector[d][index] = temp_di[d];
    }

    cellType = CellType::NORMAL;
    Surface *surfacePtr;

    int numZeroDims = getNumZeroDims(i, j, k);
    bool xNotBoundary = i != 0 && i != dim_lengths[0] - 1;
    bool yNotBoundary = j != 0 && j != dim_lengths[1] - 1;
    bool zNotBoundary = k != 0 && k != dim_lengths[2] - 1;

    std::size_t surfaceAssignmentCount(0);

    for (auto &surface : foundation.surfaces) {
      if (pointOnPoly(Point(mesh[0].centers[i], mesh[1].centers[j]), surface.polygon)) {
        if (isGreaterOrEqual(mesh[2].centers[k], surface.zMin) &&
            isLessOrEqual(mesh[2].centers[k], surface.zMax)) {
          cellType = CellType::BOUNDARY;
          surfacePtr = &surface;

          if (numZeroDims == 0) {
            // Shouldn't be a surface cell
            showMessage(MSG_ERR, "A normal cell was detected within a surface.");
          }

          // Point/Line cells not on the boundary should be
          // zero-thickness cells
          if (foundation.numberOfDimensions == 3) {
            if ((numZeroDims > 1) && xNotBoundary && yNotBoundary && zNotBoundary) {
              cellType = CellType::ZERO_THICKNESS;
            }
          } else if (foundation.numberOfDimensions == 2) {
            if ((numZeroDims > 1) && xNotBoundary && zNotBoundary) {
              cellType = CellType::ZERO_THICKNESS;
            }
          } else {
            if ((numZeroDims > 1) && zNotBoundary) {
              cellType = CellType::ZERO_THICKNESS;
            }
          }

          if (cellType == CellType::BOUNDARY) {
            // Assign cell index to surface
            surface.indices.push_back(index);
            surfaceAssignmentCount++;
            if (surfaceAssignmentCount > 1 && numZeroDims <= 1) {
              // If the index of a surface cell is assigned to multiple surfaces, throw an error
              showMessage(MSG_ERR, "A cell has been assigned to multiple surfaces.");
            }
          }
        }
      }
    }

    // this cell creation needs to be separate from the previous for loop to prevent
    // double-instantiation.
    if (cellType == CellType::ZERO_THICKNESS) {
      std::shared_ptr<ZeroThicknessCell> sp = std::make_shared<ZeroThicknessCell>(
          index, cellType, i, j, k, stepsize, foundation, surfacePtr, nullptr, mesh);
      cell.emplace_back(std::move(sp));
    } else if (cellType == CellType::BOUNDARY) {
      std::shared_ptr<BoundaryCell> sp = std::make_shared<BoundaryCell>(
          index, cellType, i, j, k, stepsize, foundation, surfacePtr, nullptr, mesh);
      cell.emplace_back(std::move(sp));
    } else { // if (cellType == CellType::NORMAL)
      Block *blockPtr = nullptr;
      // Loop through blocks. Use the last block that contains the point.
      for (auto &block : foundation.blocks) {
        if (boost::geometry::within(Point(mesh[0].centers[i], mesh[1].centers[j]), block.polygon) &&
            isGreaterThan(mesh[2].centers[k], block.zMin) &&
            isLessThan(mesh[2].centers[k], block.zMax)) {
          blockPtr = &block;
        }
      }
      if (blockPtr) {
        if (blockPtr->blockType == Block::INTERIOR_AIR) {
          cellType = CellType::INTERIOR_AIR;
          std::shared_ptr<InteriorAirCell> sp = std::make_shared<InteriorAirCell>(
              index, cellType, i, j, k, stepsize, foundation, nullptr, blockPtr, mesh);
          cell.emplace_back(std::move(sp));
        } else if (blockPtr->blockType == Block::EXTERIOR_AIR) {
          cellType = CellType::EXTERIOR_AIR;
          std::shared_ptr<ExteriorAirCell> sp = std::make_shared<ExteriorAirCell>(
              index, cellType, i, j, k, stepsize, foundation, nullptr, blockPtr, mesh);
          cell.emplace_back(std::move(sp));
        } else {
          std::shared_ptr<Cell> sp = std::make_shared<Cell>(index, cellType, i, j, k, stepsize,
                                                            foundation, nullptr, blockPtr, mesh);
          cell.emplace_back(std::move(sp));
        }
      } else {
        // If not surface or block, find interior zero-width cells
        if (foundation.numberOfDimensions == 3) {
          if (isEqual(mesh[0].deltas[i], 0.0) || isEqual(mesh[2].deltas[k], 0.0) ||
              isEqual(mesh[1].deltas[j], 0.0)) {
            cellType = CellType::ZERO_THICKNESS;
          }
        } else if (foundation.numberOfDimensions == 2) {
          if (isEqual(mesh[0].deltas[i], 0.0) || isEqual(mesh[2].deltas[k], 0.0)) {
            cellType = CellType::ZERO_THICKNESS;
          }
        } else {
          if (isEqual(mesh[2].deltas[k], 0.0)) {
            cellType = CellType::ZERO_THICKNESS;
          }
        }

        if (cellType == CellType::ZERO_THICKNESS) {
          std::shared_ptr<ZeroThicknessCell> sp = std::make_shared<ZeroThicknessCell>(
              index, cellType, i, j, k, stepsize, foundation, nullptr, nullptr, mesh);
          cell.emplace_back(std::move(sp));
        } else {
          std::shared_ptr<Cell> sp = std::make_shared<Cell>(index, cellType, i, j, k, stepsize,
                                                            foundation, nullptr, nullptr, mesh);
          cell.emplace_back(std::move(sp));
        }
      }
    }
  }

  // Set effective properties of zero-thickness cells
  // based on other cells
  for (const auto &this_cell : cell) {
    std::size_t index = this_cell->index;
    std::tie(i, j, k) = getCoordinates(index);

    int numZeroDims = getNumZeroDims(i, j, k);

    if (numZeroDims > 0 && this_cell->cellType != CellType::INTERIOR_AIR &&
        this_cell->cellType != CellType::EXTERIOR_AIR) {
      if (foundation.numberOfDimensions == 3) {
        if (i != 0 && i != dim_lengths[0] - 1 && j != 0 && j != dim_lengths[1] - 1 && k != 0 &&
            k != dim_lengths[2] - 1)
          set3DZeroThicknessCellProperties(index);
      } else if (foundation.numberOfDimensions == 2) {
        if (i != 0 && i != dim_lengths[0] - 1 && k != 0 && k != dim_lengths[2] - 1)
          set2DZeroThicknessCellProperties(index);
      } else {
        if (k != 0 && k != dim_lengths[2] - 1) {
          if (isEqual(mesh[2].deltas[k], 0.0)) {
            std::vector<std::shared_ptr<Cell>> pointSet = {cell[index - stepsize[2]],
                                                           cell[index + stepsize[2]]};

            this_cell->setZeroThicknessCellProperties(pointSet);
          }
        }
      }
    }
  }

  std::size_t dims[3]{0, 1, 2};
  if (foundation.numberOfDimensions < 3) {
    dims[1] = 5;
  }
  if (foundation.numberOfDimensions == 1) {
    dims[0] = 5;
  }

  // Calculate matrix coefficients
  for (const auto &this_cell : cell) {
    // PDE Coefficients
    this_cell->setComputeDims(dims);
    this_cell->setDistances(dxp_vector[this_cell->coords[0]], dxm_vector[this_cell->coords[0]],
                            dyp_vector[this_cell->coords[1]], dym_vector[this_cell->coords[1]],
                            dzp_vector[this_cell->coords[2]], dzm_vector[this_cell->coords[2]]);
    this_cell->setConductivities(cell);
    this_cell->setPDEcoefficients(foundation.numberOfDimensions,
                                  foundation.coordinateSystem == Foundation::CS_CYLINDRICAL);
  }

  static std::map<Surface::Orientation, std::tuple<int, int, double>> orientation_map{
      {Surface::X_POS, std::make_tuple(0, 0, PI / 2 + foundation.orientation)},
      {Surface::X_NEG, std::make_tuple(0, 1, 3 * PI / 2 + foundation.orientation)},
      {Surface::Y_POS, std::make_tuple(1, 0, foundation.orientation)},
      {Surface::Y_NEG, std::make_tuple(1, 1, PI + foundation.orientation)},
      {Surface::Z_POS, std::make_tuple(2, 0, 0.0)},
      {Surface::Z_NEG, std::make_tuple(2, 1, 0.0)}};

  for (auto &surface : foundation.surfaces) {
    surface.calcTilt();
    surface.area = 0;
    for (auto index : surface.indices) {
      surface.area += cell[index]->area;
    }
    std::tie(surface.orientation_dim, surface.orientation_dir, surface.azimuth) =
        orientation_map[surface.orientation];
  }
}

double Domain::getDistances(std::size_t i, std::size_t dim, std::size_t dir) {
  if (dim_lengths[dim] == 1) {
    return 0;
  } else if (dir == 0) {
    if (i == 0) {
      // For boundary cells assume that the cell on the other side of the
      // boundary is the same as the previous cell
      return (mesh[dim].deltas[i] + mesh[dim].deltas[i + 1]) / 2.0;
    } else {
      return (mesh[dim].deltas[i] + mesh[dim].deltas[i - 1]) / 2.0;
    }
  } else /* if (dir == 1) */ {
    if (i == dim_lengths[dim] - 1) {
      return (mesh[dim].deltas[i] + mesh[dim].deltas[i - 1]) / 2.0;
    } else {
      return (mesh[dim].deltas[i] + mesh[dim].deltas[i + 1]) / 2.0;
    }
  }
}

void Domain::set2DZeroThicknessCellProperties(std::size_t index) {
  if (isEqual(mesh[0].deltas[cell[index]->coords[0]], 0.0) &&
      isEqual(mesh[2].deltas[cell[index]->coords[2]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {
        cell[index - stepsize[0] + stepsize[2]], cell[index + stepsize[0] + stepsize[2]],
        cell[index - stepsize[0] - stepsize[2]], cell[index + stepsize[0] - stepsize[2]]};
    cell[index]->setZeroThicknessCellProperties(pointSet);
  } else if (isEqual(mesh[0].deltas[cell[index]->coords[0]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {cell[index - stepsize[0]],
                                                   cell[index + stepsize[0]]};
    cell[index]->setZeroThicknessCellProperties(pointSet);
  } else if (isEqual(mesh[2].deltas[cell[index]->coords[2]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {cell[index - stepsize[2]],
                                                   cell[index + stepsize[2]]};
    cell[index]->setZeroThicknessCellProperties(pointSet);
  }
}

void Domain::set3DZeroThicknessCellProperties(std::size_t index) {
  if (isEqual(mesh[0].deltas[cell[index]->coords[0]], 0.0) &&
      isEqual(mesh[1].deltas[cell[index]->coords[1]], 0.0) &&
      isEqual(mesh[2].deltas[cell[index]->coords[2]], 0.0)) {
    // Use all 8 full volume cells
    std::vector<std::shared_ptr<Cell>> pointSet = {
        cell[index - stepsize[0] - stepsize[1] + stepsize[2]],
        cell[index + stepsize[0] - stepsize[1] + stepsize[2]],
        cell[index - stepsize[0] - stepsize[1] - stepsize[2]],
        cell[index + stepsize[0] - stepsize[1] - stepsize[2]],
        cell[index - stepsize[0] + stepsize[1] + stepsize[2]],
        cell[index + stepsize[0] + stepsize[1] + stepsize[2]],
        cell[index - stepsize[0] + stepsize[1] - stepsize[2]],
        cell[index + stepsize[0] + stepsize[1] - stepsize[2]]};

    cell[index]->setZeroThicknessCellProperties(pointSet);
  } else if (isEqual(mesh[0].deltas[cell[index]->coords[0]], 0.0) &&
             isEqual(mesh[1].deltas[cell[index]->coords[1]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {
        cell[index - stepsize[0] - stepsize[1]], cell[index + stepsize[0] - stepsize[1]],
        cell[index - stepsize[0] + stepsize[1]], cell[index + stepsize[0] + stepsize[1]]};

    cell[index]->setZeroThicknessCellProperties(pointSet);
  } else if (isEqual(mesh[0].deltas[cell[index]->coords[0]], 0.0) &&
             isEqual(mesh[2].deltas[cell[index]->coords[2]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {
        cell[index - stepsize[0] + stepsize[2]], cell[index + stepsize[0] + stepsize[2]],
        cell[index - stepsize[0] - stepsize[2]], cell[index + stepsize[0] - stepsize[2]]};

    cell[index]->setZeroThicknessCellProperties(pointSet);
  } else if (isEqual(mesh[1].deltas[cell[index]->coords[1]], 0.0) &&
             isEqual(mesh[2].deltas[cell[index]->coords[2]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {
        cell[index - stepsize[1] + stepsize[2]],
        cell[index + stepsize[1] + stepsize[2]],
        cell[index - stepsize[1] - stepsize[2]],
        cell[index + stepsize[1] - stepsize[2]],
    };

    cell[index]->setZeroThicknessCellProperties(pointSet);
  } else if (isEqual(mesh[0].deltas[cell[index]->coords[0]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {cell[index + stepsize[0]],
                                                   cell[index - stepsize[0]]};

    cell[index]->setZeroThicknessCellProperties(pointSet);
  } else if (isEqual(mesh[1].deltas[cell[index]->coords[1]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {cell[index + stepsize[1]],
                                                   cell[index - stepsize[1]]};

    cell[index]->setZeroThicknessCellProperties(pointSet);
  } else if (isEqual(mesh[2].deltas[cell[index]->coords[2]], 0.0)) {
    std::vector<std::shared_ptr<Cell>> pointSet = {cell[index + stepsize[2]],
                                                   cell[index - stepsize[2]]};

    cell[index]->setZeroThicknessCellProperties(pointSet);
  }
}

int Domain::getNumZeroDims(std::size_t i, std::size_t j, std::size_t k) {
  int numZeroDims = 0;
  if (isEqual(mesh[0].deltas[i], 0.0))
    numZeroDims += 1;
  if (isEqual(mesh[1].deltas[j], 0.0))
    numZeroDims += 1;
  if (isEqual(mesh[2].deltas[k], 0.0))
    numZeroDims += 1;

  return numZeroDims;
}

void Domain::printCellTypes() {
  // TODO: Make the ability to output a specific slice in i, j, or k
  std::ofstream output;
  output.open("Cells.csv");

  for (std::size_t i = 0; i < dim_lengths[0]; i++) {

    output << ", " << i;
  }

  output << "\n";

  for (std::size_t n = dim_lengths[2]; n > 0; n--) {

    std::size_t k = n - 1;
    output << k;

    for (std::size_t i = 0; i < dim_lengths[0]; i++) {

      output << ", ";

      output << cell[i + (dim_lengths[1] / 2) * stepsize[1] + k * stepsize[2]]->cellType;

      //      output << cell[i + (dim_lengths[1]/2)*stepsize[1] + k*stepsize[2]]->pde[2][0];

      //      if (cell[i + (dim_lengths[1]/2)*stepsize[1] + k*stepsize[2]]->surfacePtr) {
      //        output << cell[i + (dim_lengths[1]/2)*stepsize[1] +
      //        k*stepsize[2]]->surfacePtr->type;
      //      } else {
      //        output << "";
      //      }
    }

    output << "\n";
  }
  output.close();
}

std::tuple<std::size_t, std::size_t, std::size_t> Domain::getCoordinates(std::size_t index) {
  size_t i, j, k;
  i = index % dim_lengths[0];
  j = ((index - i) % dim_lengths[1]) / dim_lengths[0];
  k = (index - i - dim_lengths[0] * j) / (dim_lengths[0] * dim_lengths[1]);
  return std::make_tuple(i, j, k);
}

std::vector<std::size_t> Domain::getDestIndex(std::size_t i, std::size_t j, std::size_t k) {
  std::vector<std::size_t> dest_index;
  dest_index.emplace_back(i + dim_lengths[0] * j + dim_lengths[0] * dim_lengths[1] * k);
  dest_index.emplace_back(j + dim_lengths[1] * i + dim_lengths[1] * dim_lengths[0] * k);
  dest_index.emplace_back(k + dim_lengths[2] * i + dim_lengths[2] * dim_lengths[0] * j);
  return dest_index;
}

std::size_t Domain::getIndex(std::size_t i, std::size_t j, std::size_t k) {
  return i + dim_lengths[0] * j + dim_lengths[0] * dim_lengths[1] * k;
}

} // namespace Kiva

#endif
