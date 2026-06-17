/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#include "fixtures/bestest-fixture.hpp"
#include "fixtures/typical-fixture.hpp"

#include "Errors.hpp"

using namespace Kiva;

class CellFixture : public BESTESTFixture {
protected:
  void SetUp() {
    specifySystem();
    ground = std::make_shared<Ground>(fnd, outputMap);
    ground->foundation.createMeshData();
    Domain domain = ground->domain;
    domain.setDomain(ground->foundation);
    cell_vector = domain.cell;
  };

  std::vector<std::shared_ptr<Cell>> cell_vector;
};

TEST_F(CellFixture, cell_basics) {
  EXPECT_EQ(cell_vector[0]->coords[0], 0u);
  EXPECT_EQ(cell_vector[0]->coords[1], 0u);
  EXPECT_EQ(cell_vector[0]->coords[2], 0u);
  EXPECT_EQ(cell_vector[0]->index, 0u);
  EXPECT_EQ(cell_vector[0]->cellType, CellType::BOUNDARY);
  EXPECT_EQ(cell_vector[0]->surfacePtr->type, Surface::SurfaceType::ST_DEEP_GROUND);

  EXPECT_EQ(cell_vector[120]->coords[0], 38u);
  EXPECT_EQ(cell_vector[120]->coords[1], 0u);
  EXPECT_EQ(cell_vector[120]->coords[2], 2u);
  EXPECT_EQ(cell_vector[120]->index, 120u);
  EXPECT_EQ(cell_vector[120]->cellType, CellType::NORMAL);
  //  TODO: replace surfacePtr and blockPtr NULL with nullptr. This EXPECT_EQ errors.
  //  EXPECT_EQ(cell_vector[120]->surfacePtr, NULL);

  EXPECT_EQ(cell_vector[47]->dims[0], 0u);
  EXPECT_EQ(cell_vector[47]->dims[1], 5u);
  EXPECT_EQ(cell_vector[47]->dims[2], 2u);
}

void resetValues(double &A, double (&Alt)[2], double &bVal) {
  A = 0.0;
  Alt[0] = 0.0;
  Alt[1] = 0.0;
  bVal = 0.0;
}

TEST_F(GC10aFixture, calcCellADI) {
  fnd.numericalScheme = Foundation::NS_ADI;
  calculate();
  double A{0.0}, Alt[]{0.0, 0.0}, bVal{0.0};

  // constant temperature boundary cell
  auto this_cell = ground->domain.cell[0];
  this_cell->calcCellADI(0, 3600.0, fnd, bcs, A, Alt, bVal);
  EXPECT_DOUBLE_EQ(A, 1);
  EXPECT_DOUBLE_EQ(Alt[1], 0);
  EXPECT_DOUBLE_EQ(Alt[0], 0);
  EXPECT_DOUBLE_EQ(bVal, this_cell->surfacePtr->temperature);

  // normal cell
  this_cell = ground->domain.cell[120];
  resetValues(A, Alt, bVal);
  this_cell->calcCellADI(0, 3600.0, fnd, bcs, A, Alt, bVal);
  double theta = 3600.0 / (fnd.numberOfDimensions * this_cell->density * this_cell->specificHeat);
  double f = fnd.fADI;
  EXPECT_DOUBLE_EQ(A, 1.0 + (2 - f) * (this_cell->pde[0][1] - this_cell->pde[0][0]) * theta);
  EXPECT_DOUBLE_EQ(Alt[1], (2 - f) * (-this_cell->pde[0][1] * theta));
  EXPECT_DOUBLE_EQ(Alt[0], (2 - f) * (this_cell->pde[0][0] * theta));
  EXPECT_DOUBLE_EQ(
      bVal,
      *this_cell->told_ptr * (1.0 + f * (this_cell->pde[2][0] - this_cell->pde[2][1]) * theta) -
          *(this_cell->told_ptr - ground->domain.stepsize[2]) * f * this_cell->pde[2][0] * theta +
          *(this_cell->told_ptr + ground->domain.stepsize[2]) * f * this_cell->pde[2][1] * theta +
          this_cell->heatGain * theta);

  resetValues(A, Alt, bVal);
  this_cell->calcCellADI(2, 3600.0, fnd, bcs, A, Alt, bVal);
  EXPECT_DOUBLE_EQ(A, 1.0 + (2 - f) * (this_cell->pde[2][1] - this_cell->pde[2][0]) * theta);
  EXPECT_DOUBLE_EQ(Alt[1], (2 - f) * (-this_cell->pde[2][1] * theta));
  EXPECT_DOUBLE_EQ(Alt[0], (2 - f) * (this_cell->pde[2][0] * theta));
  EXPECT_DOUBLE_EQ(
      bVal,
      *this_cell->told_ptr * (1.0 + f * (this_cell->pde[0][0] - this_cell->pde[0][1]) * theta) -
          *(this_cell->told_ptr - ground->domain.stepsize[0]) * f * this_cell->pde[0][0] * theta +
          *(this_cell->told_ptr + ground->domain.stepsize[0]) * f * this_cell->pde[0][1] * theta +
          this_cell->heatGain * theta);

  // zero_flux, x_neg boundary cell
  this_cell = ground->domain.cell[123];
  resetValues(A, Alt, bVal);
  this_cell->calcCellADI(0, 3600.0, fnd, bcs, A, Alt, bVal);
  EXPECT_DOUBLE_EQ(A, 1);
  EXPECT_DOUBLE_EQ(Alt[1], -1.0);
  EXPECT_DOUBLE_EQ(Alt[0], 0);
  EXPECT_DOUBLE_EQ(bVal, 0);

  resetValues(A, Alt, bVal);
  this_cell->calcCellADI(2, 3600.0, fnd, bcs, A, Alt, bVal);
  EXPECT_DOUBLE_EQ(A, 1);
  EXPECT_DOUBLE_EQ(Alt[1], 0);
  EXPECT_DOUBLE_EQ(Alt[0], 0);
  EXPECT_DOUBLE_EQ(bVal, *(this_cell->told_ptr + ground->domain.stepsize[0]));
}

TEST_F(GC10aFixture, calcCellMatrix) {
  fnd.numericalScheme = Foundation::NS_IMPLICIT;
  calculate();
  double A{0}, bVal{0};
  double Alt[3][2] = {{0}};
  auto this_cell = ground->domain.cell[0];
  this_cell->calcCellMatrix(fnd.numericalScheme, 3600.0, fnd, bcs, A, Alt, bVal);
  EXPECT_DOUBLE_EQ(A, 1);
  EXPECT_DOUBLE_EQ(Alt[0][1], 0);
  EXPECT_DOUBLE_EQ(Alt[0][0], 0);
  EXPECT_DOUBLE_EQ(bVal, this_cell->surfacePtr->temperature);

  this_cell = ground->domain.cell[120];
  this_cell->calcCellMatrix(fnd.numericalScheme, 3600.0, fnd, bcs, A, Alt, bVal);
  double theta = 3600.0 / (this_cell->density * this_cell->specificHeat);
  EXPECT_DOUBLE_EQ(A, (1.0 + (this_cell->pde[0][1] + this_cell->pde[2][1] - this_cell->pde[0][0] -
                              this_cell->pde[2][0]) *
                                 theta));
  EXPECT_DOUBLE_EQ(Alt[0][1], -this_cell->pde[0][1] * theta);
  EXPECT_DOUBLE_EQ(Alt[0][0], this_cell->pde[0][0] * theta);
  EXPECT_DOUBLE_EQ(bVal, *this_cell->told_ptr + this_cell->heatGain * theta);
}

TEST_F(GC10aFixture, calcCellMatrixSS) {
  fnd.numericalScheme = Foundation::NS_STEADY_STATE;
  calculate();
  double A{0}, bVal{0};
  double Alt[3][2] = {{0}};
  auto this_cell = ground->domain.cell[0];
  this_cell->calcCellMatrix(fnd.numericalScheme, 3600.0, fnd, bcs, A, Alt, bVal);
  EXPECT_DOUBLE_EQ(A, 1);
  EXPECT_DOUBLE_EQ(Alt[0][1], 0);
  EXPECT_DOUBLE_EQ(Alt[0][1], 0);
  EXPECT_DOUBLE_EQ(bVal, this_cell->surfacePtr->temperature);

  this_cell = ground->domain.cell[120];
  this_cell->calcCellMatrix(fnd.numericalScheme, 3600.0, fnd, bcs, A, Alt, bVal);
  EXPECT_DOUBLE_EQ(A, this_cell->pde[0][0] + this_cell->pde[2][0] - this_cell->pde[0][1] -
                          this_cell->pde[2][1]);
  EXPECT_DOUBLE_EQ(Alt[0][1], this_cell->pde[0][1]);
  EXPECT_DOUBLE_EQ(Alt[0][0], -this_cell->pde[0][0]);
  EXPECT_DOUBLE_EQ(bVal, 0);
}
