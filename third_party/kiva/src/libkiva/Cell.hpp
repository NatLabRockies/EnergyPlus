/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Cell_HPP
#define Cell_HPP

#include "Algorithms.hpp"
#include "BoundaryConditions.hpp"
#include "Foundation.hpp"
#include "Functions.hpp"
#include "Mesher.hpp"
#include "libkiva_export.h"

#include <fstream>
#include <memory>
#include <numeric>

namespace Kiva {

enum CellType {
  EXTERIOR_AIR,  // 0
  INTERIOR_AIR,  // 1
  NORMAL,        // 2
  BOUNDARY,      // 3
  ZERO_THICKNESS // 4
};

class LIBKIVA_EXPORT Cell {
public:
  Cell(const std::size_t &index, const CellType cellType, const std::size_t &i,
       const std::size_t &j, const std::size_t &k, std::size_t *stepsize,
       const Foundation &foundation, Surface *surfacePtr, Block *blockPtr, Mesher *mesh);

  virtual ~Cell(){};

  std::size_t coords[3], index;
  std::size_t *stepsize;
  std::size_t dims[3];

  // inherent properties
  double density;
  double specificHeat;
  double conductivity;
  double iHeatCapacity, iHeatCapacityADI; // used to minimize operations in caclulation loops

  double volume;
  double area, r;
  double heatGain;

  // derived properties
  double pde[3][2]{{0}}, pde_c[2]{0};
  double dist[3][2];
  double kcoeff[3][2];
  double *told_ptr;
  // organizational properties

  CellType cellType;

  Block *blockPtr;

  Surface *surfacePtr;
  Mesher *meshPtr;

  void setDistances(const double dxp_in, const double dxm_in, const double dyp_in,
                    const double dym_in, const double dzp_in, const double dzm_in);

  void setConductivities(const std::vector<std::shared_ptr<Cell>> &cell_v);

  void setPDEcoefficients(int ndims, bool cylindrical);
  double onePDEcoefficient(std::size_t dim, std::size_t dir);

  void setComputeDims(std::size_t (&dims)[3]);

  void setZeroThicknessCellProperties(std::vector<std::shared_ptr<Cell>> pointSet);

  virtual void calcCellADEUp(double timestep, const Foundation &foundation,
                             const BoundaryConditions &bcs, double &U);
  virtual void calcCellADEDown(double timestep, const Foundation &foundation,
                               const BoundaryConditions &bcs, double &V);

  virtual double calcCellExplicit(double timestep, const Foundation &foundation,
                                  const BoundaryConditions &bcs);

  virtual void calcCellMatrix(Foundation::NumericalScheme scheme, const double timestep,
                              const Foundation &foundation, const BoundaryConditions &bcs,
                              double &A, double (&Alt)[3][2], double &bVal);

  void calcCellSteadyState(const Foundation &foundation, double &A, double (&Alt)[3][2],
                           double &bVal);

  virtual void calcCellADI(std::size_t dim, const double timestep, const Foundation &foundation,
                           const BoundaryConditions &bcs, double &A, double (&Alt)[2],
                           double &bVal);
  void gatherCCoeffs(const double theta, bool cylindrical, double (&C)[3][2]);

  virtual std::array<double, 3> calculateHeatFlux(int ndims, double &TNew, std::size_t nX,
                                                  std::size_t nY, std::size_t nZ,
                                                  const std::vector<std::shared_ptr<Cell>> &cell_v);

  void Assemble(const Foundation &foundation);

  inline void doOutdoorTemp(const BoundaryConditions &bcs, double &A, double &bVal);
  inline void doIndoorTemp(const BoundaryConditions &bcs, double &A, double &bVal);

  void ADImath(std::size_t dim, const double Q, const double f, const double multiplier,
               const double (&C)[3][2], double &A, double (&Alt)[2], double &bVal);
};

class ExteriorAirCell : public Cell {
public:
  ExteriorAirCell(const std::size_t &index, const CellType cellType, const std::size_t &i,
                  const std::size_t &j, const std::size_t &k, std::size_t *stepsize,
                  const Foundation &foundation, Surface *surfacePtr, Block *blockPtr,
                  Mesher *meshptr);

  void calcCellADEUp(double timestep, const Foundation &foundation, const BoundaryConditions &bcs,
                     double &U) override;
  void calcCellADEDown(double timestep, const Foundation &foundation, const BoundaryConditions &bcs,
                       double &v) override;
  double calcCellExplicit(double timestep, const Foundation &foundation,
                          const BoundaryConditions &bcs) override;
  void calcCellMatrix(Foundation::NumericalScheme scheme, const double timestep,
                      const Foundation &foundation, const BoundaryConditions &bcs, double &A,
                      double (&Alt)[3][2], double &bVal) override;
  void calcCellADI(std::size_t dim, const double timestep, const Foundation &foundation,
                   const BoundaryConditions &bcs, double &A, double (&Alt)[2],
                   double &bVal) override;
  std::array<double, 3>
  calculateHeatFlux(int ndims, double &TNew, std::size_t nX, std::size_t nY, std::size_t nZ,
                    const std::vector<std::shared_ptr<Cell>> &cell_v) override;
};

class InteriorAirCell : public Cell {
public:
  InteriorAirCell(const std::size_t &index, const CellType cellType, const std::size_t &i,
                  const std::size_t &j, const std::size_t &k, std::size_t *stepsize,
                  const Foundation &foundation, Surface *surfacePtr, Block *blockPtr,
                  Mesher *meshptr);

  void calcCellADEUp(double timestep, const Foundation &foundation, const BoundaryConditions &bcs,
                     double &U) override;
  void calcCellADEDown(double timestep, const Foundation &foundation, const BoundaryConditions &bcs,
                       double &v) override;
  double calcCellExplicit(double timestep, const Foundation &foundation,
                          const BoundaryConditions &bcs) override;
  void calcCellMatrix(Foundation::NumericalScheme scheme, const double timestep,
                      const Foundation &foundation, const BoundaryConditions &bcs, double &A,
                      double (&Alt)[3][2], double &bVal) override;
  void calcCellADI(std::size_t dim, const double timestep, const Foundation &foundation,
                   const BoundaryConditions &bcs, double &A, double (&Alt)[2],
                   double &bVal) override;
  std::array<double, 3>
  calculateHeatFlux(int ndims, double &TNew, std::size_t nX, std::size_t nY, std::size_t nZ,
                    const std::vector<std::shared_ptr<Cell>> &cell_v) override;
};

class BoundaryCell : public Cell {
public:
  BoundaryCell(const std::size_t &index, const CellType cellType, const std::size_t &i,
               const std::size_t &j, const std::size_t &k, std::size_t *stepsize,
               const Foundation &foundation, Surface *surfacePtr, Block *blockPtr, Mesher *meshptr);

  void calcCellADEUp(double timestep, const Foundation &foundation, const BoundaryConditions &bcs,
                     double &U) override;
  void calcCellADEDown(double timestep, const Foundation &foundation, const BoundaryConditions &bcs,
                       double &V) override;
  double calcCellExplicit(double timestep, const Foundation &foundation,
                          const BoundaryConditions &bcs) override;
  void calcCellMatrix(Foundation::NumericalScheme scheme, const double timestep,
                      const Foundation &foundation, const BoundaryConditions &bcs, double &A,
                      double (&Alt)[3][2], double &bVal) override;
  void calcCellADI(std::size_t dim, const double timestep, const Foundation &foundation,
                   const BoundaryConditions &bcs, double &A, double (&Alt)[2],
                   double &bVal) override;
  std::array<double, 3>
  calculateHeatFlux(int ndims, double &TNew, std::size_t nX, std::size_t nY, std::size_t nZ,
                    const std::vector<std::shared_ptr<Cell>> &cell_v) override;

private:
  inline void zfCellADI(const std::size_t dim, const std::size_t sdim, const std::size_t sign,
                        double &A, double &Alt, double &bVal);
  inline void ifCellADI(const std::size_t dim, const std::size_t sdim, const std::size_t dir,
                        double &A, double &Alt, double &bVal);
  inline void efCellADI(const std::size_t dim, const std::size_t sdim, const std::size_t sign,
                        double &A, double &Alt, double &bVal);
  inline void zfCellMatrix(double &A, double &Alt, double &bVal);
  inline void ifCellMatrix(const std::size_t dim, const std::size_t dir, double &A, double &Alt,
                           double &bVal);
  inline void efCellMatrix(const std::size_t dim, const std::size_t dir, double &A, double &Alt,
                           double &bVal);
  inline void zfCellADEUp(const std::size_t dim, const std::size_t &dir, double &U);
  inline void ifCellADEUp(const std::size_t dim, const std::size_t dir, double &U);
  inline void efCellADEUp(const std::size_t dim, const std::size_t dir, double &U);
  inline void zfCellADEDown(const std::size_t dim, const std::size_t &dir, double &V);
  inline void ifCellADEDown(const std::size_t dim, const std::size_t dir, double &V);
  inline void efCellADEDown(const std::size_t dim, const std::size_t dir, double &V);
  inline double zfCellExplicit(const std::size_t dim, const std::size_t &dir);
  inline double ifCellExplicit(const std::size_t dim, const std::size_t &dir);
  inline double efCellExplicit(const std::size_t dim, const std::size_t &dir);
};

class ZeroThicknessCell : public Cell {
public:
  ZeroThicknessCell(const std::size_t &index, const CellType cellType, const std::size_t &i,
                    const std::size_t &j, const std::size_t &k, std::size_t *stepsize,
                    const Foundation &foundation, Surface *surfacePtr, Block *blockPtr,
                    Mesher *meshptr);

  std::array<double, 3>
  calculateHeatFlux(int ndims, double &TNew, std::size_t nX, std::size_t nY, std::size_t nZ,
                    const std::vector<std::shared_ptr<Cell>> &cell_v) override;
};

} // namespace Kiva

#endif
