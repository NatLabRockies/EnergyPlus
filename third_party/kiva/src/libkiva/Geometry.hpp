/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4100) // Unreferenced formal parameter
#pragma warning(disable : 4127) // Conditional expression is constant
#pragma warning(disable : 4244) // Possible loss of data from conversion
#pragma warning(disable : 4702) // Unreachable code
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // sprintf in lexical_cast
#elif defined(__GNUC__) && defined(__linux__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif

// Extended precision is not used by kiva
// Disable boost quadmath to avoid warnings
#define BOOST_CSTDFLOAT_NO_LIBQUADMATH_CMATH
#define BOOST_CSTDFLOAT_NO_LIBQUADMATH_IOSTREAM
#define BOOST_CSTDFLOAT_NO_LIBQUADMATH_COMPLEX

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) && defined(__linux__)
#pragma GCC diagnostic pop
#endif

#include "Functions.hpp"
#include "libkiva_export.h"

namespace Kiva {

typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> Point;
typedef boost::geometry::model::polygon<Point, true, false> Polygon;
typedef boost::geometry::model::ring<Point, true, false> Ring;
typedef boost::geometry::model::multi_polygon<Polygon> MultiPolygon;
typedef boost::geometry::model::multi_point<Point> MultiPoint;
typedef boost::geometry::model::linestring<Point> Line;
typedef boost::geometry::model::box<Point> Box;
typedef boost::geometry::model::point<double, 3, boost::geometry::cs::cartesian> Point3;
typedef boost::geometry::model::polygon<Point3, true, false> Polygon3;

namespace geom {
enum Direction { X_NEG, X_POS, Y_NEG, Y_POS };

enum Turn { LEFT, RIGHT };
} // namespace geom

bool isRectilinear(Polygon poly);
Polygon offset(const Polygon &poly, const double dist);
geom::Direction getDirectionIn(Polygon poly, std::size_t vertex);
geom::Direction getDirectionOut(Polygon poly, std::size_t vertex);
geom::Turn getTurn(Polygon poly, std::size_t vertex);
MultiPolygon mirrorX(MultiPolygon poly, double x);
MultiPolygon mirrorY(MultiPolygon poly, double y);
Polygon symmetricUnit(Polygon poly);
bool isXSymmetric(Polygon poly);
bool isYSymmetric(Polygon poly);
bool isCounterClockWise(Polygon poly);
double getXmin(Polygon poly, std::size_t vertex);
double getYmin(Polygon poly, std::size_t vertex);
double getXmax(Polygon poly, std::size_t vertex);
double getYmax(Polygon poly, std::size_t vertex);

bool LIBKIVA_EXPORT comparePointsX(Point first, Point second);
bool LIBKIVA_EXPORT comparePointsY(Point first, Point second);
bool LIBKIVA_EXPORT pointOnPoly(Point point, Polygon poly);
bool LIBKIVA_EXPORT isConvex(Polygon poly);

double LIBKIVA_EXPORT getDistance(Point a, Point b);

double getAngle(Point a, Point b, Point c);

} // namespace Kiva

#endif /* GEOMETRY_H_ */
