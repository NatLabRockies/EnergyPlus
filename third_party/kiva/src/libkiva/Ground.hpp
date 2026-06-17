/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef GROUND_HPP_
#define GROUND_HPP_

#include "Algorithms.hpp"
#include "BoundaryConditions.hpp"
#include "Domain.hpp"
#include "Foundation.hpp"
#include "GroundOutput.hpp"
#include "Mesher.hpp"
#include "libkiva_export.h"

#include <cmath>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127) // Conditional expression is constant
#pragma warning(disable : 4459) // Declaration hides global declaration
#elif defined(__clang__)
#pragma clang diagnostic push
#if __clang_major__ >= 13
#pragma clang diagnostic ignored "-Wdeprecated-copy"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#elif defined(__GNUC__) && defined(__linux__)
#pragma GCC diagnostic push
#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#endif
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseCore>
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) && defined(__linux__)
#pragma GCC diagnostic pop
#endif

namespace Kiva {

class LIBKIVA_EXPORT Ground {
public:
  // Constructor
  Ground(Foundation &foundation);
  Ground(Foundation &foundation, GroundOutput::OutputMap &outputMap);

  // Destructor
  virtual ~Ground();

  Domain domain;
  Foundation &foundation;
  GroundOutput groundOutput;

  size_t nX, nY, nZ, num_cells;

  std::vector<double> TNew; // solution, n+1
  std::vector<double> TOld; // solution, n

  void buildDomain();

  void calculateBoundaryLayer();
  void setNewBoundaryGeometry();
  void calculate(BoundaryConditions &boundaryConidtions, double ts = 0.0);

  void calculateSurfaceAverages();
  double getSurfaceAverageValue(std::pair<Surface::SurfaceType, GroundOutput::OutputType> output);
  std::array<double, 3> calculateHeatFlux(std::size_t index);
  void writeCSV(std::string path);

private:
  double timestep; // in seconds

  friend class Aggregator;
  BoundaryConditions bcs;
  // Data structures

  // ADE
  std::vector<double> U; // ADE upper sweep, n+1
  std::vector<double> V; // ADE lower sweep, n+1

  // ADI
  std::vector<double> a1; // lower diagonal
  std::vector<double> a2; // main diagonal
  std::vector<double> a3; // upper diagonal
  std::vector<double> b_; // right-hand side
  std::vector<double> x_; // solution

  // Implicit
  Eigen::SparseMatrix<double> Amat;
  std::vector<Eigen::Triplet<double>> tripletList;
  Eigen::VectorXd b, x;

  std::shared_ptr<Eigen::BiCGSTAB<Eigen::SparseMatrix<double>, Eigen::IncompleteLUT<double>>>
      pSolver;

private:
  // Calculators (Called from main calculator)
  void calculateADE();

  void calculateADEUpwardSweep();

  void calculateADEDownwardSweep();

  void calculateExplicit();

  void calculateMatrix(Foundation::NumericalScheme scheme);

  void calculateADI(int dim);

  // Misc. Functions
  void setAmatValue(const std::size_t i, const std::size_t j, const double val);
  void setbValue(const std::size_t i, const double val);
  void setValuesADI(const std::size_t &index, const double &A, const double (&Alt)[2],
                    const double &bVal);
  void solveLinearSystem();
  void clearAmat();
  double getxValue(const int i);
  std::vector<double> getXvalues();

  double getSurfaceArea(Surface::SurfaceType surfaceType);

  void setBoundaryConditions();

  std::vector<std::pair<double, double>> boundaryLayer;

  double getBoundaryValue(double dist);

  double getBoundaryDistance(double val);
  void link_cells_to_temp();
};

} // namespace Kiva

#endif /* GROUND_HPP_ */
