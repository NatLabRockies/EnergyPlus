/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Mesher_CPP
#define Mesher_CPP

#include "Mesher.hpp"

namespace Kiva {

Mesher::Mesher() {}

Mesher::Mesher(MeshData &data) : data(data) {
  dividers.push_back(data.points[0]);

  // if there is only one point (or fewer), make into a zero thickness mesh
  // (effectively eliminates the dimension from the domain.
  if (data.points.size() <= 3) {
    dividers.push_back(1.0);
    deltas.push_back(1.0);
    centers.push_back(0.5);
    return;
  }

  // Loop through intervals
  for (std::size_t i = 0; i < data.points.size() - 1; ++i) {
    double min = data.points[i];
    double max = data.points[i + 1];
    double length = max - min;
    double cellWidth;
    int numCells{0};

    if (isEqual(length, 0.0)) {
      // Zero width cells (used for boundary conditions)
      dividers.push_back(max);
      deltas.push_back(0.0);
      centers.push_back(max);
    } else {
      // Uniform Meshing
      if (data.intervals[i].growthDir == Interval::UNIFORM) {
        numCells = static_cast<int>(length / data.intervals[i].minCellDim);

        // Make sure that there is at least one cell
        if (numCells == 0)
          numCells = 1;

        if (isEqual(length / (numCells + 1), data.intervals[i].minCellDim)) {
          cellWidth = data.intervals[i].minCellDim;
          numCells = numCells + 1;
        } else {
          cellWidth = length / numCells;
        }

        for (int j = 1; j <= numCells; ++j) {
          dividers.push_back(min + j * cellWidth);
          deltas.push_back(cellWidth);
          centers.push_back(min + j * cellWidth - cellWidth / 2.0);
        }

      } else {
        // TODO: These algorithms need to be cleaned up and commented better
        // Geometric Meshing
        std::vector<double> temp;

        // Create temp. delta array
        if (isGreaterOrEqual(data.intervals[i].minCellDim, length)) {
          // If min. cell width is greater than interval length then
          // set width equal to the length
          temp.push_back(length);
          numCells = 1;

        } else {

          if (data.intervals[i].growthDir == Interval::CENTERED) {
            bool search = true;
            int N = 1;
            double multiplier;
            double nTerm;
            double seriesTerm;
            double previousMultiplier = 1.0;

            while (search) {
              seriesTerm = 0.0;

              if (isOdd(N)) {
                nTerm = pow(data.intervals[i].maxGrowthCoeff, (N - 1) / 2);
                for (int j = 0; j <= (N - 1) / 2 - 1; j++) {
                  seriesTerm += 2 * pow(data.intervals[i].maxGrowthCoeff, j);
                }
              } else {
                nTerm = 0;
                for (int j = 0; j <= N / 2 - 1; j++) {
                  seriesTerm += 2 * pow(data.intervals[i].maxGrowthCoeff, j);
                }
              }
              multiplier = seriesTerm + nTerm;

              if (data.intervals[i].minCellDim * multiplier > length) {
                numCells = N - 1;
                multiplier = previousMultiplier;
                search = false;
              } else {
                previousMultiplier = multiplier;
                N += 1;
              }
            }
            double initialCellWidth = length / previousMultiplier;
            temp.push_back(initialCellWidth);

            for (int j = 1; j < numCells; j++) {
              if (isOdd(numCells)) {
                if (j <= (numCells - 1) / 2)
                  temp.push_back(temp[j - 1] * data.intervals[i].maxGrowthCoeff);
                else
                  temp.push_back(temp[j - 1] / data.intervals[i].maxGrowthCoeff);
              } else {
                if (j < numCells / 2)
                  temp.push_back(temp[j - 1] * data.intervals[i].maxGrowthCoeff);
                else if (j == numCells / 2)
                  temp.push_back(temp[j - 1]);
                else
                  temp.push_back(temp[j - 1] / data.intervals[i].maxGrowthCoeff);
              }
            }

          } else {
            bool search = true;
            int N = 0;
            double multiplier{0};

            while (search) {
              multiplier = 0.0;
              for (int j = 0; j <= N; j++) {
                multiplier += pow(data.intervals[i].maxGrowthCoeff, j);
              }

              if (data.intervals[i].minCellDim * multiplier > length) {
                numCells = N;
                multiplier -= pow(data.intervals[i].maxGrowthCoeff, N);
                search = false;
              } else {
                N += 1;
              }
            }
            double initialCellWidth = length / multiplier;
            temp.push_back(initialCellWidth);

            for (int j = 1; j < numCells; j++) {
              temp.push_back(temp[j - 1] * data.intervals[i].maxGrowthCoeff);
            }
          }
        }

        // build arrays
        if (data.intervals[i].growthDir == Interval::FORWARD) {
          double position = min;
          for (int j = 0; j < numCells; ++j) {
            dividers.push_back(position + temp[j]);
            deltas.push_back(temp[j]);
            centers.push_back(position + temp[j] / 2.0);
            position += temp[j];
          }

        } else if (data.intervals[i].growthDir == Interval::BACKWARD) {
          double position = min;
          for (int j = 1; j <= numCells; ++j) {
            dividers.push_back(position + temp[numCells - j]);
            deltas.push_back(temp[numCells - j]);
            centers.push_back(position + temp[numCells - j] / 2.0);
            position += temp[numCells - j];
          }
        } else {
          double position = min;
          for (int j = 0; j < numCells; ++j) {
            dividers.push_back(position + temp[j]);
            deltas.push_back(temp[j]);
            centers.push_back(position + temp[j] / 2.0);
            position += temp[j];
          }
        }
      }
    }
  }
}

std::size_t Mesher::getNearestIndex(double position) {
  if (isLessOrEqual(position, this->centers[0]))
    return 0;
  else if (isGreaterOrEqual(position, this->centers[this->centers.size() - 1]))
    return this->centers.size() - 1;
  else {
    for (std::size_t i = 1; i < this->centers.size(); i++) {
      if (isGreaterOrEqual(position, this->centers[i - 1]) &&
          isLessOrEqual(position, this->centers[i])) {
        double diffDown = position - this->centers[i - 1];
        double diffUp = this->centers[i] - position;

        if (isLessOrEqual(diffDown, diffUp))
          return i - 1;
        else
          return i;
      }
    }
    showMessage(MSG_ERR, "Could not find the nearest Index.");
    return 0;
  }
}

std::size_t Mesher::getNextIndex(double position) {
  if (isLessThan(position, this->centers[0]))
    return 0;
  else if (isGreaterOrEqual(position, this->centers[this->centers.size() - 1]))
    return this->centers.size() - 1;
  else {
    for (std::size_t i = 0; i < this->centers.size() - 1; i++) {
      if (isGreaterOrEqual(position, this->centers[i]) &&
          isLessThan(position, this->centers[i + 1]))
        return i + 1;
    }
    showMessage(MSG_ERR, "Could not find the next Index.");
    return 0;
  }
}

std::size_t Mesher::getPreviousIndex(double position) {
  if (isLessOrEqual(position, this->centers[0]))
    return 0;
  else if (isGreaterThan(position, this->centers[this->centers.size() - 1]))
    return this->centers.size() - 1;
  else {
    for (std::size_t i = 1; i < this->centers.size(); i++) {
      if (isGreaterThan(position, this->centers[i - 1]) &&
          isLessOrEqual(position, this->centers[i]))
        return i - 1;
    }
    showMessage(MSG_ERR, "Could not find the previous Index.");
    return 0;
  }
}
} // namespace Kiva

#endif
