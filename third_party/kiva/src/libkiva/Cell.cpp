/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Cell_CPP
#define Cell_CPP

#include "Cell.hpp"

namespace Kiva {

static const double PI = 4.0 * atan(1.0);

Cell::Cell(const std::size_t &index, const CellType cellType, const std::size_t &i,
           const std::size_t &j, const std::size_t &k, std::size_t *stepsize,
           const Foundation &foundation, Surface *surfacePtr, Block *blockPtr, Mesher *meshPtr)
    : coords{i, j, k}, index(index), stepsize(stepsize), cellType(cellType), blockPtr(blockPtr),
      surfacePtr(surfacePtr), meshPtr(meshPtr) {
  Assemble(foundation);
}

void Cell::Assemble(const Foundation &foundation) {
  if (blockPtr) {
    density = blockPtr->material.density;
    specificHeat = blockPtr->material.specificHeat;
    conductivity = blockPtr->material.conductivity;
  } else {
    density = foundation.soil.density;
    specificHeat = foundation.soil.specificHeat;
    conductivity = foundation.soil.conductivity;
  }
  heatGain = 0.0;
  volume =
      meshPtr[0].deltas[coords[0]] * meshPtr[1].deltas[coords[1]] * meshPtr[2].deltas[coords[2]];

  iHeatCapacity = 1 / (density * specificHeat);
  iHeatCapacityADI = iHeatCapacity / foundation.numberOfDimensions; // because of each sub-timestep

  if (foundation.numberOfDimensions == 2) {
    r = meshPtr[0].centers[coords[0]];
  }
}

void Cell::setDistances(const double dxp_in, const double dxm_in, const double dyp_in,
                        const double dym_in, const double dzp_in, const double dzm_in) {
  dist[0][1] = dxp_in;
  dist[0][0] = dxm_in;
  dist[1][1] = dyp_in;
  dist[1][0] = dym_in;
  dist[2][1] = dzp_in;
  dist[2][0] = dzm_in;
}

void Cell::setConductivities(const std::vector<std::shared_ptr<Cell>> &cell_v) {
  for (std::size_t dim = 0; dim < 3; ++dim) {
    //  dir == 0
    if (coords[dim] == 0) {
      // For boundary cells assume that the cell on the other side of the
      // boundary is the same as the current cell
      kcoeff[dim][0] = conductivity;
    } else {
      kcoeff[dim][0] = 1 / (meshPtr[dim].deltas[coords[dim]] / (2 * dist[dim][0] * conductivity) +
                            meshPtr[dim].deltas[coords[dim] - 1] /
                                (2 * dist[dim][0] * cell_v[index - stepsize[dim]]->conductivity));
    }

    //  dir == 1
    if (coords[dim] == meshPtr[dim].centers.size() - 1) {
      kcoeff[dim][1] = conductivity;
    } else {
      kcoeff[dim][1] = 1 / (meshPtr[dim].deltas[coords[dim]] / (2 * dist[dim][1] * conductivity) +
                            meshPtr[dim].deltas[coords[dim] + 1] /
                                (2 * dist[dim][1] * cell_v[index + stepsize[dim]]->conductivity));
    }
  }
}

void Cell::setPDEcoefficients(int ndims, bool cylindrical) {

  for (auto dim : dims) {
    if (dim < 5) {
      pde[dim][1] = onePDEcoefficient(dim, 1);
      pde[dim][0] = onePDEcoefficient(dim, 0);
    }
  }

  // Radial X terms
  if (ndims == 2 && cylindrical) {
    pde_c[1] = (dist[0][0] * kcoeff[0][1]) / ((dist[0][0] + dist[0][1]) * dist[0][1]);
    pde_c[0] = (dist[0][1] * kcoeff[0][0]) / ((dist[0][0] + dist[0][1]) * dist[0][0]);
  }
}

double Cell::onePDEcoefficient(std::size_t dim, std::size_t dir) {
  int sign = dir == 0 ? -1 : 1;
  double c = sign * (2 * kcoeff[dim][dir]) / ((dist[dim][0] + dist[dim][1]) * dist[dim][dir]);
  return c;
}

void Cell::setComputeDims(std::size_t (&in_dims)[3]) {
  for (std::size_t d = 0; d < 3; ++d) {
    dims[d] = in_dims[d];
  }
}

void Cell::setZeroThicknessCellProperties(std::vector<std::shared_ptr<Cell>> pointSet) {
  std::vector<double> volumes;
  std::vector<double> densities;
  std::vector<double> specificHeats;
  std::vector<double> conductivities;

  std::vector<double> masses;
  std::vector<double> capacities;
  std::vector<double> weightedConductivity;

  for (const auto &p_cell : pointSet) {
    // Do not add air cell properties into the weighted average
    if (p_cell->cellType != CellType::INTERIOR_AIR && p_cell->cellType != CellType::EXTERIOR_AIR) {
      double vol = p_cell->volume;
      double rho = p_cell->density;
      double cp = p_cell->specificHeat;
      double kth = p_cell->conductivity;

      volumes.push_back(vol);
      masses.push_back(vol * rho);
      capacities.push_back(vol * rho * cp);
      weightedConductivity.push_back(vol * kth);
    }
  }

  // if the neighboring cells are all air cells set properties to air properties
  if (volumes.size() == 0) {
    volumes.push_back(1.0);
    masses.push_back(1.275);
    capacities.push_back(1.275 * 1007);
    weightedConductivity.push_back(0.02587);
  }

  double totalVolume = std::accumulate(volumes.begin(), volumes.end(), 0.0);

  density = std::accumulate(masses.begin(), masses.end(), 0.0) / totalVolume;

  specificHeat =
      std::accumulate(capacities.begin(), capacities.end(), 0.0) / (totalVolume * density);

  conductivity =
      std::accumulate(weightedConductivity.begin(), weightedConductivity.end(), 0.0) / totalVolume;
}

void Cell::calcCellADEUp(double timestep, const Foundation &foundation,
                         const BoundaryConditions & /*bcs*/, double &U) {
  double theta = timestep * iHeatCapacity;

  double C[3][2]{{0}};
  gatherCCoeffs(theta, foundation.coordinateSystem == Foundation::CS_CYLINDRICAL, C);

  double bit{1}, divisor{1};
  U = heatGain * theta;
  for (auto dim : dims) {
    if (dim < 5) {
      bit -= C[dim][1];
      divisor -= C[dim][0];
      U += *(told_ptr + stepsize[dim]) * C[dim][1] - *(&U - stepsize[dim]) * C[dim][0];
    }
  }
  U = (*told_ptr * bit + U) / divisor;
}

void Cell::calcCellADEDown(double timestep, const Foundation &foundation,
                           const BoundaryConditions & /*bcs*/, double &V) {
  double theta = timestep * iHeatCapacity;

  double C[3][2]{{0}};
  gatherCCoeffs(theta, foundation.coordinateSystem == Foundation::CS_CYLINDRICAL, C);

  double bit{1}, divisor{1};
  V = heatGain * theta;
  for (auto dim : dims) {
    if (dim < 5) {
      bit += C[dim][0];
      divisor += C[dim][1];
      V += *(&V + stepsize[dim]) * C[dim][1] - *(told_ptr - stepsize[dim]) * C[dim][0];
    }
  }
  V = (*told_ptr * bit + V) / divisor;
}

double Cell::calcCellExplicit(double timestep, const Foundation &foundation,
                              const BoundaryConditions & /*bcs*/) {
  double theta = timestep * iHeatCapacity;

  double C[3][2]{{0}};
  gatherCCoeffs(theta, foundation.coordinateSystem == Foundation::CS_CYLINDRICAL, C);

  double bit{1};
  double TNew = heatGain * theta;
  for (auto dim : dims) {
    if (dim < 5) {
      bit += C[dim][0] - C[dim][1];
      TNew += *(told_ptr + stepsize[dim]) * C[dim][1] - *(told_ptr - stepsize[dim]) * C[dim][0];
    }
  }
  TNew += *told_ptr * bit;
  return TNew;
}

void Cell::calcCellMatrix(Foundation::NumericalScheme scheme, const double timestep,
                          const Foundation &foundation, const BoundaryConditions & /*bcs*/,
                          double &A, double (&Alt)[3][2], double &bVal) {
  if (scheme == Foundation::NS_STEADY_STATE) {
    calcCellSteadyState(foundation, A, Alt, bVal);
  } else {
    double theta = timestep * iHeatCapacity;

    double f = scheme == Foundation::NS_IMPLICIT ? 1.0 : 0.5;

    bool cylindrical = (foundation.coordinateSystem == Foundation::CS_CYLINDRICAL);
    double C[3][2]{{0}};
    gatherCCoeffs(theta, cylindrical, C);

    double bit{0};
    bVal = heatGain * theta;
    for (auto dim : dims) {
      if (dim < 5) {
        bit += C[dim][1] - C[dim][0];
        Alt[dim][1] = -f * C[dim][1];
        Alt[dim][0] = f * C[dim][0];
        bVal += *(told_ptr + stepsize[dim]) * (1 - f) * C[dim][1] -
                *(told_ptr - stepsize[dim]) * (1 - f) * C[dim][0];
      }
    }
    A = (1.0 + f * bit);
    bVal += *told_ptr * (1.0 - (1 - f) * bit);
  }
}

void Cell::calcCellSteadyState(const Foundation &foundation, double &A, double (&Alt)[3][2],
                               double &bVal) {

  A = 0;
  for (auto dim : dims) {
    if (dim < 5) {
      Alt[dim][1] = pde[dim][1];
      Alt[dim][0] = -pde[dim][0];
      A += Alt[dim][1] + Alt[dim][0];
    }
  }
  if (foundation.coordinateSystem == Foundation::CS_CYLINDRICAL && coords[0] != 0) {
    Alt[0][1] += pde_c[1] / r;
    Alt[0][0] += -pde_c[0] / r;
    A += pde_c[1] / r - pde_c[0] / r;
  }
  A *= -1;
  bVal = -heatGain;
}

void Cell::calcCellADI(std::size_t dim, const double timestep, const Foundation &foundation,
                       const BoundaryConditions & /*bcs*/, double &A, double (&Alt)[2],
                       double &bVal) {
  double theta = timestep * iHeatCapacityADI;

  double Q = heatGain * theta;

  if (foundation.numberOfDimensions == 1) {
    A = 1.0 + (pde[2][1] - pde[2][0]) * theta;
    Alt[0] = pde[2][0] * theta;
    Alt[1] = -pde[2][1] * theta;

    bVal = *told_ptr + Q;
    return;
  }

  double f = foundation.fADI;
  double multiplier = foundation.numberOfDimensions == 2 ? (2.0 - f) : (3.0 - 2.0 * f);
  double C[3][2]{{0}};
  gatherCCoeffs(theta, foundation.coordinateSystem == Foundation::CS_CYLINDRICAL, C);

  ADImath(dim, Q, f, multiplier, C, A, Alt, bVal);
}

void Cell::ADImath(std::size_t dim, const double Q, const double f, const double multiplier,
                   const double (&C)[3][2], double &A, double (&Alt)[2], double &bVal) {
  bVal = Q;
  double bit{0};
  for (auto sdim : dims) {
    if (sdim == dim) {
      Alt[0] = multiplier * C[sdim][0];
      Alt[1] = -multiplier * C[sdim][1];
      A = 1.0 - (Alt[0] + Alt[1]);
    } else if (sdim < 5) {
      bit += C[sdim][0] - C[sdim][1];
      bVal += *(told_ptr + stepsize[sdim]) * f * C[sdim][1] -
              *(told_ptr - stepsize[sdim]) * f * C[sdim][0];
    }
  }
  bVal += *told_ptr * (1.0 + f * bit);
}

void Cell::gatherCCoeffs(const double theta, bool cylindrical, double (&C)[3][2]) {
  for (auto dim : dims) {
    if (dim < 5) {
      C[dim][0] = pde[dim][0] * theta;
      C[dim][1] = pde[dim][1] * theta;
    }
  }
  if (cylindrical && coords[0] != 0) {
    C[0][0] += pde_c[0] * theta / r;
    C[0][1] += pde_c[1] * theta / r;
  }
}

std::array<double, 3>
Cell::calculateHeatFlux(int ndims, double &TNew, std::size_t nX, std::size_t nY, std::size_t nZ,
                        const std::vector<std::shared_ptr<Cell>> & /*cell_v*/) {
  double CXP = 0;
  double CXM = 0;
  double CYP = 0;
  double CYM = 0;
  double CZP = -kcoeff[2][1] * dist[2][0] / (dist[2][1] + dist[2][0]) / dist[2][1];
  double CZM = -kcoeff[2][0] * dist[2][1] / (dist[2][1] + dist[2][0]) / dist[2][0];

  if (ndims > 1) {
    CXP = -kcoeff[0][1] * dist[0][0] / (dist[0][1] + dist[0][0]) / dist[0][1];
    CXM = -kcoeff[0][0] * dist[0][1] / (dist[0][1] + dist[0][0]) / dist[0][0];
  }

  if (ndims == 3) {
    CYP = -kcoeff[1][1] * dist[1][0] / (dist[1][1] + dist[1][0]) / dist[1][1];
    CYM = -kcoeff[1][0] * dist[1][1] / (dist[1][1] + dist[1][0]) / dist[1][0];
  }

  double DTXP = 0;
  double DTXM = 0;
  double DTYP = 0;
  double DTYM = 0;
  double DTZP = 0;
  double DTZM = 0;

  if (coords[0] != nX - 1)
    DTXP = *(&TNew + stepsize[0]) - TNew;

  if (coords[0] != 0)
    DTXM = TNew - *(&TNew - stepsize[0]);

  if (coords[1] != nY - 1)
    DTYP = *(&TNew + stepsize[1]) - TNew;

  if (coords[1] != 0)
    DTYM = TNew - *(&TNew - stepsize[1]);

  if (coords[2] != nZ - 1)
    DTZP = *(&TNew + stepsize[2]) - TNew;

  if (coords[2] != 0)
    DTZM = TNew - *(&TNew - stepsize[2]);

  double Qx = CXP * DTXP + CXM * DTXM;
  double Qy = CYP * DTYP + CYM * DTYM;
  double Qz = CZP * DTZP + CZM * DTZM;

  return {Qx, Qy, Qz};
}

void Cell::doOutdoorTemp(const BoundaryConditions &bcs, double &A, double &bVal) {
  A = 1.0;
  bVal = bcs.outdoorTemp;
}

void Cell::doIndoorTemp(const BoundaryConditions &bcs, double &A, double &bVal) {
  A = 1.0;
  bVal = bcs.slabConvectiveTemp;
}

ExteriorAirCell::ExteriorAirCell(const std::size_t &index, const CellType cellType,
                                 const std::size_t &i, const std::size_t &j, const std::size_t &k,
                                 std::size_t *stepsize, const Foundation &foundation,
                                 Surface *surfacePtr, Block *blockPtr, Mesher *meshPtr)
    : Cell(index, cellType, i, j, k, stepsize, foundation, surfacePtr, blockPtr, meshPtr) {}

void ExteriorAirCell::calcCellADEUp(double /*timestep*/, const Foundation & /*foundation*/,
                                    const BoundaryConditions &bcs, double &U) {
  U = bcs.outdoorTemp;
}

void ExteriorAirCell::calcCellADEDown(double /*timestep*/, const Foundation & /*foundation*/,
                                      const BoundaryConditions &bcs, double &V) {
  V = bcs.outdoorTemp;
}

double ExteriorAirCell::calcCellExplicit(double /*timestep*/, const Foundation & /*foundation*/,
                                         const BoundaryConditions &bcs) {
  return bcs.outdoorTemp;
}

void ExteriorAirCell::calcCellADI(std::size_t /*dim*/, const double /*timestep*/,
                                  const Foundation &, const BoundaryConditions &bcs, double &A,
                                  double (&)[2], double &bVal) {
  doOutdoorTemp(bcs, A, bVal);
}

void ExteriorAirCell::calcCellMatrix(Foundation::NumericalScheme, const double /*timestep*/,
                                     const Foundation &, const BoundaryConditions &bcs, double &A,
                                     double (&)[3][2], double &bVal) {
  doOutdoorTemp(bcs, A, bVal);
}

std::array<double, 3>
ExteriorAirCell::calculateHeatFlux(int /*ndims*/, double & /*TNew*/, std::size_t /*nX*/,
                                   std::size_t /*nY*/, std::size_t /*nZ*/,
                                   const std::vector<std::shared_ptr<Cell>> & /*cell_v*/) {
  return {0.0, 0.0, 0.0};
}

InteriorAirCell::InteriorAirCell(const std::size_t &index, const CellType cellType,
                                 const std::size_t &i, const std::size_t &j, const std::size_t &k,
                                 std::size_t *stepsize, const Foundation &foundation,
                                 Surface *surfacePtr, Block *blockPtr, Mesher *meshPtr)
    : Cell(index, cellType, i, j, k, stepsize, foundation, surfacePtr, blockPtr, meshPtr) {}

void InteriorAirCell::calcCellADEUp(double /*timestep*/, const Foundation & /*foundation*/,
                                    const BoundaryConditions &bcs, double &U) {
  U = bcs.slabConvectiveTemp;
}

void InteriorAirCell::calcCellADEDown(double /*timestep*/, const Foundation & /*foundation*/,
                                      const BoundaryConditions &bcs, double &V) {
  V = bcs.slabConvectiveTemp;
}

double InteriorAirCell::calcCellExplicit(double /*timestep*/, const Foundation & /*foundation*/,
                                         const BoundaryConditions &bcs) {
  return bcs.slabConvectiveTemp;
}

void InteriorAirCell::calcCellMatrix(Foundation::NumericalScheme, const double /*timestep*/,
                                     const Foundation &, const BoundaryConditions &bcs, double &A,
                                     double (&)[3][2], double &bVal) {
  doIndoorTemp(bcs, A, bVal);
}

void InteriorAirCell::calcCellADI(std::size_t /*dim*/, const double /*timestep*/,
                                  const Foundation &, const BoundaryConditions &bcs, double &A,
                                  double (&)[2], double &bVal) {
  doIndoorTemp(bcs, A, bVal);
}

std::array<double, 3>
InteriorAirCell::calculateHeatFlux(int /*ndims*/, double & /*TNew*/, std::size_t /*nX*/,
                                   std::size_t /*nY*/, std::size_t /*nZ*/,
                                   const std::vector<std::shared_ptr<Cell>> & /*cell_v*/) {
  return {0.0, 0.0, 0.0};
}

BoundaryCell::BoundaryCell(const std::size_t &index, const CellType cellType, const std::size_t &i,
                           const std::size_t &j, const std::size_t &k, std::size_t *stepsize,
                           const Foundation &foundation, Surface *surfacePtr, Block *blockPtr,
                           Mesher *meshPtr)
    : Cell(index, cellType, i, j, k, stepsize, foundation, surfacePtr, blockPtr, meshPtr) {
  if (foundation.numberOfDimensions == 2 &&
      foundation.coordinateSystem == Foundation::CS_CYLINDRICAL) {
    if (surfacePtr->orientation == Surface::X_POS || surfacePtr->orientation == Surface::X_NEG) {
      area = 2.0 * PI * meshPtr[0].centers[coords[0]] * meshPtr[2].deltas[coords[2]];
    } else // if (surface.orientation == Surface::Z_POS ||
           // surface.orientation == Surface::Z_NEG)
    {
      area = PI * (meshPtr[0].dividers[coords[0] + 1] * meshPtr[0].dividers[coords[0] + 1] -
                   meshPtr[0].dividers[coords[0]] * meshPtr[0].dividers[coords[0]]);
    }
  } else if (foundation.numberOfDimensions == 2 &&
             foundation.coordinateSystem == Foundation::CS_CARTESIAN) {
    if (surfacePtr->orientation == Surface::X_POS || surfacePtr->orientation == Surface::X_NEG) {
      area = 2.0 * meshPtr[2].deltas[coords[2]] * foundation.linearAreaMultiplier;
    } else // if (surface.orientation == Surface::Z_POS ||
           // surface.orientation == Surface::Z_NEG)
    {
      area = 2.0 * meshPtr[0].deltas[coords[0]] * foundation.linearAreaMultiplier;
    }
  } else if (foundation.numberOfDimensions == 3) {
    if (surfacePtr->orientation == Surface::X_POS || surfacePtr->orientation == Surface::X_NEG) {
      area = meshPtr[1].deltas[coords[1]] * meshPtr[2].deltas[coords[2]];
    } else if (surfacePtr->orientation == Surface::Y_POS ||
               surfacePtr->orientation == Surface::Y_NEG) {
      area = meshPtr[0].deltas[coords[0]] * meshPtr[2].deltas[coords[2]];
    } else // if (surface.orientation == Surface::Z_POS ||
           // surface.orientation == Surface::Z_NEG)
    {
      area = meshPtr[0].deltas[coords[0]] * meshPtr[1].deltas[coords[1]];
    }

    if (foundation.useSymmetry) {
      if (foundation.isXSymm)
        area = 2 * area;

      if (foundation.isYSymm)
        area = 2 * area;
    }
  } else /* if (foundation.numberOfDimensions == 1) */
  {
    area = 1.0;
  }
}

void BoundaryCell::calcCellADEUp(double /*timestep*/, const Foundation & /*foundation*/,
                                 const BoundaryConditions &bcs, double &U) {
  std::size_t dim = surfacePtr->orientation_dim;
  std::size_t dir = surfacePtr->orientation_dir;

  switch (surfacePtr->boundaryConditionType) {
  case Surface::ZERO_FLUX:
    zfCellADEUp(dim, dir, U);
    break;
  case Surface::CONSTANT_TEMPERATURE:
    U = surfacePtr->temperature;
    break;
  case Surface::INTERIOR_TEMPERATURE:
    U = bcs.slabConvectiveTemp;
    break;
  case Surface::EXTERIOR_TEMPERATURE:
    U = bcs.outdoorTemp;
    break;
  case Surface::INTERIOR_FLUX:
    ifCellADEUp(dim, dir, U);
    break;
  case Surface::EXTERIOR_FLUX:
    efCellADEUp(dim, dir, U);
    break;
  }
}

void BoundaryCell::calcCellADEDown(double /*timestep*/, const Foundation & /*foundation*/,
                                   const BoundaryConditions &bcs, double &V) {
  std::size_t dim = surfacePtr->orientation_dim;
  std::size_t dir = surfacePtr->orientation_dir;

  switch (surfacePtr->boundaryConditionType) {
  case Surface::ZERO_FLUX:
    zfCellADEDown(dim, dir, V);
    break;
  case Surface::CONSTANT_TEMPERATURE:
    V = surfacePtr->temperature;
    break;
  case Surface::INTERIOR_TEMPERATURE:
    V = bcs.slabConvectiveTemp;
    break;
  case Surface::EXTERIOR_TEMPERATURE:
    V = bcs.outdoorTemp;
    break;
  case Surface::INTERIOR_FLUX:
    ifCellADEDown(dim, dir, V);
    break;
  case Surface::EXTERIOR_FLUX:
    efCellADEDown(dim, dir, V);
    break;
  }
}

double BoundaryCell::calcCellExplicit(double /*timestep*/, const Foundation & /*foundation*/,
                                      const BoundaryConditions &bcs) {
  std::size_t dim = surfacePtr->orientation_dim;
  std::size_t dir = surfacePtr->orientation_dir;

  switch (surfacePtr->boundaryConditionType) {
  case Surface::ZERO_FLUX:
    return zfCellExplicit(dim, dir);
  case Surface::CONSTANT_TEMPERATURE:
    return surfacePtr->temperature;
  case Surface::INTERIOR_TEMPERATURE:
    return bcs.slabConvectiveTemp;
  case Surface::EXTERIOR_TEMPERATURE:
    return bcs.outdoorTemp;
  case Surface::INTERIOR_FLUX:
    return ifCellExplicit(dim, dir);
  default: // case Surface::EXTERIOR_FLUX:
    return efCellExplicit(dim, dir);
  }
}

void BoundaryCell::calcCellADI(std::size_t dim, const double /*timestep*/,
                               const Foundation & /*foundation*/, const BoundaryConditions &bcs,
                               double &A, double (&Alt)[2], double &bVal) {
  std::size_t sdim = surfacePtr->orientation_dim;
  std::size_t dir = surfacePtr->orientation_dir;

  switch (surfacePtr->boundaryConditionType) {
  case Surface::ZERO_FLUX:
    zfCellADI(dim, sdim, dir, A, Alt[dir], bVal);
    break;
  case Surface::CONSTANT_TEMPERATURE:
    A = 1.0;
    bVal = surfacePtr->temperature;
    break;
  case Surface::INTERIOR_TEMPERATURE:
    doIndoorTemp(bcs, A, bVal);
    break;
  case Surface::EXTERIOR_TEMPERATURE:
    doOutdoorTemp(bcs, A, bVal);
    break;
  case Surface::INTERIOR_FLUX:
    ifCellADI(dim, sdim, dir, A, Alt[dir], bVal);
    break;
  case Surface::EXTERIOR_FLUX:
    efCellADI(dim, sdim, dir, A, Alt[dir], bVal);
    break;
  }
}

void BoundaryCell::calcCellMatrix(Foundation::NumericalScheme, const double /*timestep*/,
                                  const Foundation & /*foundation*/, const BoundaryConditions &bcs,
                                  double &A, double (&Alt)[3][2], double &bVal) {
  std::size_t dim = surfacePtr->orientation_dim;
  std::size_t dir = surfacePtr->orientation_dir;

  switch (surfacePtr->boundaryConditionType) {
  case Surface::ZERO_FLUX: {
    zfCellMatrix(A, Alt[dim][dir], bVal);
    break;
  }
  case Surface::CONSTANT_TEMPERATURE: {
    A = 1.0;
    bVal = surfacePtr->temperature;
    break;
  }
  case Surface::INTERIOR_TEMPERATURE: {
    doIndoorTemp(bcs, A, bVal);
    break;
  }
  case Surface::EXTERIOR_TEMPERATURE: {
    doOutdoorTemp(bcs, A, bVal);
    break;
  }
  case Surface::INTERIOR_FLUX: {
    ifCellMatrix(dim, dir, A, Alt[dim][dir], bVal);
    break;
  }
  case Surface::EXTERIOR_FLUX: {
    efCellMatrix(dim, dir, A, Alt[dim][dir], bVal);
    break;
  }
  }
}

std::array<double, 3>
BoundaryCell::calculateHeatFlux(int ndims, double &TNew, std::size_t nX, std::size_t nY,
                                std::size_t nZ,
                                const std::vector<std::shared_ptr<Cell>> & /*cell_v*/) {

  double CXP = 0;
  double CXM = 0;
  double CYP = 0;
  double CYM = 0;
  double CZP = -kcoeff[2][1] * dist[2][0] / (dist[2][1] + dist[2][0]) / dist[2][1];
  double CZM = -kcoeff[2][0] * dist[2][1] / (dist[2][1] + dist[2][0]) / dist[2][0];

  if (ndims > 1) {
    CXP = -kcoeff[0][1] * dist[0][0] / (dist[0][1] + dist[0][0]) / dist[0][1];
    CXM = -kcoeff[0][0] * dist[0][1] / (dist[0][1] + dist[0][0]) / dist[0][0];
  }

  if (ndims == 3) {
    CYP = -kcoeff[1][1] * dist[1][0] / (dist[1][1] + dist[1][0]) / dist[1][1];
    CYM = -kcoeff[1][0] * dist[1][1] / (dist[1][1] + dist[1][0]) / dist[1][0];
  }

  double DTXP = 0;
  double DTXM = 0;
  double DTYP = 0;
  double DTYM = 0;
  double DTZP = 0;
  double DTZM = 0;

  if (coords[0] != nX - 1)
    DTXP = *(&TNew + stepsize[0]) - TNew;

  if (coords[0] != 0)
    DTXM = TNew - *(&TNew - stepsize[0]);

  if (coords[1] != nY - 1)
    DTYP = *(&TNew + stepsize[1]) - TNew;

  if (coords[1] != 0)
    DTYM = TNew - *(&TNew - stepsize[1]);

  if (coords[2] != nZ - 1)
    DTZP = *(&TNew + stepsize[2]) - TNew;

  if (coords[2] != 0)
    DTZM = TNew - *(&TNew - stepsize[2]);

  switch (surfacePtr->orientation) {
  case Surface::X_NEG: {
    CXP = -kcoeff[0][1] / dist[0][1];
    CXM = 0;
  } break;
  case Surface::X_POS: {
    CXP = 0;
    CXM = -kcoeff[0][0] / dist[0][0];
  } break;
  case Surface::Y_NEG: {
    CYP = -kcoeff[1][1] / dist[1][1];
    CYM = 0;
  } break;
  case Surface::Y_POS: {
    CYP = 0;
    CYM = -kcoeff[1][0] / dist[1][0];
  } break;
  case Surface::Z_NEG: {
    CZP = -kcoeff[2][1] / dist[2][1];
    CZM = 0;
  } break;
  case Surface::Z_POS: {
    CZP = 0;
    CZM = -kcoeff[2][0] / dist[2][0];
  } break;
  }
  double Qx = CXP * DTXP + CXM * DTXM;
  double Qy = CYP * DTYP + CYM * DTYM;
  double Qz = CZP * DTZP + CZM * DTZM;

  return {Qx, Qy, Qz};
}

#define INTFLUX_PREFACE                                                                            \
  double Tair = surfacePtr->temperature;                                                           \
  double Trad = surfacePtr->radiantTemperature;                                                    \
  double cosTilt = surfacePtr->cosTilt;                                                            \
  double hc = surfacePtr->convectionAlgorithm(*told_ptr, Tair, surfacePtr->hfTerm,                 \
                                              surfacePtr->propPtr->roughness, cosTilt);            \
  double hr = getSimpleInteriorIRCoeff(surfacePtr->propPtr->emissivity, *told_ptr, Trad);

#define EXTFLUX_PREFACE                                                                            \
  double Tair = surfacePtr->temperature;                                                           \
  double cosTilt = surfacePtr->cosTilt;                                                            \
  double Fqtr = surfacePtr->effectiveLWViewFactorQtr;                                              \
  double hc = surfacePtr->convectionAlgorithm(*told_ptr, Tair, surfacePtr->hfTerm,                 \
                                              surfacePtr->propPtr->roughness, cosTilt);            \
  double hr = getExteriorIRCoeff(surfacePtr->propPtr->emissivity, *told_ptr, Tair, Fqtr);

void BoundaryCell::zfCellADI(const std::size_t dim, const std::size_t sdim, const std::size_t sign,
                             double &A, double &Alt, double &bVal) {
  A = 1.0;
  if (dim == sdim) {
    Alt = -1.0;
    bVal = 0;
  } else {
    Alt = 0.0;
    bVal = *(told_ptr + sign * stepsize[sdim]);
  }
}

void BoundaryCell::ifCellADI(const std::size_t dim, const std::size_t sdim, const std::size_t dir,
                             double &A, double &Alt, double &bVal) {
  INTFLUX_PREFACE

  int sign = (dir == 0) ? -1 : 1;

  A = kcoeff[sdim][dir] / dist[sdim][dir] + (hc + hr);
  if (dim == sdim) {
    Alt = -kcoeff[sdim][dir] / dist[sdim][dir];
    bVal = hc * Tair + hr * Trad + heatGain;
  } else {
    Alt = 0.0;
    bVal = *(told_ptr + sign * stepsize[sdim]) * kcoeff[sdim][dir] / dist[sdim][dir] + hc * Tair +
           hr * Trad + heatGain;
  }
}

void BoundaryCell::efCellADI(const std::size_t dim, const std::size_t sdim, const std::size_t dir,
                             double &A, double &Alt, double &bVal) {
  EXTFLUX_PREFACE

  int sign = (dir == 0) ? -1 : 1;

  A = kcoeff[sdim][dir] / dist[sdim][dir] + (hc + hr);
  if (dim == sdim) {
    Alt = -kcoeff[sdim][dir] / dist[sdim][dir];
    bVal = (hc + hr * Fqtr) * Tair + heatGain;
  } else {
    Alt = 0.0;
    bVal = *(told_ptr + sign * stepsize[sdim]) * kcoeff[sdim][dir] / dist[sdim][dir] +
           (hc + hr * Fqtr) * Tair + heatGain;
  }
}

void BoundaryCell::zfCellMatrix(double &A, double &Alt, double &bVal) {
  A = 1.0;
  Alt = -1.0;
  bVal = 0.0;
}

void BoundaryCell::ifCellMatrix(const std::size_t dim, const std::size_t dir, double &A,
                                double &Alt, double &bVal) {
  INTFLUX_PREFACE

  A = kcoeff[dim][dir] / dist[dim][dir] + (hc + hr);
  Alt = -kcoeff[dim][dir] / dist[dim][dir];
  bVal = hc * Tair + hr * Trad + heatGain;
}

void BoundaryCell::efCellMatrix(const std::size_t dim, const std::size_t dir, double &A,
                                double &Alt, double &bVal) {
  EXTFLUX_PREFACE

  A = kcoeff[dim][dir] / dist[dim][dir] + (hc + hr);
  Alt = -kcoeff[dim][dir] / dist[dim][dir];
  bVal = (hc + hr * Fqtr) * Tair + heatGain;
}

void BoundaryCell::zfCellADEUp(const std::size_t dim, const std::size_t &dir, double &U) {
  if (dir == 1) {
    U = *(told_ptr + stepsize[dim]);
  } else /* if (dir == 0) */ {
    U = *(&U - stepsize[dim]);
  }
}

void BoundaryCell::ifCellADEUp(const std::size_t dim, const std::size_t dir, double &U) {
  INTFLUX_PREFACE

  double bit;
  if (dir == 1) {
    bit = *(told_ptr + stepsize[dim]);
  } else /*if (dir == 0)*/ {
    bit = *(&U - stepsize[dim]);
  }
  U = (kcoeff[dim][dir] * bit / dist[dim][dir] + hc * Tair + hr * Trad + heatGain) /
      (kcoeff[dim][dir] / dist[dim][dir] + (hc + hr));
}

void BoundaryCell::efCellADEUp(const std::size_t dim, const std::size_t dir, double &U) {
  EXTFLUX_PREFACE

  double bit;
  if (dir == 1) {
    bit = *(told_ptr + stepsize[dim]);
  } else /*if (dir == 0)*/ {
    bit = *(&U - stepsize[dim]);
  }
  U = (kcoeff[dim][dir] * bit / dist[dim][dir] + (hc + hr * Fqtr) * Tair + heatGain) /
      (kcoeff[dim][dir] / dist[dim][dir] + (hc + hr));
}

void BoundaryCell::zfCellADEDown(const std::size_t dim, const std::size_t &dir, double &V) {
  if (dir == 1) {
    V = *(&V + stepsize[dim]);
  } else /* if (dir == 0) */ {
    V = *(told_ptr - stepsize[dim]);
  }
}

void BoundaryCell::ifCellADEDown(const std::size_t dim, const std::size_t dir, double &V) {
  INTFLUX_PREFACE

  double bit;
  if (dir == 1) {
    bit = *(&V + stepsize[dim]);
  } else /*if (dir == 0)*/ {
    bit = *(told_ptr - stepsize[dim]);
  }
  V = (kcoeff[dim][dir] * bit / dist[dim][dir] + hc * Tair + hr * Trad + heatGain) /
      (kcoeff[dim][dir] / dist[dim][dir] + (hc + hr));
}

void BoundaryCell::efCellADEDown(const std::size_t dim, const std::size_t dir, double &V) {
  EXTFLUX_PREFACE

  double bit;
  if (dir == 1) {
    bit = *(&V + stepsize[dim]);
  } else /*if (dir == 0)*/ {
    bit = *(told_ptr - stepsize[dim]);
  }
  V = (kcoeff[dim][dir] * bit / dist[dim][dir] + (hc + hr * Fqtr) * Tair + heatGain) /
      (kcoeff[dim][dir] / dist[dim][dir] + (hc + hr));
}

double BoundaryCell::zfCellExplicit(const std::size_t dim, const std::size_t &dir) {
  int sign = (dir == 0) ? -1 : 1;
  return *(told_ptr + sign * stepsize[dim]);
}

double BoundaryCell::ifCellExplicit(const std::size_t dim, const std::size_t &dir) {
  INTFLUX_PREFACE

  int sign = (dir == 0) ? -1 : 1;

  return (kcoeff[dim][dir] * *(told_ptr + sign * stepsize[dim]) / dist[dim][dir] + hc * Tair +
          hr * Trad + heatGain) /
         (kcoeff[dim][dir] / dist[dim][dir] + (hc + hr));
}

double BoundaryCell::efCellExplicit(const std::size_t dim, const std::size_t &dir) {
  EXTFLUX_PREFACE

  return (kcoeff[dim][dir] * *(told_ptr + stepsize[dim]) / dist[dim][dir] +
          (hc + hr * Fqtr) * Tair + heatGain) /
         (kcoeff[dim][dir] / dist[dim][dir] + (hc + hr));
}

ZeroThicknessCell::ZeroThicknessCell(const std::size_t &index, const CellType cellType,
                                     const std::size_t &i, const std::size_t &j,
                                     const std::size_t &k, std::size_t *stepsize,
                                     const Foundation &foundation, Surface *surfacePtr,
                                     Block *blockPtr, Mesher *meshPtr)
    : Cell(index, cellType, i, j, k, stepsize, foundation, surfacePtr, blockPtr, meshPtr) {}

std::array<double, 3>
ZeroThicknessCell::calculateHeatFlux(int ndims, double &TNew, std::size_t nX, std::size_t nY,
                                     std::size_t nZ,
                                     const std::vector<std::shared_ptr<Cell>> &cell_v) {
  std::array<double, 3> Qm;
  std::array<double, 3> Qp;

  if (isEqual(meshPtr[0].deltas[coords[0]], 0.0)) {
    Qm = cell_v[index - stepsize[0]]->calculateHeatFlux(ndims, *(&TNew - stepsize[0]), nX, nY, nZ,
                                                        cell_v);
    Qp = cell_v[index + stepsize[0]]->calculateHeatFlux(ndims, *(&TNew + stepsize[0]), nX, nY, nZ,
                                                        cell_v);
  }
  if (isEqual(meshPtr[1].deltas[coords[1]], 0.0)) {
    Qm = cell_v[index - stepsize[1]]->calculateHeatFlux(ndims, *(&TNew - stepsize[1]), nX, nY, nZ,
                                                        cell_v);
    Qp = cell_v[index + stepsize[1]]->calculateHeatFlux(ndims, *(&TNew + stepsize[1]), nX, nY, nZ,
                                                        cell_v);
  }
  if (isEqual(meshPtr[2].deltas[coords[2]], 0.0)) {
    Qm = cell_v[index - stepsize[2]]->calculateHeatFlux(ndims, *(&TNew - stepsize[2]), nX, nY, nZ,
                                                        cell_v);
    Qp = cell_v[index + stepsize[2]]->calculateHeatFlux(ndims, *(&TNew + stepsize[2]), nX, nY, nZ,
                                                        cell_v);
  }

  double Qx = (Qm[0] + Qp[0]) * 0.5;
  double Qy = (Qm[1] + Qp[1]) * 0.5;
  double Qz = (Qm[2] + Qp[2]) * 0.5;

  return {Qx, Qy, Qz};
}

} // namespace Kiva

#endif
