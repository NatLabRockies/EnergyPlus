/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef GEOMETRY_CPP_
#define GEOMETRY_CPP_

#include "Geometry.hpp"
#include "Errors.hpp"

namespace Kiva {

static const double PI = 4.0 * atan(1.0);

bool isCounterClockWise(Polygon poly) {
  double area = boost::geometry::area(poly);
  if (area < 0) {
    return false;
  }
  return true;
}

bool isRectilinear(Polygon poly) {
  for (std::size_t v = 0; v < poly.outer().size(); v++) {
    double x = poly.outer()[v].get<0>();
    double y = poly.outer()[v].get<1>();
    double xNext, yNext;
    if (v == poly.outer().size() - 1) {
      xNext = poly.outer()[0].get<0>();
      yNext = poly.outer()[0].get<1>();
    } else {
      xNext = poly.outer()[v + 1].get<0>();
      yNext = poly.outer()[v + 1].get<1>();
    }

    if (isEqual(x, xNext) || isEqual(y, yNext)) {
      // Do nothing
    } else {
      return false;
    }
  }
  return true;
}

Polygon offset(const Polygon &poly, const double dist) {
  static const boost::geometry::strategy::buffer::join_miter join_strategy;
  boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy(dist);
  static const boost::geometry::strategy::buffer::end_flat end_strategy;
  static const boost::geometry::strategy::buffer::side_straight side_strategy;
  static const boost::geometry::strategy::buffer::point_square point_strategy;

  MultiPolygon offset;

  boost::geometry::buffer(poly, offset, distance_strategy, side_strategy, join_strategy,
                          end_strategy, point_strategy);

  return offset[0];
}

geom::Direction getDirectionIn(Polygon poly, std::size_t vertex) {
  if (!isRectilinear(poly)) {
    showMessage(MSG_ERR, "Cannot get direction of vertex for non-rectilinear polygon.");
  }

  double xPrev;
  double yPrev;
  double x;
  double y;

  std::size_t nV = poly.outer().size();

  if (vertex == 0) {
    xPrev = poly.outer()[nV - 1].get<0>();
    yPrev = poly.outer()[nV - 1].get<1>();
  } else {
    xPrev = poly.outer()[vertex - 1].get<0>();
    yPrev = poly.outer()[vertex - 1].get<1>();
  }

  x = poly.outer()[vertex].get<0>();
  y = poly.outer()[vertex].get<1>();

  if (isLessThan(x, xPrev)) {
    return geom::X_NEG;
  } else if (isGreaterThan(x, xPrev)) {
    return geom::X_POS;
  } else if (isLessThan(y, yPrev)) {
    return geom::Y_NEG;
  } else // if (isGreaterThan(y,yPrev))
  {
    return geom::Y_POS;
  }
}

geom::Direction getDirectionOut(Polygon poly, std::size_t vertex) {
  if (!isRectilinear(poly)) {
    showMessage(MSG_ERR, "Cannot get direction of vertex for non-rectilinear polygon.");
  }

  double xNext;
  double yNext;
  double x;
  double y;

  std::size_t nV = poly.outer().size();

  if (vertex == nV - 1) {
    xNext = poly.outer()[0].get<0>();
    yNext = poly.outer()[0].get<1>();
  } else {
    xNext = poly.outer()[vertex + 1].get<0>();
    yNext = poly.outer()[vertex + 1].get<1>();
  }

  x = poly.outer()[vertex].get<0>();
  y = poly.outer()[vertex].get<1>();

  if (isLessThan(xNext, x)) {
    return geom::X_NEG;
  } else if (isGreaterThan(xNext, x)) {
    return geom::X_POS;
  } else if (isLessThan(yNext, y)) {
    return geom::Y_NEG;
  } else // if (isGreaterThan(yNext,y))
  {
    return geom::Y_POS;
  }
}

geom::Turn getTurn(Polygon poly, std::size_t vertex) {
  switch (getDirectionIn(poly, vertex)) {
  case geom::X_NEG: {
    if (getDirectionOut(poly, vertex) == geom::Y_POS)
      return geom::RIGHT;
    else // if (getDirectionOut(poly,vertex) == Y_NEG)
      return geom::LEFT;
  } break;
  case geom::X_POS: {
    if (getDirectionOut(poly, vertex) == geom::Y_POS)
      return geom::LEFT;
    else // if (getDirectionOut(poly,vertex) == Y_NEG)
      return geom::RIGHT;
  } break;
  case geom::Y_NEG: {
    if (getDirectionOut(poly, vertex) == geom::X_POS)
      return geom::LEFT;
    else // if (getDirectionOut(poly,vertex) == X_NEG)
      return geom::RIGHT;
  } break;
  default: // case geom::Y_POS:
  {
    if (getDirectionOut(poly, vertex) == geom::X_POS)
      return geom::RIGHT;
    else // if (getDirectionOut(poly,vertex) == X_NEG)
      return geom::LEFT;
  } break;
  }
}

#if defined(KIVA_3D)
MultiPolygon mirrorX(MultiPolygon poly, double x) {
  boost::geometry::strategy::transform::matrix_transformer<double, 2, 2> transform(-1, 0, 2 * x, 0,
                                                                                   1, 0, 0, 0, 1);

  MultiPolygon mirror;

  boost::geometry::transform(poly, mirror, transform);

  boost::geometry::reverse(mirror);

  return mirror;
}

MultiPolygon mirrorY(MultiPolygon poly, double y) {
  boost::geometry::strategy::transform::matrix_transformer<double, 2, 2> transform(1, 0, 0, 0, -1,
                                                                                   2 * y, 0, 0, 1);

  MultiPolygon mirror;

  boost::geometry::transform(poly, mirror, transform);

  boost::geometry::reverse(mirror);

  return mirror;
}

bool isXSymmetric(Polygon poly) {

  // Find centroid
  Point centroid;
  boost::geometry::centroid(poly, centroid);
  double centroidX = centroid.get<0>();

  // Create Left and Right bounding boxes
  Box bb;
  boost::geometry::envelope(poly, bb);

  Box bbLeft(bb.min_corner(), Point(centroidX, bb.max_corner().get<1>()));
  Box bbRight(Point(centroidX, bb.min_corner().get<1>()), bb.max_corner());

  // Create Left and Right polygons
  MultiPolygon left;
  MultiPolygon right;

  boost::geometry::intersection(poly, bbLeft, left);
  boost::geometry::intersection(poly, bbRight, right);

  // Mirror right polygon across the centroid
  right = mirrorX(right, centroidX);

  MultiPolygon intersection;

  boost::geometry::intersection(left, right, intersection);

  return (isEqual(boost::geometry::area(left), boost::geometry::area(intersection), 1e-4));

  /* Weird stuff needed for equals statement...
   * Turns out the equals statement has a pretty tight tolerance
   *
  boost::geometry::unique(left);
  boost::geometry::unique(right);

  boost::geometry::append(left[0], Point(left[0].outer()[0].get<0>(),left[0].outer()[0].get<1>()));
  boost::geometry::append(right[0],
  Point(right[0].outer()[0].get<0>(),right[0].outer()[0].get<1>()));

  return boost::geometry::equals(left,right);
  */
}

bool isYSymmetric(Polygon poly) {

  // Find centroid
  Point centroid;
  boost::geometry::centroid(poly, centroid);
  double centroidY = centroid.get<1>();

  // Create Bottom and Top bounding boxes
  Box bb;
  boost::geometry::envelope(poly, bb);

  Box bbBottom(bb.min_corner(), Point(bb.max_corner().get<0>(), centroidY));
  Box bbTop(Point(bb.min_corner().get<0>(), centroidY), bb.max_corner());

  // Create Bottom and Top polygons
  MultiPolygon bottom;
  MultiPolygon top;

  boost::geometry::intersection(poly, bbTop, top);
  boost::geometry::intersection(poly, bbBottom, bottom);

  // Mirror top polygon across the centroid
  top = mirrorY(top, centroidY);

  MultiPolygon intersection;

  boost::geometry::intersection(bottom, top, intersection);

  return (isEqual(boost::geometry::area(bottom), boost::geometry::area(intersection), 1e-4));

  /* Weird stuff needed for equals statement...
   * Turns out the equals statement has a pretty tight tolerance
   *
  boost::geometry::unique(bottom);
  boost::geometry::unique(top);

  boost::geometry::append(bottom[0],
  Point(bottom[0].outer()[0].get<0>(),bottom[0].outer()[0].get<1>()));
  boost::geometry::append(top[0], Point(top[0].outer()[0].get<0>(),top[0].outer()[0].get<1>()));

  return boost::geometry::equals(top,bottom);
  */
}

Polygon symmetricUnit(Polygon poly) {
  MultiPolygon symPolys;
  symPolys.push_back(poly);

  Box bb;
  boost::geometry::envelope(poly, bb);

  bool isXsymm = isXSymmetric(poly);
  bool isYsymm = isYSymmetric(poly);

  if (isXsymm) {
    // Find centroid
    Point centroid{};
    boost::geometry::centroid(poly, centroid);
    double centroidX = centroid.get<0>();

    // Create Right bounding box
    Box bbRight(Point(centroidX, bb.min_corner().get<1>()), bb.max_corner());

    // Create Right polygon
    MultiPolygon xSymPolys;
    Polygon right;
    boost::geometry::convert(bbRight, right);
    boost::geometry::intersection(symPolys[0], right, xSymPolys);
    symPolys = xSymPolys;
  }

  if (isYsymm) {
    // Find centroid
    Point centroid{};
    boost::geometry::centroid(poly, centroid);
    double centroidY = centroid.get<1>();

    // Create Top bounding box
    Box bbTop(Point(bb.min_corner().get<0>(), centroidY), bb.max_corner());

    // Create Top polygon
    MultiPolygon ySymPolys;
    Polygon top;
    boost::geometry::convert(bbTop, top);
    boost::geometry::intersection(symPolys[0], top, ySymPolys);
    symPolys = ySymPolys;
  }

  return symPolys[0];
}
#endif

double getXmax(Polygon poly, std::size_t vertex) {
  double x = poly.outer()[vertex].get<0>();
  double xNext;
  std::size_t nV = poly.outer().size();

  if (vertex == nV - 1) {
    xNext = poly.outer()[0].get<0>();
  } else {
    xNext = poly.outer()[vertex + 1].get<0>();
  }

  return std::max(x, xNext);
}

double getYmax(Polygon poly, std::size_t vertex) {
  double y = poly.outer()[vertex].get<1>();
  double yNext;
  std::size_t nV = poly.outer().size();

  if (vertex == nV - 1) {
    yNext = poly.outer()[0].get<1>();
  } else {
    yNext = poly.outer()[vertex + 1].get<1>();
  }

  return std::max(y, yNext);
}

double getXmin(Polygon poly, std::size_t vertex) {
  double x = poly.outer()[vertex].get<0>();
  double xNext;
  std::size_t nV = poly.outer().size();

  if (vertex == nV - 1) {
    xNext = poly.outer()[0].get<0>();
  } else {
    xNext = poly.outer()[vertex + 1].get<0>();
  }

  return std::min(x, xNext);
}

double getYmin(Polygon poly, std::size_t vertex) {
  double y = poly.outer()[vertex].get<1>();
  double yNext;
  std::size_t nV = poly.outer().size();

  if (vertex == nV - 1) {
    yNext = poly.outer()[0].get<1>();
  } else {
    yNext = poly.outer()[vertex + 1].get<1>();
  }

  return std::min(y, yNext);
}

bool comparePointsX(Point first, Point second) { return (first.get<0>() < second.get<0>()); }

bool comparePointsY(Point first, Point second) { return (first.get<1>() < second.get<1>()); }

bool pointOnPoly(Point point, Polygon poly) { return boost::geometry::intersects(point, poly); }

bool isConvex(Polygon poly) {
  Polygon hull;
  boost::geometry::convex_hull(poly, hull);
  return boost::geometry::equals(poly, hull);
}

double getDistance(Point a, Point b) {
  double ax = a.get<0>();
  double ay = a.get<1>();
  double bx = b.get<0>();
  double by = b.get<1>();
  double x = bx - ax;
  double y = by - ay;
  return sqrt(x * x + y * y);
}

double getAngle(Point a, Point b, Point c) {
  double angle;
  // angle at point b
  double A = getDistance(a, b);
  double B = getDistance(b, c);
  double C = getDistance(c, a);

  double ax = a.get<0>();
  double ay = a.get<1>();
  double bx = b.get<0>();
  double by = b.get<1>();
  double cx = c.get<0>();
  double cy = c.get<1>();

  double sign = (bx - ax) * (cy - ay) - (cx - ax) * (by - ay);

  angle = acos((A * A + B * B - C * C) / (2 * A * B));

  if (sign < 0)
    angle += PI;

  return angle;
}

} // namespace Kiva

#endif /* GEOMETRY_CPP_ */
