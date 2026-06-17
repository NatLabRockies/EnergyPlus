/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Functions_HPP
#define Functions_HPP

#include "libkiva_export.h"

#include <algorithm>
#include <fstream>
#include <math.h>
#include <vector>

namespace Kiva {

static const double EPSILON = 1E-7;

bool LIBKIVA_EXPORT isLessThan(double first, double second, double epsilon = EPSILON);
bool LIBKIVA_EXPORT isLessOrEqual(double first, double second, double epsilon = EPSILON);
bool LIBKIVA_EXPORT isEqual(double first, double second, double epsilon = EPSILON);
bool LIBKIVA_EXPORT isGreaterThan(double first, double second, double epsilon = EPSILON);
bool LIBKIVA_EXPORT isGreaterOrEqual(double first, double second, double epsilon = EPSILON);
bool LIBKIVA_EXPORT isEven(int N);
bool LIBKIVA_EXPORT isOdd(int N);
void solveTDM(const std::vector<double> &a1, const std::vector<double> &a2, std::vector<double> &a3,
              std::vector<double> &b, std::vector<double> &x);
} // namespace Kiva

#endif
