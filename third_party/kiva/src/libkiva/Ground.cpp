/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Ground_CPP
#define Ground_CPP

#undef PRNTSURF

#include "Ground.hpp"
#include "Errors.hpp"
//#include <unsupported/Eigen/SparseExtra>

namespace Kiva {

static const double PI = 4.0 * atan(1.0);

static const bool TDMA = true;

Ground::Ground(Foundation &foundation) : foundation(foundation) {
  pSolver = std::make_shared<
      Eigen::BiCGSTAB<Eigen::SparseMatrix<double>, Eigen::IncompleteLUT<double>>>();
}

Ground::Ground(Foundation &foundation, GroundOutput::OutputMap &outputMap)
    : foundation(foundation), groundOutput(outputMap) {
  pSolver = std::make_shared<
      Eigen::BiCGSTAB<Eigen::SparseMatrix<double>, Eigen::IncompleteLUT<double>>>();
}

Ground::~Ground() {}

void Ground::buildDomain() {
  // Create mesh
  foundation.createMeshData();

  // Build matrices for PDE term coefficients
  domain.setDomain(foundation);

  nX = domain.mesh[0].centers.size();
  nY = domain.mesh[1].centers.size();
  nZ = domain.mesh[2].centers.size();
  num_cells = nX * nY * nZ;

  // Initialize matices
  if (foundation.numericalScheme == Foundation::NS_ADE) {
    U.resize(num_cells);
    V.resize(num_cells);
  }

  if ((foundation.numericalScheme == Foundation::NS_ADI || foundation.numberOfDimensions == 1) &&
      TDMA) {
    a1.resize(num_cells, 0.0);
    a2.resize(num_cells, 0.0);
    a3.resize(num_cells, 0.0);
    b_.resize(num_cells, 0.0);
    x_.resize(num_cells);
  }

  pSolver->setMaxIterations(foundation.maxIterations);
  pSolver->setTolerance(foundation.tolerance);
  tripletList.reserve(num_cells * (1 + 2 * foundation.numberOfDimensions));
  Amat.resize(num_cells, num_cells);
  b.resize(num_cells);
  x.resize(num_cells);
  x.fill(283.15);

  TNew.resize(num_cells);
  TOld.resize(num_cells);

  link_cells_to_temp();
}

void Ground::calculateADE() {
// Solve for new values (Main loop)
#if defined(_OPENMP)
#pragma omp parallel sections num_threads(2)
#endif
  {
#if defined(_OPENMP)
#pragma omp section
#endif
    calculateADEUpwardSweep();
#if defined(_OPENMP)
#pragma omp section
#endif
    calculateADEDownwardSweep();
  }
  for (size_t index = 0; index < num_cells; ++index) {
    TNew[index] = 0.5 * (U[index] + V[index]);

    // Update old values for next timestep
    TOld[index] = TNew[index];
  }
}

void Ground::calculateADEUpwardSweep() {
  // Upward sweep (Solve U Matrix starting from 1, 1)
  for (size_t index = 0; index < num_cells; index++) {
    auto this_cell = domain.cell[index];
    this_cell->calcCellADEUp(timestep, foundation, bcs, U[index]);
  }
}

void Ground::calculateADEDownwardSweep() {
  // Downward sweep (Solve V Matrix starting from I, K)
  for (std::size_t n = num_cells; n > 0; n--) {
    std::size_t index = n - 1;
    auto this_cell = domain.cell[index];
    this_cell->calcCellADEDown(timestep, foundation, bcs, V[index]);
  }
}

void Ground::calculateExplicit() {
  for (std::size_t index = 0; index < num_cells; index++) {
    auto this_cell = domain.cell[index];
    TNew[index] = this_cell->calcCellExplicit(timestep, foundation, bcs);
  }
  TOld.assign(TNew.begin(), TNew.end());
}

void Ground::calculateMatrix(Foundation::NumericalScheme scheme) {
  for (std::size_t index = 0; index < num_cells; index++) {
    auto this_cell = domain.cell[index];
    double A, bVal;
    double Alt[3][2] = {{0}};
    this_cell->calcCellMatrix(scheme, timestep, foundation, bcs, A, Alt, bVal);
    setAmatValue(index, index, A);
    for (std::size_t dim = 0; dim < 3; dim++) {
      if (Alt[dim][0] != 0) {
        setAmatValue(index, this_cell->index - domain.stepsize[dim], Alt[dim][0]);
      }
      if (Alt[dim][1] != 0) {
        setAmatValue(index, this_cell->index + domain.stepsize[dim], Alt[dim][1]);
      }
    }
    setbValue(index, bVal);
  }

  solveLinearSystem();

  // Read solution into temperature matrix
  TNew = getXvalues();
  // Update old values for next timestep
  TOld.assign(TNew.begin(), TNew.end());
  clearAmat();
}

void Ground::calculateADI(int dim) {
  double A, Alt[2], bVal;

  auto dest_index = domain.dest_index_vector[dim].begin();
  auto cell_iter = domain.cell.begin();
  for (; dest_index < domain.dest_index_vector[dim].end(); ++cell_iter, ++dest_index) {
    A = 0.0;
    Alt[0] = 0.0;
    Alt[1] = 0.0;
    bVal = 0.0;
    (*cell_iter)->calcCellADI(dim, timestep, foundation, bcs, A, Alt, bVal);
    setValuesADI(*dest_index, A, Alt, bVal);
  }

  solveLinearSystem();

  std::size_t index{0};
  dest_index = domain.dest_index_vector[dim].begin();
  for (; dest_index < domain.dest_index_vector[dim].end(); ++index, ++dest_index) {
    TNew[index] = x_[*dest_index];
  }

  // Update old values for next timestep
  TOld.assign(TNew.begin(), TNew.end());

  clearAmat();
}

void Ground::calculate(BoundaryConditions &boundaryConditions, double ts) {
  bcs = boundaryConditions;
  timestep = ts;
  // update boundary conditions
  setBoundaryConditions();

  // Calculate Temperatures
  switch (foundation.numericalScheme) {
  case Foundation::NS_ADE:
    calculateADE();
    break;
  case Foundation::NS_EXPLICIT:
    calculateExplicit();
    break;
  case Foundation::NS_ADI: {
    if (foundation.numberOfDimensions > 1)
      calculateADI(0);
    if (foundation.numberOfDimensions == 3)
      calculateADI(1);
    calculateADI(2);
  } break;
  case Foundation::NS_IMPLICIT:
    calculateMatrix(Foundation::NS_IMPLICIT);
    break;
  case Foundation::NS_CRANK_NICOLSON:
    calculateMatrix(Foundation::NS_CRANK_NICOLSON);
    break;
  case Foundation::NS_STEADY_STATE:
    calculateMatrix(Foundation::NS_STEADY_STATE);
    break;
  }
}

void Ground::setAmatValue(const std::size_t i, const std::size_t j, const double val) {
  if ((foundation.numericalScheme == Foundation::NS_ADI || foundation.numberOfDimensions == 1) &&
      TDMA) {
    if (j < i)
      a1[i] = val;
    else if (j == i)
      a2[i] = val;
    else
      a3[i] = val;
  } else {
    tripletList.emplace_back(static_cast<int>(i), static_cast<int>(j), val);
  }
}

void Ground::setbValue(const std::size_t i, const double val) {
  if ((foundation.numericalScheme == Foundation::NS_ADI || foundation.numberOfDimensions == 1) &&
      TDMA) {
    b_[i] = val;
  } else {
    b(i) = val;
  }
}

void Ground::setValuesADI(const std::size_t &index, const double &A, const double (&Alt)[2],
                          const double &bVal) {
  a1[index] = Alt[0];
  a2[index] = A;
  a3[index] = Alt[1];
  b_[index] = bVal;
}

void Ground::solveLinearSystem() {
  if ((foundation.numericalScheme == Foundation::NS_ADI || foundation.numberOfDimensions == 1) &&
      TDMA) {
    solveTDM(a1, a2, a3, b_, x_);
  } else {
    int iters;
    double residual;

    bool success;

    Amat.setFromTriplets(tripletList.begin(), tripletList.end());
    pSolver->compute(Amat);
    x = pSolver->solveWithGuess(b, x);
    int status = pSolver->info();

    //    Eigen::saveMarket(Amat, "Amat.mtx");
    //    Eigen::saveMarketVector(b, "b.mtx");
    success = status == Eigen::Success;
    if (!success) {
      iters = static_cast<int>(pSolver->iterations());
      residual = pSolver->error();

      std::stringstream ss;
      ss << "Solution did not converge after " << iters << " iterations. The final residual was: ("
         << residual << ").";
      showMessage(MSG_ERR, ss.str());
    }
  }
}

void Ground::clearAmat() {
  if ((foundation.numericalScheme == Foundation::NS_ADI || foundation.numberOfDimensions == 1) &&
      TDMA) {
    std::fill(a1.begin(), a1.end(), 0.0);
    std::fill(a2.begin(), a2.end(), 0.0);
    std::fill(a3.begin(), a3.end(), 0.0);
    std::fill(b_.begin(), b_.end(), 0.0);

  } else {
    tripletList.clear();
    tripletList.reserve(nX * nY * nZ * (1 + 2 * foundation.numberOfDimensions));
  }
}

double Ground::getxValue(const int i) {
  if ((foundation.numericalScheme == Foundation::NS_ADI || foundation.numberOfDimensions == 1) &&
      TDMA) {
    return x_[i];
  } else {
    return x(i);
  }
}

std::vector<double> Ground::getXvalues() {
  if ((foundation.numericalScheme == Foundation::NS_ADI || foundation.numberOfDimensions == 1) &&
      TDMA) {
    return x_;
  } else {
    std::vector<double> v(x.data(), x.data() + x.rows());
    return v;
  }
}

double Ground::getSurfaceArea(Surface::SurfaceType surfaceType) {
  double totalArea = 0;

  // Find surface(s)
  for (const auto &surface : foundation.surfaces) {
    if (surface.type == surfaceType) {
      totalArea += surface.area;
    }
  }

  return totalArea;
}

void Ground::calculateSurfaceAverages() {
  for (auto surfaceType : groundOutput.outputMap) {

    double constructionRValue = 0.0;
    double surfaceArea = foundation.surfaceAreas[surfaceType];

    if (surfaceType == Surface::ST_SLAB_CORE) {
      constructionRValue = foundation.slab.totalResistance();
    } else if (surfaceType == Surface::ST_SLAB_PERIM) {
      constructionRValue = foundation.slab.totalResistance();
    } else if (surfaceType == Surface::ST_WALL_INT) {
      constructionRValue = foundation.wall.totalResistance();
    }

    double totalQ = 0.0;
    double totalQc = 0.0;
    // double totalQr = 0.0;
    double TA = 0;
    double hA = 0.0, hcA = 0.0, hrA = 0.0;
    double totalArea = 0.0;
    double TAconv = 0.0;

    if (foundation.hasSurface[surfaceType]) {
      // Find surface(s)
      for (auto &surface : foundation.surfaces) {
        if (surface.type == surfaceType) {
          double Trad = surface.radiantTemperature;
          double Tair = surface.temperature;

#ifdef PRNTSURF
          std::ofstream output;
          output.open("surface.csv");
          output << "x, T, h, q, dx\n";
#endif

          for (auto index : surface.indices) {
            auto this_cell = domain.cell[index];
            double hc = surface.convectionAlgorithm(TNew[index], Tair, surface.hfTerm,
                                                    surface.propPtr->roughness, surface.cosTilt);
            double hr = getSimpleInteriorIRCoeff(surface.propPtr->emissivity, TNew[index], Trad);
            double q = this_cell->heatGain;

            double &A = this_cell->area;

            double Ahc = A * hc;
            double Ahr = A * hr;

            double Qc = Ahc * (Tair - TNew[index]);
            double Qr = Ahr * (Trad - TNew[index]);

            totalArea += A;

            hcA += Ahc;
            hrA += Ahr;
            hA += Ahc + Ahr;

            totalQc += Qc;
            // totalQr += Qr;
            totalQ += Qc + Qr + q * A;

            TA += TNew[index] * A;
            TAconv += Tair * A;

#ifdef PRNTSURF
            output << domain.mesh[0].centers[i] << ", " << TNew[index] << ", " << h << ", "
                   << h * (Tair - TNew[index]) << ", " << domain.mesh[0].deltas[i] << "\n";
#endif
          }

#ifdef PRNTSURF
          output.close();
#endif
        }
      }
    }

    if (totalArea > 0.0) {
      double Tconv = TAconv / totalArea;
      double Tavg = hcA == 0 ? Tconv : Tconv - totalQc / hcA;
      double hcAvg = hcA / totalArea;
      double hrAvg = hrA / totalArea;
      double hAvg = hA / totalArea;

      groundOutput.outputValues[{surfaceType, GroundOutput::OT_TEMP}] = Tavg;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_AVG_TEMP}] = TA / totalArea;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_FLUX}] = totalQ / totalArea;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_RATE}] =
          totalQ / totalArea * surfaceArea;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_CONV}] = hcAvg;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_RAD}] = hrAvg;

      groundOutput.outputValues[{surfaceType, GroundOutput::OT_EFF_TEMP}] =
          Tconv - (totalQ / totalArea) * (constructionRValue + 1 / hAvg) - 273.15;
    } else {
      double Tconv = bcs.slabConvectiveTemp;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_TEMP}] = Tconv;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_AVG_TEMP}] = Tconv;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_FLUX}] = 0.0;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_RATE}] = 0.0;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_CONV}] = 0.0;
      groundOutput.outputValues[{surfaceType, GroundOutput::OT_RAD}] = 0.0;

      groundOutput.outputValues[{surfaceType, GroundOutput::OT_EFF_TEMP}] = Tconv - 273.15;
    }
  }
}

double
Ground::getSurfaceAverageValue(std::pair<Surface::SurfaceType, GroundOutput::OutputType> output) {
  return groundOutput.outputValues[output];
}

void Ground::calculateBoundaryLayer() {
  Foundation fd = foundation;

  BoundaryConditions preBCs;
  preBCs.localWindSpeed = 0;
  preBCs.outdoorTemp = 273.15;
  preBCs.slabConvectiveTemp = preBCs.wallConvectiveTemp = preBCs.slabRadiantTemp =
      preBCs.wallRadiantTemp = 293.15;
  fd.coordinateSystem = Foundation::CS_CARTESIAN;
  fd.numberOfDimensions = 2;
  fd.reductionStrategy = Foundation::RS_AP;
  fd.numericalScheme = Foundation::NS_STEADY_STATE;
  fd.farFieldWidth = 100;

  Ground pre(fd);
  pre.buildDomain();
  pre.calculate(preBCs);

  std::vector<double> x2s;
  std::vector<double> fluxSums;

  double fluxSum = 0.0;

  double x1_0 = 0.0;

  bool firstIndex = true;

  size_t i_min = pre.domain.mesh[0].getNearestIndex(boost::geometry::area(foundation.polygon) /
                                                    boost::geometry::perimeter(foundation.polygon));

  size_t k = pre.domain.mesh[2].getNearestIndex(0.0);

  size_t j = pre.nY / 2;

  for (size_t i = i_min; i < pre.nX; i++) {
    std::size_t index = pre.domain.getIndex(i, j, k);
    double Qz = pre.domain.cell[index]->calculateHeatFlux(pre.foundation.numberOfDimensions,
                                                          pre.TNew[index], pre.nX, pre.nY, pre.nZ,
                                                          pre.domain.cell)[2];
    double x1 = pre.domain.mesh[0].dividers[i];
    double x2 = pre.domain.mesh[0].dividers[i + 1];

    if (Qz > 0.0) {
      fluxSum += std::max(Qz, 0.0) * (x2 - x1);

      if (firstIndex)
        x1_0 = x1;
      x2s.push_back(x2);
      fluxSums.push_back(fluxSum);

      firstIndex = false;
    }
  }

  // std::ofstream output;
  // output.open("Boundary.csv");

  // output << 0.0 << ", " << 0.0 << "\n";

  boundaryLayer.push_back(std::make_pair(0, 0));

  for (std::size_t i = 0; i < fluxSums.size() - 1;
       i++) // last cell is a zero-thickness cell, so don't include it.
  {
    // output << x2s[i] - x1_0 << ", " << fluxSums[i]/fluxSum << "\n";
    boundaryLayer.push_back(std::make_pair(x2s[i] - x1_0, fluxSums[i] / fluxSum));
  }
}

double Ground::getBoundaryValue(double dist) {
  double val = 0.0;
  if (dist > boundaryLayer[boundaryLayer.size() - 1].first)
    val = 1.0;
  else {
    for (std::size_t i = 0; i < boundaryLayer.size() - 1; i++) {
      if (dist >= boundaryLayer[i].first && dist < boundaryLayer[i + 1].first) {
        double m = (boundaryLayer[i + 1].first - boundaryLayer[i].first) /
                   (boundaryLayer[i + 1].second - boundaryLayer[i].second);
        val = (dist - boundaryLayer[i].first) / m + boundaryLayer[i].second;
        continue;
      }
    }
  }
  return val;
}

double Ground::getBoundaryDistance(double val) {
  double dist = 0.0;
  if (val > 1.0 || val < 0.0) {
    showMessage(MSG_ERR, "Boundary value passed not between 0.0 and 1.0.");
  } else {
    for (std::size_t i = 0; i < boundaryLayer.size() - 1; i++) {
      if (val >= boundaryLayer[i].second && val < boundaryLayer[i + 1].second) {
        double m = (boundaryLayer[i + 1].second - boundaryLayer[i].second) /
                   (boundaryLayer[i + 1].first - boundaryLayer[i].first);
        dist = (val - boundaryLayer[i].second) / m + boundaryLayer[i].first;
        continue;
      }
    }
  }
  return dist;
}

void Ground::setNewBoundaryGeometry() {
  double area = boost::geometry::area(foundation.polygon);
  double perimeter = boost::geometry::perimeter(foundation.polygon);
  double interiorPerimeter = 0.0;

  std::size_t nV = foundation.polygon.outer().size();
  for (std::size_t v = 0; v < nV; v++) {
    std::size_t vPrev, vNext, vNext2;

    if (v == 0)
      vPrev = nV - 1;
    else
      vPrev = v - 1;

    if (v == nV - 1)
      vNext = 0;
    else
      vNext = v + 1;

    if (v == nV - 2)
      vNext2 = 0;
    else if (v == nV - 1)
      vNext2 = 1;
    else
      vNext2 = v + 2;

    Point p1 = foundation.polygon.outer()[vPrev];
    Point p2 = foundation.polygon.outer()[v];
    Point p3 = foundation.polygon.outer()[vNext];
    Point p4 = foundation.polygon.outer()[vNext2];

    // Correct U-turns
    if (foundation.isExposedPerimeter[vPrev] && foundation.isExposedPerimeter[v] &&
        foundation.isExposedPerimeter[vNext]) {
      if (isEqual(getAngle(p1, p2, p3) + getAngle(p2, p3, p4), PI)) {
        double d12 = getDistance(p1, p2);
        double d23 = getDistance(p2, p3);
        double d43 = getDistance(p3, p4);
        double edgeDistance = d23;
        double reductionDistance = std::min(d12, d43);
        double reductionValue = 1 - getBoundaryValue(edgeDistance);
        perimeter -= 2 * reductionDistance * reductionValue;
      }
    }

    if (foundation.isExposedPerimeter[vPrev] && foundation.isExposedPerimeter[v]) {
      double alpha = getAngle(p1, p2, p3);
      double d12 = getDistance(p1, p2);
      double d23 = getDistance(p2, p3);

      if (sin(alpha) > 0) {
        double f = getBoundaryDistance(1 - sin(alpha / 2) / (1 + cos(alpha / 2))) / sin(alpha / 2);

        // Chamfer
        double d = f / cos(alpha / 2);
        if (d12 < d || d23 < d) {
          d12 = std::min(d12, d23);
          d23 = std::min(d12, d23);
        } else {
          d12 = d;
          d23 = d;
        }
        double d13 = sqrt(d12 * d12 + d23 * d23 - 2 * d12 * d23 * cos(alpha));

        perimeter += d13 - (d12 + d23);
      }
    }

    if (!foundation.isExposedPerimeter[v]) {
      interiorPerimeter += getDistance(p2, p3);
    }
  }

  foundation.reductionStrategy = Foundation::RS_CUSTOM;
  foundation.twoParameters = false;
  foundation.reductionLength2 = area / (perimeter - interiorPerimeter);
}

void Ground::setBoundaryConditions() {

  double &azi = bcs.solarAzimuth;
  double &alt = bcs.solarAltitude;
  double &qDN = bcs.directNormalFlux;
  double &qDH = bcs.diffuseHorizontalFlux;
  double qGH = cos(PI / 2 - alt) * qDN + qDH;

  double cosAlt = cos(alt);

  for (auto &surface : foundation.surfaces) {
    if (surface.type == Surface::ST_GRADE || surface.type == Surface::ST_WALL_EXT) {
      bool isWall = surface.type == Surface::ST_WALL_EXT;

      // Short wave
      double pssf;
      double q;

      double incidence = 0.0;

      if (surface.orientation == Surface::Z_POS) {
        incidence = cos(PI / 2 - alt);
      } else if (surface.orientation == Surface::Z_NEG) {
        incidence = cos(PI / 2 - alt - PI);
      } else {
        if (foundation.numberOfDimensions == 2) {
          // incidence is the average incidence on the exterior of a vertical cylinder
          // 2*(int(cos(alt)*cos(x),x,0,PI/2))/(2*PI)
          // 2*(integral of incidence over a quarter of the cylinder) = lit portion
          // divide by the total radians in the circle (2*PI)
          // = 2*(cos(alt))/(2*PI)
          // = cos(alt)/PI
          incidence = cosAlt / PI;
        } else {
          if (foundation.numberOfDimensions == 3 && !foundation.useSymmetry) {
            // incidence = cos(alt)*cos(azi-aziSurf)*sin(tilt)+sin(alt)*cos(tilt)
            // simplifies for tilt = PI/2 to = cos(alt)*cos(azi-aziSurf)
            incidence = cosAlt * cos(azi - surface.azimuth);
          } else // if (foundation.coordinateSystem == Foundation::CS_3D_SYMMETRY)
          {
            // if symmetric, use average incidence (one side will be facing the sun,
            // the other won't).
            if (surface.orientation == Surface::Y_POS || surface.orientation == Surface::Y_NEG) {
              if (foundation.isXSymm) {
                double incidenceYPos = cos(alt) * cos(azi - foundation.orientation);
                if (incidenceYPos < 0)
                  incidenceYPos = 0;

                double incidenceYNeg = cos(alt) * cos(azi - (PI + foundation.orientation));
                if (incidenceYNeg < 0)
                  incidenceYNeg = 0;

                incidence = (incidenceYPos + incidenceYNeg) / 2.0;

              } else {
                incidence = cosAlt * cos(azi - surface.azimuth);
              }
            }

            if (surface.orientation == Surface::X_POS || surface.orientation == Surface::X_NEG) {
              if (foundation.isYSymm) {
                double incidenceXPos = cosAlt * cos(azi - (PI / 2 + foundation.orientation));
                if (incidenceXPos < 0)
                  incidenceXPos = 0;

                double incidenceXNeg = cosAlt * cos(azi - (3 * PI / 3 + foundation.orientation));
                if (incidenceXNeg < 0)
                  incidenceXNeg = 0;

                incidence = (incidenceXPos + incidenceXNeg) / 2.0;

              } else {
                incidence = cosAlt * cos(azi - surface.azimuth);
              }
            }
          }
        }
      }

      // if sun is below horizon, incidence is zero
      if (sin(alt) < 0)
        incidence = 0;
      if (incidence < 0)
        incidence = 0;

      double Fsky = (1.0 + surface.cosTilt) / 2.0;
      double Fg = 1.0 - Fsky;
      double rho_g = 1.0 - foundation.grade.absorptivity;

      for (auto index : surface.indices) {
        auto this_cell = domain.cell[index];
        double alpha = this_cell->surfacePtr->propPtr->absorptivity;

        if (qGH > 0.0) {
          pssf = incidence;
          q = alpha * (qDN * pssf + qDH * Fsky + qGH * Fg * rho_g);
        } else {
          q = 0;
        }
        this_cell->heatGain = q;
      }

      // convection
      ForcedConvectionTerm hfFunc = isWall ? bcs.extWallForcedTerm : bcs.gradeForcedTerm;
      surface.hfTerm =
          hfFunc(surface.cosTilt, surface.azimuth, bcs.windDirection, bcs.localWindSpeed);

      surface.convectionAlgorithm =
          isWall ? bcs.extWallConvectionAlgorithm : bcs.gradeConvectionAlgorithm;

      surface.effectiveLWViewFactorQtr =
          std::sqrt(std::sqrt(getEffectiveExteriorViewFactor(bcs.skyEmissivity, surface.tilt)));

      surface.temperature = bcs.outdoorTemp;

    } else if (surface.type == Surface::ST_SLAB_CORE || surface.type == Surface::ST_SLAB_PERIM ||
               surface.type == Surface::ST_WALL_INT) {
      bool isWall = surface.type == Surface::ST_WALL_INT;
      double absRadiation = isWall ? bcs.wallAbsRadiation : bcs.slabAbsRadiation;

      for (auto index : surface.indices) {
        auto this_cell = domain.cell[index];
        this_cell->heatGain = absRadiation;
      }

      surface.temperature = isWall ? bcs.wallConvectiveTemp : bcs.slabConvectiveTemp;
      surface.radiantTemperature = isWall ? bcs.wallRadiantTemp : bcs.slabRadiantTemp;

      // convection
      surface.hfTerm = 0.0; // Assume no air movement inside
      surface.convectionAlgorithm =
          isWall ? bcs.intWallConvectionAlgorithm : bcs.slabConvectionAlgorithm;

    } else if (surface.type == Surface::ST_DEEP_GROUND) {
      surface.temperature = bcs.deepGroundTemperature;
    }
  }
}

void Ground::link_cells_to_temp() {
  for (const auto &this_cell : domain.cell) {
    this_cell->told_ptr = &TOld[this_cell->index];
  }
}

double getArrayValue(std::vector<std::vector<std::vector<double>>> Mat, std::size_t i,
                     std::size_t j, std::size_t k) {
  return Mat[i][j][k];
}

std::array<double, 3> Ground::calculateHeatFlux(std::size_t index) {
  return domain.cell[index]->calculateHeatFlux(foundation.numberOfDimensions, TNew[index], nX, nY,
                                               nZ, domain.cell);
}

void Ground::writeCSV(std::string path) {

  // Used for debugging purposes

  std::ofstream output;
  output.open(path);

  std::size_t j = nY / 2;
  for (std::size_t i = 0; i < nX; i++) {

    output << ", , " << i;
  }

  output << "\n, ";

  for (std::size_t i = 0; i < nX; i++) {

    output << ", " << domain.mesh[0].centers[i];
  }

  output << "\n";

  for (std::size_t n = nZ; n > 0; n--) {

    std::size_t k = n - 1;
    output << k << ", " << domain.mesh[2].centers[k];

    for (std::size_t i = 0; i < nX; i++) {

      double value;
      std::size_t index = domain.getIndex(i, j, k);

      std::array<double, 3> Qflux = calculateHeatFlux(index);
      double Qx = Qflux[0];
      double Qy = Qflux[1];
      double Qz = Qflux[2];
      double Qmag = sqrt(Qx * Qx + Qy * Qy + Qz * Qz);
      value = Qmag;

      value = domain.cell[index]->surfacePtr
                  ? static_cast<double>(domain.cell[index]->surfacePtr->type)
                  : -1.0;

      output << ", " << value;
    }

    output << "\n";
  }
  output.close();
}

} // namespace Kiva

#endif
