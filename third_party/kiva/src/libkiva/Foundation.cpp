/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#include "Foundation.hpp"
#include "Errors.hpp"

namespace Kiva {

static const double PI = 4.0 * atan(1.0);

Material::Material() : conductivity{0}, density{0}, specificHeat{0} {}

Material::Material(double k, double rho, double cp)
    : conductivity(k), density(rho), specificHeat(cp) {}

InputBlock::InputBlock() : x(0.0), z(0.0), width(0.0), depth(0.0), box(Point(0, 0), Point(0, 0)) {}

double Wall::totalWidth() {
  double width = 0.0;

  for (size_t n = 0; n < layers.size(); n++)
    width += layers[n].thickness;

  return width;
}

double Wall::totalResistance() {
  double R = 0.0;

  for (size_t n = 0; n < layers.size(); n++)
    R += (layers[n].thickness / layers[n].material.conductivity);

  return R;
}

double Slab::totalWidth() {
  double width = 0.0;

  for (size_t n = 0; n < layers.size(); n++)
    width += layers[n].thickness;

  return width;
}

double Slab::totalResistance() {
  double R = 0.0;

  for (size_t n = 0; n < layers.size(); n++)
    R += (layers[n].thickness / layers[n].material.conductivity);

  return R;
}

Mesh::Mesh()
    : minCellDim(0.02), maxNearGrowthCoeff(1.5), maxDepthGrowthCoeff(1.5),
      maxInteriorGrowthCoeff(1.5), maxExteriorGrowthCoeff(1.5) {}

void Block::setSquarePolygon() {
  polygon.outer().push_back(Point(xMin, yMin));
  polygon.outer().push_back(Point(xMin, yMax));
  polygon.outer().push_back(Point(xMax, yMax));
  polygon.outer().push_back(Point(xMax, yMin));
}

void Surface::setSquarePolygon() {
  polygon.outer().push_back(Point(xMin, yMin));
  polygon.outer().push_back(Point(xMin, yMax));
  polygon.outer().push_back(Point(xMax, yMax));
  polygon.outer().push_back(Point(xMax, yMin));
}

void Surface::calcTilt() {
  if (orientation == Surface::Z_POS) {
    tilt = 0;
    cosTilt = 1.0;
  } else if (orientation == Surface::Z_NEG) {
    tilt = PI;
    cosTilt = -1.0;
  } else {
    tilt = PI / 2.0;
    cosTilt = 0.0;
  }
}

inline bool compareRanges(RangeType first, RangeType second) {
  return (first.range.first < second.range.first);
}

bool Ranges::isType(double position, RangeType::Type type) {
  // find specific Range
  for (std::size_t r = 0; r < ranges.size(); r++) {
    if (isGreaterThan(position, ranges[r].range.first) &&
        isLessOrEqual(position, ranges[r].range.second)) {
      if (ranges[r].type == type)
        return true;
    }
  }
  return false;
}

Foundation::Foundation()
    : deepGroundDepth(40.0), farFieldWidth(40.0), foundationDepth(0.0), orientation(0.0),
      deepGroundBoundary(DGB_ZERO_FLUX), wallTopInteriorTemperature{0},
      wallTopExteriorTemperature{0}, wallTopBoundary(WTB_ZERO_FLUX),
      soil(Material(1.73, 1842, 419)), grade(SurfaceProperties(0.9, 0.9, 0.03)),
      coordinateSystem(CS_CARTESIAN), numberOfDimensions(2), useSymmetry(true),
      reductionStrategy(RS_BOUNDARY), twoParameters{false}, reductionLength1{0},
      reductionLength2{0}, linearAreaMultiplier{0}, isXSymm{false}, isYSymm{false},
      exposedFraction(1.0), useDetailedExposedPerimeter(false), buildingHeight(0.0), hasWall(true),
      hasSlab(true), perimeterSurfaceWidth(0.0), hasPerimeterSurface(false), mesh(Mesh()),
      numericalScheme(NS_ADI), fADI(0.00001), tolerance(1.0e-6),
      maxIterations(100000), netArea{0}, netPerimeter{0} {}

void Foundation::createMeshData() {
  std::size_t nV = polygon.outer().size();

  for (std::size_t v = 0; v < nV; v++) {
    double thisX = polygon.outer()[v].get<0>();
    double thisY = polygon.outer()[v].get<1>();
    double nextX, nextY;

    if (v < nV - 1) {
      nextX = polygon.outer()[v + 1].get<0>();
      nextY = polygon.outer()[v + 1].get<1>();
    } else {
      nextX = polygon.outer()[0].get<0>();
      nextY = polygon.outer()[0].get<1>();
    }

    Polygon3 poly;
    poly.outer().push_back(Point3(thisX, thisY, 0.0));
    poly.outer().push_back(Point3(thisX, thisY, buildingHeight));
    poly.outer().push_back(Point3(nextX, nextY, buildingHeight));
    poly.outer().push_back(Point3(nextX, nextY, 0.0));
    buildingSurfaces.push_back(poly);
  }

  if (!isCounterClockWise(polygon)) {
    if (useDetailedExposedPerimeter) {
      showMessage(MSG_ERR,
                  "Foundation floor polygon was entered as clockwise using detailed exposed "
                  "perimeter. Unable to automatically reassign perimeter exposures.");
    }
    boost::geometry::correct(polygon);
    showMessage(
        MSG_WARN,
        "Foundation floor polygon was modified to be counterclockwise as required in Kiva.");
  }

  Material air;
  air.conductivity = 0.02587;
  air.density = 1.275;
  air.specificHeat = 1007;

  // Meshing info
  Interval zeroThickness;
  zeroThickness.maxGrowthCoeff = 1.0;
  zeroThickness.minCellDim = 1.0;
  zeroThickness.growthDir = Interval::UNIFORM;

  Interval near;
  near.maxGrowthCoeff = mesh.maxNearGrowthCoeff;
  near.minCellDim = mesh.minCellDim;
  near.growthDir = Interval::CENTERED;

  Interval deep;
  deep.maxGrowthCoeff = mesh.maxDepthGrowthCoeff;
  deep.minCellDim = mesh.minCellDim;
  deep.growthDir = Interval::BACKWARD;

  Interval minInterior;
  minInterior.maxGrowthCoeff = mesh.maxInteriorGrowthCoeff;
  minInterior.minCellDim = mesh.minCellDim;
  minInterior.growthDir = Interval::BACKWARD;

  Interval midInterior;
  midInterior.maxGrowthCoeff = mesh.maxInteriorGrowthCoeff;
  midInterior.minCellDim = mesh.minCellDim;
  midInterior.growthDir = Interval::CENTERED;

  Interval minExterior;
  minExterior.maxGrowthCoeff = mesh.maxExteriorGrowthCoeff;
  minExterior.minCellDim = mesh.minCellDim;
  minExterior.growthDir = Interval::BACKWARD;

  Interval maxExterior;
  maxExterior.maxGrowthCoeff = mesh.maxExteriorGrowthCoeff;
  maxExterior.minCellDim = mesh.minCellDim;
  maxExterior.growthDir = Interval::FORWARD;

  // Set misc. "Z" dimensions (relative to grade)
  double zGrade = 0.0;
  double zMax = hasWall ? wall.heightAboveGrade : zGrade;
  double zMin = -deepGroundDepth;
  double zSlab = zMax - foundationDepth;
  double zSlabBottom = hasSlab ? (zSlab - slab.totalWidth()) : zSlab;
  double zWall = hasWall ? (zSlabBottom - wall.depthBelowSlab) : zGrade;

  // Set misc. "X/Y" dimensions (relative to wall interior)

  double xyWallInterior = 0.0;
  double xyWallExterior = hasWall ? wall.totalWidth() : xyWallInterior;
  double xyPerimeterSurface = hasPerimeterSurface ? -perimeterSurfaceWidth : xyWallInterior;
  double xySlabNear = xyPerimeterSurface;
  double xyGradeNear = xyWallExterior;

  // Process input blocks

  // Surface list
  std::vector<Surface> surf2D;

  MultiPolygon intBoxes;
  MultiPolygon extBoxes;

  double xyNearMin = xyWallInterior;
  double xyNearMax = xyWallExterior;
  double zNearMin = zGrade;

  double xyWallTopInterior = xyWallInterior;
  double xyWallTopExterior = xyWallExterior;

  for (auto &b : inputBlocks) {
    double bZ1 = zMax - b.z;
    double bX1 = b.x;
    double bZ2 = bZ1 - b.depth;
    double bX2 = bX1 + b.width;
    double bZmin = std::min(bZ1, bZ2);
    double bZmax = std::max(bZ1, bZ2);
    double bXmin = std::min(bX1, bX2);
    double bXmax = std::max(bX1, bX2);

    if (bZmin < zMin) {
      showMessage(MSG_ERR, "'Material Block' cannot be below the deep ground boundary.");
    }
    if (bZmax > zMax) {
      showMessage(MSG_ERR, "'Material Block' cannot be above the wall top.");
    }

    // Blocks along the top may be part of a wall.
    if (isEqual(bZmax, zMax)) {
      if (isGreaterOrEqual(bXmax, xyWallTopInterior) && isLessThan(bXmin, xyWallTopInterior)) {
        // Block is likely part of wall if it's narrow relative to wall or if there is any
        // foundation depth
        if (std::abs(b.width) <= wall.totalWidth() / 2. || foundationDepth > 0.0) {
          xyWallTopInterior = std::min(bXmin, xyWallTopInterior);
          if (foundationDepth == 0.0) {
            xySlabNear = xyWallTopInterior;
          }
        }
      }
      if (isLessOrEqual(bXmin, xyWallTopExterior) && isGreaterThan(bXmax, xyWallTopExterior)) {
        // Block is likely part of wall if it's narrow relative to wall or if the wall top is above
        // grade
        if (std::abs(b.width) <= wall.totalWidth() / 2. ||
            (hasWall && wall.heightAboveGrade > 0.0)) {
          xyWallTopExterior = std::max(bXmax, xyWallTopExterior);
          // Also move the grade surface since part of it is now the wall top
          if (hasWall && wall.heightAboveGrade == 0.0 && isLessOrEqual(bZmin, zGrade)) {
            xyGradeNear = xyWallTopExterior;
          }
        }
      }
    }

    zNearMin = std::min(zNearMin, bZmin);
    xyNearMin = std::min(xyNearMin, bXmin);
    xyNearMax = std::max(xyNearMax, bXmax);

    b.box = Box(Point(bXmin, bZmin), Point(bXmax, bZmax));

    if (bZmax > zSlab && bXmin < xyWallInterior) {
      boost::geometry::union_(b.box, intBoxes, intBoxes);
    }
    if (bZmax > zGrade && bXmax > xyWallExterior) {
      boost::geometry::union_(b.box, extBoxes, extBoxes);
    }
  }

  // Determine interior block surfaces
  if (isLessThan(zSlab, zMax)) {
    // Interior bounding Box
    Box intBound(Point(xyNearMin - 1.0, zSlab), Point(xyWallInterior, zMax));

    MultiPolygon diff;
    boost::geometry::difference(intBound, intBoxes, diff);
    if (diff.size() != 1) {
      showMessage(MSG_ERR, "'Material Blocks' cannot create an enclosure.");
    }
    if (diff[0].inners().size() > 0) {
      showMessage(MSG_ERR, "'Material Blocks' must touch an existing boundary.");
    }

    int numTops = 0;

    // Anything less than a foot (0.3m) away from the wall is still considered part of the wall
    double xWallMin = xyWallTopInterior - 0.3;

    std::size_t nVs = diff[0].outer().size();
    for (std::size_t v = 0; v < nVs; v++) {

      std::size_t vNext;
      if (v == nVs - 1) {
        vNext = 0;
      } else {
        vNext = v + 1;
      }

      double x1 = diff[0].outer()[v].get<0>();
      double x2 = diff[0].outer()[vNext].get<0>();
      double z1 = diff[0].outer()[v].get<1>();
      double z2 = diff[0].outer()[vNext].get<1>();

      geom::Direction dOut = getDirectionOut(diff[0], v);
      geom::Direction dIn = getDirectionIn(diff[0], v);

      if (dOut == geom::Y_POS && isEqual(x1, xyNearMin - 1.0)) {
        // left boundary
        continue;
      }
      if (dOut == geom::X_POS && isEqual(z1, zMax)) {
        // top boundary
        if (dIn != geom::X_POS) {
          numTops++;
        }
        continue;
      }

      Surface surf;
      surf.boundaryConditionType = Surface::INTERIOR_FLUX;

      switch (dOut) {

      case geom::Y_NEG: { // down (Most likely interior wall. Unlikely, but could be slab if the
                          // floor is stepped.)

        surf.orientation = Surface::X_NEG;

        surf.zMin = z2;
        surf.zMax = z1;
        surf.xMin = x2;
        surf.xMax = x1;

        if (x1 > xWallMin || isEqual(z1, zMax)) {
          surf.type = Surface::ST_WALL_INT;
          surf.propPtr = &wall.interior;
          if (isEqual(z1, zMax)) {
            xyWallTopInterior = x1;
          }
          xWallMin = x1 - 0.3;
        } else {
          if (isLessOrEqual(x1, xyPerimeterSurface)) {
            surf.type = Surface::ST_SLAB_CORE;
          } else {
            surf.type = Surface::ST_SLAB_PERIM;
          }
          surf.propPtr = &slab.interior;
        }
      } break;

      case geom::X_NEG: { // left (most likely interior slab, but could be top of a wall surface)

        surf.orientation = Surface::Z_POS;

        if (x2 > xWallMin) {
          // Ends close to the wall, must be top of part of the wall (bump out)
          surf.type = Surface::ST_WALL_INT;
          surf.propPtr = &wall.interior;
          surf.xMin = x2;
          surf.xMax = x1;
        } else {
          if (isLessOrEqual(x1, xyPerimeterSurface)) {
            if (isEqual(x2, xyNearMin - 1.0)) {
              xySlabNear = x1;
              // Slab surface will be created separately
              continue;
            }
            surf.xMin = x2;
            surf.xMax = x1;
            surf.type = Surface::ST_SLAB_CORE;
          } else if (isGreaterThan(x2, xyPerimeterSurface)) {
            surf.xMin = x2;
            surf.xMax = x1;
            surf.type = Surface::ST_SLAB_PERIM;
          } else {
            surf.type = Surface::ST_SLAB_CORE;
            surf.xMin = x2;
            surf.xMax = xyPerimeterSurface;

            Surface surf2;
            surf2.boundaryConditionType = Surface::INTERIOR_FLUX;
            surf2.orientation = Surface::Z_POS;
            surf2.zMin = z2;
            surf2.zMax = z2;
            surf2.xMin = xyPerimeterSurface;
            surf2.xMax = x1;
            surf2.type = Surface::ST_SLAB_PERIM;
            surf2.propPtr = &slab.interior;

            surf2D.push_back(surf2);

            if (isEqual(x2, xyNearMin - 1.0)) {
              xySlabNear = xyPerimeterSurface;
              // Slab surface will be created separately
              continue;
            }
          }
          surf.propPtr = &slab.interior;
        }

        surf.zMin = z2;
        surf.zMax = z1;
      } break;

      case geom::Y_POS: { // up (very unlikely -- assume it's part of the slab)

        surf.orientation = Surface::X_POS;

        surf.zMin = z1;
        surf.zMax = z2;
        surf.xMin = x2;
        surf.xMax = x1;

        if (isLessOrEqual(x1, xyPerimeterSurface)) {
          surf.type = Surface::ST_SLAB_CORE;
        } else {
          surf.type = Surface::ST_SLAB_PERIM;
        }
        surf.propPtr = &slab.interior;
      } break;

      case geom::X_POS: { // right (unlikely to be anything but part of the wall)

        surf.orientation = Surface::Z_NEG;

        surf.xMin = x1;
        surf.xMax = x2;
        surf.type = Surface::ST_WALL_INT;

        surf.zMin = z2;
        surf.zMax = z1;
        surf.propPtr = &wall.interior;
      } break;
      }

      surf2D.push_back(surf);
    }

    if (numTops > 1) {
      showMessage(MSG_ERR, "'Material Blocks' must touch the slab, wall, or grade boundary.");
    }
  } else {
    // Add surface for perimeter for slabs (zSlab == zMax) if necessary
    if (xyPerimeterSurface < xyWallTopInterior) {
      Surface surf;
      surf.boundaryConditionType = Surface::INTERIOR_FLUX;
      surf.orientation = Surface::Z_POS;
      surf.zMin = zSlab;
      surf.zMax = zMax;
      surf.xMin = xyPerimeterSurface;
      surf.xMax = xyWallTopInterior;
      surf.type = Surface::ST_SLAB_PERIM;
      surf.propPtr = &slab.interior;

      surf2D.push_back(surf);

      xySlabNear = xyPerimeterSurface;
    }
  }

  // Determine exterior block surfaces
  if (isLessThan(zGrade, zMax)) {
    // Exterior bounding Box
    Box extBound(Point(xyWallExterior, zGrade), Point(xyNearMax + 1.0, zMax));

    MultiPolygon diff;
    boost::geometry::difference(extBound, extBoxes, diff);
    if (diff.size() != 1) {
      showMessage(MSG_ERR, "'Material Blocks' cannot create an enclosure.");
    }
    if (diff[0].inners().size() > 0) {
      showMessage(MSG_ERR, "'Material Blocks' must touch an existing boundary.");
    }

    int numTops = 0;

    // Anything less than a foot (0.3m) away from the wall is still considered part of the wall
    double xWallMin = xyWallTopExterior + 0.3;

    std::size_t nVs = diff[0].outer().size();
    for (std::size_t v = 0; v < nVs; v++) {

      std::size_t vNext;
      if (v == nVs - 1) {
        vNext = 0;
      } else {
        vNext = v + 1;
      }

      double x1 = diff[0].outer()[v].get<0>();
      double x2 = diff[0].outer()[vNext].get<0>();
      double z1 = diff[0].outer()[v].get<1>();
      double z2 = diff[0].outer()[vNext].get<1>();

      geom::Direction dOut = getDirectionOut(diff[0], v);
      geom::Direction dIn = getDirectionIn(diff[0], v);

      if (dOut == geom::Y_NEG && isEqual(x1, xyNearMax + 1.0)) {
        // right boundary
        continue;
      }
      if (dOut == geom::X_POS && isEqual(z1, zMax)) {
        // top boundary
        if (dIn != geom::X_POS) {
          numTops++;
        }
        continue;
      }
      if (dOut == geom::X_NEG && isEqual(x1, xyNearMax + 1.0)) {
        // grade near
        xyGradeNear = x2;
        continue;
      }

      Surface surf;
      surf.boundaryConditionType = Surface::EXTERIOR_FLUX;

      switch (dOut) {

      case geom::Y_NEG: { // down (very unlikely -- assume it's part of the grade)

        surf.orientation = Surface::X_NEG;

        surf.zMin = z2;
        surf.zMax = z1;
        surf.xMin = x2;
        surf.xMax = x1;
        surf.type = Surface::ST_GRADE;
        surf.propPtr = &grade;
      } break;

      case geom::X_NEG: { // left (most likely exterior grade, but could be top of a wall surface)

        surf.orientation = Surface::Z_POS;

        if (x1 > xWallMin) {
          surf.type = Surface::ST_GRADE;
          surf.propPtr = &grade;
        } else {
          surf.type = Surface::ST_WALL_EXT;
          surf.propPtr = &wall.exterior;
        }

        surf.xMin = x2;
        surf.xMax = x1;
        surf.zMin = z2;
        surf.zMax = z1;
      } break;

      case geom::Y_POS: { // up (Most likely exterior wall. Unlikely, but could be grade if the
                          // ground is stepped.)

        surf.orientation = Surface::X_POS;

        surf.zMin = z1;
        surf.zMax = z2;
        surf.xMin = x2;
        surf.xMax = x1;

        if (x1 < xWallMin || isEqual(z2, zMax)) {
          surf.type = Surface::ST_WALL_EXT;
          surf.propPtr = &wall.exterior;
          if (isEqual(z2, zMax)) {
            xyWallTopExterior = x1;
          }
        } else {
          surf.type = Surface::ST_GRADE;
          surf.propPtr = &grade;
        }
      } break;

      case geom::X_POS: { // right (unlikely to be anything but part of the wall)

        surf.orientation = Surface::Z_NEG;

        surf.xMin = x1;
        surf.xMax = x2;
        surf.type = Surface::ST_WALL_EXT;
        surf.propPtr = &wall.exterior;

        surf.zMin = z2;
        surf.zMax = z1;
      } break;
      }

      surf2D.push_back(surf);
    }

    if (numTops > 1) {
      showMessage(MSG_ERR, "'Material Blocks' must touch the slab, wall, or grade boundary.");
    }
  }

  double zNearDeep = std::min({zGrade, zSlab, zSlabBottom, zWall, zNearMin});
  double xyNearInt = std::min({xyWallInterior, xyPerimeterSurface, xyNearMin});
  double xyNearExt = std::max({xyWallExterior, xyNearMax});

  // Set depending on N dimensions, coordinate system, and 2D appoximation method
  double xMin, xMax, yMin, yMax;

  Ranges xRanges;
  Ranges yRanges;
  Ranges zRanges;

  RangeType zDeepRange;
  zDeepRange.range.first = zMin;
  zDeepRange.range.second = zNearDeep;
  zDeepRange.type = RangeType::DEEP;

  zRanges.ranges.push_back(zDeepRange);

  RangeType zNearRange;
  zNearRange.range.first = zNearDeep;
  zNearRange.range.second = zMax;
  zNearRange.type = RangeType::NEAR;

  zRanges.ranges.push_back(zNearRange);

  // Calculate area and perimeter
  double area = boost::geometry::area(polygon);           // [m2] Area of foundation
  double perimeter = boost::geometry::perimeter(polygon); // [m] Perimeter of foundation

  double interiorPerimeter = 0.0;

  if (useDetailedExposedPerimeter) {
    for (std::size_t v = 0; v < nV; v++) {
      std::size_t vNext;
      if (v == nV - 1) {
        vNext = 0;
      } else {
        vNext = v + 1;
      }

      Point a = polygon.outer()[v];
      Point b = polygon.outer()[vNext];

      if (!isExposedPerimeter[v]) {
        interiorPerimeter += getDistance(a, b);
      }
    }
  } else {
    interiorPerimeter = perimeter * (1.0 - exposedFraction);
  }

  netArea = area;
  netPerimeter = (perimeter - interiorPerimeter);
  exposedFraction = netPerimeter / perimeter;

  // Set 3D foundation areas (for calculation of total heat transfer rates)

  Box boundingBox;
  boost::geometry::envelope(polygon, boundingBox);

  double xMinBB = boundingBox.min_corner().get<0>();
  double yMinBB = boundingBox.min_corner().get<1>();

  double xMaxBB = boundingBox.max_corner().get<0>();
  double yMaxBB = boundingBox.max_corner().get<1>();

  xMin = xMinBB - farFieldWidth;
  yMin = yMinBB - farFieldWidth;

  xMax = xMaxBB + farFieldWidth;
  yMax = yMaxBB + farFieldWidth;

  // Initialize to zero
  surfaceAreas[Surface::ST_FAR_FIELD_AIR] = 0.0;
  surfaceAreas[Surface::ST_FAR_FIELD] = 0.0;
  surfaceAreas[Surface::ST_DEEP_GROUND] = 0.0;
  surfaceAreas[Surface::ST_SLAB_CORE] = 0.0;
  surfaceAreas[Surface::ST_GRADE] = 0.0;
  surfaceAreas[Surface::ST_TOP_AIR_INT] = 0.0;
  surfaceAreas[Surface::ST_TOP_AIR_EXT] = 0.0;
  surfaceAreas[Surface::ST_WALL_TOP] = 0.0;
  surfaceAreas[Surface::ST_SLAB_PERIM] = 0.0;
  surfaceAreas[Surface::ST_WALL_INT] = 0.0;
  surfaceAreas[Surface::ST_WALL_EXT] = 0.0;

  // Add surface-by-surface
  if (isGreaterThan(zMax, 0.0)) {
    surfaceAreas[Surface::ST_FAR_FIELD_AIR] +=
        (yMax - yMin) * (zMax - zGrade) * 2 + (xMax - xMin) * (zMax - zGrade) * 2;
  }
  surfaceAreas[Surface::ST_FAR_FIELD] +=
      (yMax - yMin) * (zGrade - zMin) * 2 + (xMax - xMin) * (zGrade - zMin) * 2;
  surfaceAreas[Surface::ST_DEEP_GROUND] += (yMax - yMin) * (xMax - xMin);
  surfaceAreas[Surface::ST_SLAB_CORE] += boost::geometry::area(offset(polygon, xySlabNear));
  {
    Polygon poly;
    boost::geometry::convert(boundingBox, poly);
    Polygon inner = offset(polygon, xyGradeNear);

    Ring ring;
    boost::geometry::convert(inner, ring);
    boost::geometry::reverse(ring);
    poly.inners().push_back(ring);

    surfaceAreas[Surface::ST_GRADE] += boost::geometry::area(poly);
  }
  surfaceAreas[Surface::ST_TOP_AIR_INT] +=
      boost::geometry::area(offset(polygon, xyWallTopInterior));
  if (zMax > zGrade) {
    Polygon poly;
    boost::geometry::convert(boundingBox, poly);
    Polygon inner = offset(polygon, xyWallTopExterior);

    Ring ring;
    boost::geometry::convert(inner, ring);
    boost::geometry::reverse(ring);
    poly.inners().push_back(ring);

    surfaceAreas[Surface::ST_TOP_AIR_EXT] += boost::geometry::area(poly);
  }
  if (hasWall) {
    Polygon poly = offset(polygon, xyWallTopExterior);

    Polygon temp;
    temp = offset(polygon, xyWallTopInterior);
    Ring ring;
    boost::geometry::convert(temp, ring);
    boost::geometry::reverse(ring);

    poly.inners().push_back(ring);

    surfaceAreas[Surface::ST_WALL_TOP] += boost::geometry::area(poly);
  }

  // General surfaces
  for (auto &s : surf2D) {
    if (s.orientation == Surface::X_POS || s.orientation == Surface::X_NEG) {
      Polygon poly;
      poly = offset(polygon, s.xMin);

      auto nV_poly = poly.outer().size();
      for (std::size_t v = 0; v < nV_poly; v++) {
        std::size_t vNext;
        if (v == nV_poly - 1) {
          vNext = 0;
        } else {
          vNext = v + 1;
        }

        Point a = poly.outer()[v];
        Point b = poly.outer()[vNext];
        surfaceAreas[s.type] += (s.zMax - s.zMin) * getDistance(a, b) * exposedFraction;
      }

    } else {

      Polygon poly = offset(polygon, s.xMax);

      Polygon temp = offset(polygon, s.xMin);
      Ring ring;
      boost::geometry::convert(temp, ring);
      boost::geometry::reverse(ring);

      poly.inners().push_back(ring);

      surfaceAreas[s.type] += boost::geometry::area(poly);
    }
  }

  for (auto &s : surfaceAreas) {
    hasSurface[s.first] = s.second > 0.0;
  }

  if (isEqual(netPerimeter, 0.0)) {
    // TODO: 1D (i.e., cases with no exposed perimeter)

    reductionStrategy = RS_AP;

    numberOfDimensions = 1;
    xMin = 0.0;
    xMax = 1.0;

    yMin = 0.0;
    yMax = 1.0;

    // SURFACES and BLOCKS
    {
      Surface surface;
      surface.type = Surface::ST_DEEP_GROUND;
      surface.xMin = 0.0;
      surface.xMax = 1.0;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zMin;
      if (deepGroundBoundary == DGB_FIXED_TEMPERATURE) {
        surface.boundaryConditionType = Surface::CONSTANT_TEMPERATURE;
      } else {
        surface.boundaryConditionType = Surface::ZERO_FLUX;
      }
      surface.orientation = Surface::Z_NEG;
      surfaces.push_back(surface);
    }

    // Slab
    {
      Surface surface;
      surface.type = Surface::ST_SLAB_CORE;
      surface.xMin = 0.0;
      surface.xMax = 1.0;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zSlab;
      surface.zMax = zSlab;
      surface.boundaryConditionType = Surface::INTERIOR_FLUX;
      surface.orientation = Surface::Z_POS;
      surface.propPtr = &slab.interior;
      surfaces.push_back(surface);
    }

    if (foundationDepth > 0.0) {
      // Interior Air Top Surface
      Surface surface;
      surface.type = Surface::ST_TOP_AIR_INT;
      surface.xMin = 0.0;
      surface.xMax = 1.0;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zMax;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::INTERIOR_TEMPERATURE;
      surface.orientation = Surface::Z_POS;
      surfaces.push_back(surface);
    }

    // BLOCKS

    // Indoor Air
    {
      Block block;
      block.material = air;
      block.blockType = Block::INTERIOR_AIR;
      block.xMin = 0.0;
      block.xMax = 1.0;
      block.yMin = 0.0;
      block.yMax = 1.0;
      block.setSquarePolygon();
      block.zMin = zSlab;
      block.zMax = zMax;
      blocks.push_back(block);
    }

    if (hasSlab) {
      // Foundation Slab
      double zPosition = zSlabBottom;

      for (size_t n = 0; n < slab.layers.size(); n++) {
        Block block;
        block.material = slab.layers[n].material;
        block.blockType = Block::SOLID;
        block.xMin = 0.0;
        block.xMax = 1.0;
        block.yMin = 0.0;
        block.yMax = 1.0;
        block.setSquarePolygon();
        block.zMin = zPosition;
        block.zMax = zPosition + slab.layers[n].thickness;
        blocks.push_back(block);
        zPosition = block.zMax;
      }
    }
  }

  if (numberOfDimensions == 2) {

    // TODO: 2D

    linearAreaMultiplier = 1.0;

    double ap = area / (perimeter - interiorPerimeter);

    if (reductionStrategy == RS_AP) {
      twoParameters = false;
      if (coordinateSystem == CS_CYLINDRICAL) {
        reductionLength2 = 2.0 * ap;
      } else if (coordinateSystem == CS_CARTESIAN) {
        reductionLength2 = ap;
      }
    } else if (reductionStrategy == RS_RR) {
      twoParameters = false;
      double rrA = (perimeter - sqrt(perimeter * perimeter - 4 * PI * area)) / PI;
      double rrB = (perimeter - PI * rrA) * 0.5;
      reductionLength2 = (rrA)*0.5;
      linearAreaMultiplier = rrB;
    } else if (reductionStrategy != RS_CUSTOM) {
      showMessage(MSG_ERR, "Invalid two-dimensional transformation strategy.");
    }

    xMin = 0.0;
    xMax = reductionLength2 + farFieldWidth;

    yMin = 0.0;
    yMax = 1.0;

    double xRef2 = reductionLength2;
    double xRef1 = reductionLength1;

    // SURFACES and BLOCKS

    // Symmetry Surface
    {
      Surface surface;
      surface.type = Surface::ST_SYMMETRY;
      surface.xMin = xMin;
      surface.xMax = xMin;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      if (twoParameters)
        surface.zMax = zGrade;
      else
        surface.zMax = zSlab;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::X_NEG;
      surfaces.push_back(surface);
    }

    // Interior Air Symmetry Temperature
    if (!twoParameters) {
      Surface surface;
      surface.type = Surface::ST_SYMMETRY_AIR;
      surface.xMin = xMin;
      surface.xMax = xMin;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zSlab;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::INTERIOR_TEMPERATURE;
      surface.orientation = Surface::X_NEG;
      surfaces.push_back(surface);
    }

    // Far Field Surface
    {
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMax;
      surface.xMax = xMax;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::X_POS;
      surfaces.push_back(surface);
    }

    // Exterior Air Right Surface
    {
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD_AIR;
      surface.xMin = xMax;
      surface.xMax = xMax;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zGrade;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
      surface.orientation = Surface::X_POS;
      surfaces.push_back(surface);
    }

    // Deep ground surface
    {
      Surface surface;
      surface.type = Surface::ST_DEEP_GROUND;
      surface.xMin = xMin;
      surface.xMax = xMax;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zMin;
      if (deepGroundBoundary == DGB_FIXED_TEMPERATURE) {
        surface.boundaryConditionType = Surface::CONSTANT_TEMPERATURE;
      } else {
        surface.boundaryConditionType = Surface::ZERO_FLUX;
      }
      surface.orientation = Surface::Z_NEG;
      surfaces.push_back(surface);
    }

    // Slab
    {
      Surface surface;
      surface.type = Surface::ST_SLAB_CORE;
      if (!twoParameters) {
        surface.xMin = xMin;
        surface.xMax = xRef2 + xySlabNear;
      } else {
        surface.xMin = xRef1 - xySlabNear;
        surface.xMax = xRef2 + xySlabNear;
      }
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zSlab;
      surface.zMax = zSlab;
      surface.boundaryConditionType = Surface::INTERIOR_FLUX;
      surface.orientation = Surface::Z_POS;
      surface.propPtr = &slab.interior;
      surfaces.push_back(surface);
    }

    // Grade
    {
      Surface surface;
      surface.type = Surface::ST_GRADE;
      surface.xMin = xRef2 + xyGradeNear;
      surface.xMax = xMax;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zGrade;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::EXTERIOR_FLUX;
      surface.orientation = Surface::Z_POS;
      surface.propPtr = &grade;
      surfaces.push_back(surface);
    }
    if (twoParameters) {
      Surface surface;
      surface.type = Surface::ST_GRADE;
      surface.xMin = xMin;
      surface.xMax = xRef1 - xyGradeNear;
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zGrade;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::EXTERIOR_FLUX;
      surface.orientation = Surface::Z_POS;
      surface.propPtr = &grade;
      surfaces.push_back(surface);
    }

    if (foundationDepth > 0.0) {
      // Interior Air Top Surface
      Surface surface;
      surface.type = Surface::ST_TOP_AIR_INT;
      if (!twoParameters) {
        surface.xMin = xMin;
        surface.xMax = xRef2 + xyWallTopInterior;
      } else {
        surface.xMin = xRef1 - xyWallTopInterior;
        surface.xMax = xRef2 + xyWallTopInterior;
      }
      surface.yMin = 0.0;
      surface.yMax = 1.0;
      surface.setSquarePolygon();
      surface.zMin = zMax;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::INTERIOR_TEMPERATURE;
      surface.orientation = Surface::Z_POS;
      surfaces.push_back(surface);
    }

    if (zMax > zGrade) {
      {
        // Exterior Air Top Surface
        Surface surface;
        surface.type = Surface::ST_TOP_AIR_EXT;
        surface.xMin = xRef2 + xyWallTopExterior;
        surface.xMax = xMax;
        surface.yMin = 0.0;
        surface.yMax = 1.0;
        surface.setSquarePolygon();
        surface.zMin = zMax;
        surface.zMax = zMax;
        surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
        surface.orientation = Surface::Z_POS;
        surfaces.push_back(surface);
      }
      if (twoParameters) {
        // Exterior Air Top Surface
        Surface surface;
        surface.type = Surface::ST_TOP_AIR_EXT;
        surface.xMin = xMin;
        surface.xMax = xRef1 - xyWallTopExterior;
        surface.yMin = 0.0;
        surface.yMax = 1.0;
        surface.setSquarePolygon();
        surface.zMin = zMax;
        surface.zMax = zMax;
        surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
        surface.orientation = Surface::Z_POS;
        surfaces.push_back(surface);
      }
    }

    if (wallTopBoundary == WTB_LINEAR_DT) {
      // Wall Top
      if (hasWall) {
        {
          double position = 0.0;
          double &Tin = wallTopInteriorTemperature;
          double &Tout = wallTopExteriorTemperature;
          double Tdiff = (Tin - Tout);
          std::size_t N = (std::size_t)((xyWallTopExterior - xyWallTopInterior + DBL_EPSILON) /
                                        mesh.minCellDim);
          double temperature = Tin - (1.0 / N) / 2 * Tdiff;

          for (std::size_t n = 1; n <= N; n++) {
            Surface surface;
            surface.type = Surface::ST_WALL_TOP;
            surface.xMin = xRef2 + position;
            surface.xMax = xRef2 + position + (xyWallTopExterior - xyWallTopInterior) / N;
            surface.yMin = 0.0;
            surface.yMax = 1.0;
            surface.setSquarePolygon();
            surface.zMin = zMax;
            surface.zMax = zMax;
            surface.boundaryConditionType = Surface::CONSTANT_TEMPERATURE;
            surface.orientation = Surface::Z_POS;
            surface.temperature = temperature;
            surfaces.push_back(surface);

            position += (xyWallTopExterior - xyWallTopInterior) / N;
            temperature -= (1.0 / N) * Tdiff;
          }
        }

        if (twoParameters) {
          double position = 0.0;
          double &Tin = wallTopInteriorTemperature;
          double &Tout = wallTopExteriorTemperature;
          double Tdiff = (Tin - Tout);
          std::size_t N = (std::size_t)((xyWallTopExterior - xyWallTopInterior + DBL_EPSILON) /
                                        mesh.minCellDim);
          double temperature = Tin - (1.0 / N) / 2 * Tdiff;

          for (std::size_t n = 1; n <= N; n++) {
            Surface surface;
            surface.type = Surface::ST_WALL_TOP;
            surface.xMin = xRef1 - position - (xyWallTopExterior - xyWallTopInterior) / N;
            surface.xMax = xRef1 - position;
            surface.yMin = 0.0;
            surface.yMax = 1.0;
            surface.setSquarePolygon();
            surface.zMin = zMax;
            surface.zMax = zMax;
            surface.boundaryConditionType = Surface::CONSTANT_TEMPERATURE;
            surface.orientation = Surface::Z_POS;
            surface.temperature = temperature;
            surfaces.push_back(surface);

            position += (xyWallTopExterior - xyWallTopInterior) / N;
            temperature -= (1.0 / N) * Tdiff;
          }
        }
      }
    } else {
      // Wall Top
      if (hasWall) {
        {
          Surface surface;
          surface.type = Surface::ST_WALL_TOP;
          surface.xMin = xRef2 + xyWallTopInterior;
          surface.xMax = xRef2 + xyWallTopExterior;
          surface.yMin = 0.0;
          surface.yMax = 1.0;
          surface.setSquarePolygon();
          surface.zMin = zMax;
          surface.zMax = zMax;
          surface.boundaryConditionType = Surface::ZERO_FLUX;
          surface.orientation = Surface::Z_POS;
          surfaces.push_back(surface);
        }
        if (twoParameters) {
          Surface surface;
          surface.type = Surface::ST_WALL_TOP;
          surface.xMin = xRef1 - xyWallTopExterior;
          surface.xMax = xRef1 - xyWallTopInterior;
          surface.yMin = 0.0;
          surface.yMax = 1.0;
          surface.setSquarePolygon();
          surface.zMin = zMax;
          surface.zMax = zMax;
          surface.boundaryConditionType = Surface::ZERO_FLUX;
          surface.orientation = Surface::Z_POS;
          surfaces.push_back(surface);
        }
      }
    }

    // General surfaces
    for (auto &s : surf2D) {
      {
        Surface surface;
        surface.type = s.type;
        surface.xMin = xRef2 + s.xMin;
        surface.xMax = xRef2 + s.xMax;
        surface.yMin = 0.0;
        surface.yMax = 1.0;
        surface.setSquarePolygon();
        surface.zMin = s.zMin;
        surface.zMax = s.zMax;
        surface.boundaryConditionType = s.boundaryConditionType;
        surface.orientation = s.orientation;
        surface.propPtr = s.propPtr;

        surfaces.push_back(surface);
      }
      if (twoParameters) {
        Surface surface;
        surface.type = s.type;
        surface.xMin = xRef1 - s.xMin;
        surface.xMax = xRef1 - s.xMax;
        surface.yMin = 0.0;
        surface.yMax = 1.0;
        surface.setSquarePolygon();
        surface.zMin = s.zMin;
        surface.zMax = s.zMax;
        surface.boundaryConditionType = s.boundaryConditionType;
        surface.orientation = s.orientation;
        if (s.orientation == Surface::X_POS) {
          surface.orientation = Surface::X_NEG;
        } else if (s.orientation == Surface::X_NEG) {
          surface.orientation = Surface::X_POS;
        }
        surface.propPtr = s.propPtr;

        surfaces.push_back(surface);
      }
    }

    // BLOCKS

    // Indoor Air
    {
      Block block;
      block.material = air;
      block.blockType = Block::INTERIOR_AIR;
      if (twoParameters) {
        block.xMin = xRef1 - xyWallInterior;
        block.xMax = xRef2 + xyWallInterior;
      } else {
        block.xMin = xMin;
        block.xMax = xRef2 + xyWallInterior;
      }
      block.yMin = 0.0;
      block.yMax = 1.0;
      block.setSquarePolygon();
      block.zMin = zSlab;
      block.zMax = zMax;
      blocks.push_back(block);
    }

    // Exterior Air
    {
      Block block;
      block.material = air;
      block.blockType = Block::EXTERIOR_AIR;
      block.xMin = xRef2 + xyWallExterior;
      block.xMax = xMax;
      block.yMin = 0.0;
      block.yMax = 1.0;
      block.setSquarePolygon();
      block.zMin = zGrade;
      block.zMax = zMax;
      blocks.push_back(block);
    }
    if (twoParameters) {
      Block block;
      block.material = air;
      block.blockType = Block::EXTERIOR_AIR;
      block.xMin = xMin;
      block.xMax = xRef1 - xyWallExterior;
      block.yMin = 0.0;
      block.yMax = 1.0;
      block.setSquarePolygon();
      block.zMin = zGrade;
      block.zMax = zMax;
      blocks.push_back(block);
    }

    if (hasSlab) {
      // Foundation Slab
      double zPosition = zSlabBottom;

      for (size_t n = 0; n < slab.layers.size(); n++) {
        Block block;
        block.material = slab.layers[n].material;
        block.blockType = Block::SOLID;
        if (!twoParameters) {
          block.xMin = xMin;
          block.xMax = xRef2;
        } else {
          block.xMin = xRef1;
          block.xMax = xRef2;
        }
        block.yMin = 0.0;
        block.yMax = 1.0;
        block.setSquarePolygon();
        block.zMin = zPosition;
        block.zMax = zPosition + slab.layers[n].thickness;
        blocks.push_back(block);
        zPosition = block.zMax;
      }
    }

    if (hasWall) {
      {
        double xPosition = xRef2;

        // Foundation Wall
        for (size_t n = wall.layers.size(); n > 0; n--) {
          size_t index = n - 1;
          Block block;
          block.material = wall.layers[index].material;
          block.blockType = Block::SOLID;
          block.xMin = xPosition;
          block.xMax = xPosition + wall.layers[index].thickness;
          block.yMin = 0.0;
          block.yMax = 1.0;
          block.setSquarePolygon();
          block.zMin = zWall;
          block.zMax = zMax;
          xPosition = block.xMax;
          blocks.push_back(block);
        }
      }

      if (twoParameters) {
        double xPosition = xRef1;

        // Foundation Wall
        for (size_t n = wall.layers.size(); n > 0; n--) {
          size_t index = n - 1;
          Block block;
          block.material = wall.layers[index].material;
          block.blockType = Block::SOLID;
          block.xMin = xPosition - wall.layers[index].thickness;
          block.xMax = xPosition;
          block.yMin = 0.0;
          block.yMax = 1.0;
          block.setSquarePolygon();
          block.zMin = zWall;
          block.zMax = zMax;
          xPosition = block.xMin;
          blocks.push_back(block);
        }
      }
    }

    // General blocks
    for (auto &b : inputBlocks) {
      {
        Block block;
        block.material = b.material;
        block.blockType = Block::SOLID;
        block.xMin = xRef2 + b.box.min_corner().get<0>();
        block.xMax = xRef2 + b.box.max_corner().get<0>();
        block.yMin = 0.0;
        block.yMax = 1.0;
        block.setSquarePolygon();
        block.zMin = b.box.min_corner().get<1>();
        block.zMax = b.box.max_corner().get<1>();
        blocks.push_back(block);
      }

      if (twoParameters) {
        Block block;
        block.material = b.material;
        block.blockType = Block::SOLID;
        block.xMin = xRef1 - b.box.min_corner().get<0>();
        block.xMax = xRef1 - b.box.max_corner().get<0>();
        block.yMin = 0.0;
        block.yMax = 1.0;
        block.setSquarePolygon();
        block.zMin = b.box.min_corner().get<1>();
        block.zMax = b.box.max_corner().get<1>();
        blocks.push_back(block);
      }
    }

    // Set range types

    if (!twoParameters) {
      RangeType xInteriorRange;
      xInteriorRange.range.first = xMin;
      xInteriorRange.range.second = xRef2 + xyNearInt;
      xInteriorRange.type = RangeType::MIN_INTERIOR;
      xRanges.ranges.push_back(xInteriorRange);

      RangeType xNearRange;
      xNearRange.range.first = xRef2 + xyNearInt;
      xNearRange.range.second = xRef2 + xyNearExt;
      xNearRange.type = RangeType::NEAR;
      xRanges.ranges.push_back(xNearRange);

      RangeType xExteriorRange;
      xExteriorRange.range.first = xRef2 + xyNearExt;
      xExteriorRange.range.second = xMax;
      xExteriorRange.type = RangeType::MAX_EXTERIOR;
      xRanges.ranges.push_back(xExteriorRange);
    } else {
      RangeType xMinExteriorRange;
      xMinExteriorRange.range.first = xMin;
      xMinExteriorRange.range.second = xRef1 - xyNearExt;
      xMinExteriorRange.type = RangeType::MIN_INTERIOR;
      xRanges.ranges.push_back(xMinExteriorRange);

      RangeType xNearRange1;
      xNearRange1.range.first = xRef1 - xyNearExt;
      xNearRange1.range.second = xRef1 - xyNearInt;
      xNearRange1.type = RangeType::NEAR;
      xRanges.ranges.push_back(xNearRange1);

      RangeType xInteriorRange;
      xInteriorRange.range.first = xRef1 - xyNearInt;
      xInteriorRange.range.second = xRef2 + xyNearInt;
      xInteriorRange.type = RangeType::MID_INTERIOR;
      xRanges.ranges.push_back(xInteriorRange);

      RangeType xNearRange2;
      xNearRange2.range.first = xRef2 + xyNearInt;
      xNearRange2.range.second = xRef2 + xyNearExt;
      xNearRange2.type = RangeType::NEAR;
      xRanges.ranges.push_back(xNearRange2);

      RangeType xMaxExteriorRange;
      xMaxExteriorRange.range.first = xRef2 + xyNearExt;
      xMaxExteriorRange.range.second = xMax;
      xMaxExteriorRange.type = RangeType::MAX_EXTERIOR;
      xRanges.ranges.push_back(xMaxExteriorRange);
    }

  } else if (numberOfDimensions == 3 && !useSymmetry) {
#if defined(KIVA_3D)
    // TODO 3D

    xMin = xMinBB - farFieldWidth;
    yMin = yMinBB - farFieldWidth;

    xMax = xMaxBB + farFieldWidth;
    yMax = yMaxBB + farFieldWidth;

    if (isGreaterThan(zMax, 0.0)) {

      // Exterior Air Perimeter Surfaces
      {
        {
          // X Min
          Surface surface;
          surface.type = Surface::ST_FAR_FIELD_AIR;
          surface.xMin = xMin;
          surface.xMax = xMin;
          surface.yMin = yMin;
          surface.yMax = yMax;
          surface.setSquarePolygon();
          surface.zMin = zGrade;
          surface.zMax = zMax;
          surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
          surface.orientation = Surface::X_NEG;
          surfaces.push_back(surface);
        }

        {
          // X Max
          Surface surface;
          surface.type = Surface::ST_FAR_FIELD_AIR;
          surface.xMin = xMax;
          surface.xMax = xMax;
          surface.yMin = yMin;
          surface.yMax = yMax;
          surface.setSquarePolygon();
          surface.zMin = zGrade;
          surface.zMax = zMax;
          surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
          surface.orientation = Surface::X_POS;
          surfaces.push_back(surface);
        }

        {
          // Y Min
          Surface surface;
          surface.type = Surface::ST_FAR_FIELD_AIR;
          surface.xMin = xMin;
          surface.xMax = xMax;
          surface.yMin = yMin;
          surface.yMax = yMin;
          surface.setSquarePolygon();
          surface.zMin = zGrade;
          surface.zMax = zMax;
          surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
          surface.orientation = Surface::Y_NEG;
          surfaces.push_back(surface);
        }

        {
          // Y Max
          Surface surface;
          surface.type = Surface::ST_FAR_FIELD_AIR;
          surface.xMin = xMin;
          surface.xMax = xMax;
          surface.yMin = yMax;
          surface.yMax = yMax;
          surface.setSquarePolygon();
          surface.zMin = zGrade;
          surface.zMax = zMax;
          surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
          surface.orientation = Surface::Y_POS;
          surfaces.push_back(surface);
        }
      }
    }

    // Far Field Surfaces
    {
      // X Min
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMin;
      surface.xMax = xMin;
      surface.yMin = yMin;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::X_NEG;
      surfaces.push_back(surface);
    }

    {
      // X Max
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMax;
      surface.xMax = xMax;
      surface.yMin = yMin;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::X_POS;
      surfaces.push_back(surface);
    }

    {
      // Y Min
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMin;
      surface.xMax = xMax;
      surface.yMin = yMin;
      surface.yMax = yMin;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::Y_NEG;
      surfaces.push_back(surface);
    }

    {
      // Y Max
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMin;
      surface.xMax = xMax;
      surface.yMin = yMax;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::Y_POS;
      surfaces.push_back(surface);
    }

    // Deep ground surface
    {
      Surface surface;
      surface.type = Surface::ST_DEEP_GROUND;
      surface.xMin = xMin;
      surface.xMax = xMax;
      surface.yMin = yMin;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zMin;
      if (deepGroundBoundary == DGB_FIXED_TEMPERATURE) {
        surface.boundaryConditionType = Surface::CONSTANT_TEMPERATURE;
      } else {
        surface.boundaryConditionType = Surface::ZERO_FLUX;
      }
      surface.orientation = Surface::Z_NEG;
      surfaces.push_back(surface);
    }

    // Slab
    {
      Polygon poly;
      poly = offset(polygon, xySlabNear);

      Surface surface;
      surface.type = Surface::ST_SLAB_CORE;
      surface.polygon = poly;
      surface.zMin = zSlab;
      surface.zMax = zSlab;
      surface.boundaryConditionType = Surface::INTERIOR_FLUX;
      surface.orientation = Surface::Z_POS;
      surface.propPtr = &slab.interior;
      surfaces.push_back(surface);
    }

    // Grade
    {
      Polygon poly;
      poly = offset(polygon, xyGradeNear);

      Ring ring;
      boost::geometry::convert(poly, ring);
      boost::geometry::reverse(ring);

      Surface surface;
      surface.type = Surface::ST_GRADE;
      surface.xMin = xMin;
      surface.xMax = xMax;
      surface.yMin = yMin;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.polygon.inners().push_back(ring);
      surface.zMin = zGrade;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::EXTERIOR_FLUX;
      surface.orientation = Surface::Z_POS;
      surface.propPtr = &grade;
      surfaces.push_back(surface);
    }

    if (foundationDepth > 0.0) {
      // Interior Air Top Surface
      Polygon poly;
      poly = offset(polygon, xyWallTopInterior);

      Surface surface;
      surface.type = Surface::ST_TOP_AIR_INT;
      surface.polygon = poly;
      surface.zMin = zMax;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::INTERIOR_TEMPERATURE;
      surface.orientation = Surface::Z_POS;
      surfaces.push_back(surface);
    }

    if (zMax > zGrade) {
      // Exterior Air Top Surface
      {
        Polygon poly;
        poly = offset(polygon, xyWallTopExterior);

        Ring ring;
        boost::geometry::convert(poly, ring);
        boost::geometry::reverse(ring);

        Surface surface;
        surface.type = Surface::ST_TOP_AIR_EXT;
        surface.xMin = xMin;
        surface.xMax = xMax;
        surface.yMin = yMin;
        surface.yMax = yMax;
        surface.setSquarePolygon();
        surface.polygon.inners().push_back(ring);
        surface.zMin = zMax;
        surface.zMax = zMax;
        surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
        surface.orientation = Surface::Z_POS;
        surfaces.push_back(surface);
      }
    }

    if (wallTopBoundary == WTB_LINEAR_DT) {
      // Wall Top
      if (hasWall) {
        double position = 0.0;
        double &Tin = wallTopInteriorTemperature;
        double &Tout = wallTopExteriorTemperature;
        double Tdiff = (Tin - Tout);
        std::size_t N =
            (std::size_t)((xyWallTopExterior - xyWallTopInterior + DBL_EPSILON) / mesh.minCellDim);
        double temperature = Tin - (1.0 / N) / 2 * Tdiff;

        for (std::size_t n = 1; n <= N; n++) {

          Polygon poly;
          poly = offset(polygon, position + (xyWallTopExterior - xyWallTopInterior) / N);

          Polygon temp;
          temp = offset(polygon, position);
          Ring ring;
          boost::geometry::convert(temp, ring);
          boost::geometry::reverse(ring);

          poly.inners().push_back(ring);

          Surface surface;
          surface.type = Surface::ST_WALL_TOP;
          surface.polygon = poly;
          surface.zMin = zMax;
          surface.zMax = zMax;
          surface.boundaryConditionType = Surface::CONSTANT_TEMPERATURE;
          surface.orientation = Surface::Z_POS;
          surface.temperature = temperature;
          surfaces.push_back(surface);

          position += (xyWallTopExterior - xyWallTopInterior) / N;
          temperature -= (1.0 / N) * Tdiff;
        }
      }
    } else {
      // Wall Top
      if (hasWall) {
        Polygon poly;
        poly = offset(polygon, xyWallTopExterior);

        Polygon temp;
        temp = offset(polygon, xyWallTopInterior);
        Ring ring;
        boost::geometry::convert(temp, ring);
        boost::geometry::reverse(ring);

        poly.inners().push_back(ring);

        Surface surface;
        surface.type = Surface::ST_WALL_TOP;
        surface.polygon = poly;
        surface.zMin = zMax;
        surface.zMax = zMax;
        surface.boundaryConditionType = Surface::ZERO_FLUX;
        surface.orientation = Surface::Z_POS;
        surfaces.push_back(surface);
      }
    }

    // General surfaces
    for (auto &s : surf2D) {
      if (s.orientation == Surface::X_POS || s.orientation == Surface::X_NEG) {
        Polygon poly;
        poly = offset(polygon, s.xMin);

        for (std::size_t v = 0; v < nV; v++) {
          Surface surface;
          surface.type = s.type;
          surface.xMin = getXmin(poly, v);
          surface.xMax = getXmax(poly, v);
          surface.yMin = getYmin(poly, v);
          surface.yMax = getYmax(poly, v);
          surface.setSquarePolygon();
          surface.zMin = s.zMin;
          surface.zMax = s.zMax;
          surface.boundaryConditionType = s.boundaryConditionType;
          switch (getDirectionOut(poly, v)) {
          case geom::Y_POS:
            surface.orientation = Surface::X_POS;
            break;
          case geom::X_POS:
            surface.orientation = Surface::Y_NEG;
            break;
          case geom::Y_NEG:
            surface.orientation = Surface::X_NEG;
            break;
          case geom::X_NEG:
            surface.orientation = Surface::Y_POS;
            break;
          }
          surface.propPtr = s.propPtr;
          surfaces.push_back(surface);
        }

      } else {

        Polygon poly;
        poly = offset(polygon, s.xMax);

        Polygon temp;
        temp = offset(polygon, s.xMin);
        Ring ring;
        boost::geometry::convert(temp, ring);
        boost::geometry::reverse(ring);

        poly.inners().push_back(ring);

        Surface surface;
        surface.type = s.type;
        surface.polygon = poly;
        surface.zMin = s.zMin;
        surface.zMax = s.zMax;
        surface.boundaryConditionType = s.boundaryConditionType;
        surface.orientation = s.orientation;
        surface.propPtr = s.propPtr;
        surfaces.push_back(surface);
      }
    }

    // BLOCKS

    if (hasSlab) {
      // Foundation Slab
      double zPosition = zSlabBottom;

      for (size_t n = 0; n < slab.layers.size(); n++) {
        Block block;
        block.material = slab.layers[n].material;
        block.blockType = Block::SOLID;
        block.polygon = polygon;
        block.zMin = zPosition;
        block.zMax = zPosition + slab.layers[n].thickness;
        blocks.push_back(block);
        zPosition = block.zMax;
      }
    }

    // Indoor Air
    {
      Polygon poly;
      poly = offset(polygon, xyWallInterior);

      Block block;
      block.material = air;
      block.blockType = Block::INTERIOR_AIR;
      block.polygon = poly;
      block.zMin = zSlab;
      block.zMax = zMax;
      blocks.push_back(block);
    }

    if (hasWall) {
      double xyPosition = 0.0;

      // Foundation Wall
      for (size_t n = wall.layers.size(); n > 0; n--) {
        size_t index = n - 1;
        Polygon poly;
        poly = offset(polygon, xyPosition + wall.layers[index].thickness);

        Polygon temp;
        temp = offset(polygon, xyPosition);
        Ring ring;
        boost::geometry::convert(temp, ring);
        boost::geometry::reverse(ring);

        poly.inners().push_back(ring);

        Block block;
        block.material = wall.layers[index].material;
        block.blockType = Block::SOLID;
        block.polygon = poly;
        block.zMin = zWall;
        block.zMax = zMax;
        xyPosition += wall.layers[index].thickness;
        blocks.push_back(block);
      }
    }

    // Exterior Air
    {
      Polygon poly;
      poly = offset(polygon, xyWallExterior);

      Ring ring;
      boost::geometry::convert(poly, ring);
      boost::geometry::reverse(ring);

      Block block;
      block.material = air;
      block.blockType = Block::EXTERIOR_AIR;
      block.xMin = xMin;
      block.xMax = xMax;
      block.yMin = yMin;
      block.yMax = yMax;
      block.setSquarePolygon();
      block.polygon.inners().push_back(ring);
      block.zMin = zGrade;
      block.zMax = zMax;
      blocks.push_back(block);
    }

    // General blocks
    for (auto &b : inputBlocks) {
      Polygon poly;
      poly = offset(polygon, b.box.max_corner().get<0>());

      Polygon temp;
      temp = offset(polygon, b.box.min_corner().get<0>());
      Ring ring;
      boost::geometry::convert(temp, ring);
      boost::geometry::reverse(ring);

      poly.inners().push_back(ring);

      Block block;
      block.material = b.material;
      block.blockType = Block::SOLID;
      block.polygon = poly;
      block.zMin = b.box.min_corner().get<1>();
      block.zMax = b.box.max_corner().get<1>();
      blocks.push_back(block);
    }

    // Set x and y near ranges
    std::vector<RangeType> xNearRanges;
    std::vector<RangeType> yNearRanges;

    for (std::size_t v = 0; v < nV; v++) {
      double x = polygon.outer()[v].get<0>();
      double y = polygon.outer()[v].get<1>();

      switch (getDirectionOut(polygon, v)) {
      case geom::Y_POS: {
        RangeType range;
        range.range.first = x - xyNearExt;
        range.range.second = x - xyNearInt;
        range.type = RangeType::NEAR;
        xNearRanges.push_back(range);
      } break;

      case geom::Y_NEG: {
        RangeType range;
        range.range.first = x + xyNearInt;
        range.range.second = x + xyNearExt;
        range.type = RangeType::NEAR;
        xNearRanges.push_back(range);
      } break;
      case geom::X_POS: {
        RangeType range;
        range.range.first = y + xyNearInt;
        range.range.second = y + xyNearExt;
        range.type = RangeType::NEAR;
        yNearRanges.push_back(range);
      } break;
      case geom::X_NEG: {
        RangeType range;
        range.range.first = y - xyNearExt;
        range.range.second = y - xyNearInt;
        range.type = RangeType::NEAR;
        yNearRanges.push_back(range);
      } break;
      }
    }

    // Set X range types
    sort(xNearRanges.begin(), xNearRanges.end(), compareRanges);

    // Merge overlapping ranges
    for (std::size_t r = 1; r < xNearRanges.size(); r++) {
      if (isLessOrEqual(xNearRanges[r].range.first, xNearRanges[r - 1].range.second)) {
        xNearRanges[r - 1].range.second = xNearRanges[r].range.second;
        xNearRanges.erase(xNearRanges.begin() + r - 1);
        r -= 1;
      }
    }

    RangeType xMinExteriorRange;
    xMinExteriorRange.range.first = xMin;
    xMinExteriorRange.range.second = xNearRanges[0].range.first;
    xMinExteriorRange.type = RangeType::MIN_EXTERIOR;
    xRanges.ranges.push_back(xMinExteriorRange);

    for (std::size_t r = 0; r < xNearRanges.size(); r++) {
      if (r == 0) {
        RangeType xNearRange;
        xNearRange.range.first = xNearRanges[r].range.first;
        xNearRange.range.second = xNearRanges[r].range.second;
        xNearRange.type = RangeType::NEAR;
        xRanges.ranges.push_back(xNearRange);
      } else {
        RangeType xInteriorRange;
        xInteriorRange.range.first = xNearRanges[r - 1].range.second;
        xInteriorRange.range.second = xNearRanges[r].range.first;
        xInteriorRange.type = RangeType::MID_INTERIOR;
        xRanges.ranges.push_back(xInteriorRange);

        RangeType xNearRange;
        xNearRange.range.first = xNearRanges[r].range.first;
        xNearRange.range.second = xNearRanges[r].range.second;
        xNearRange.type = RangeType::NEAR;
        xRanges.ranges.push_back(xNearRange);
      }
    }

    RangeType xMaxExteriorRange;
    xMaxExteriorRange.range.first = xNearRanges[xNearRanges.size() - 1].range.second;
    xMaxExteriorRange.range.second = xMax;
    xMaxExteriorRange.type = RangeType::MAX_EXTERIOR;
    xRanges.ranges.push_back(xMaxExteriorRange);

    // Set Y range types
    sort(yNearRanges.begin(), yNearRanges.end(), compareRanges);

    // Merge overlapping ranges
    for (std::size_t r = 1; r < yNearRanges.size(); r++) {
      if (isLessOrEqual(yNearRanges[r].range.first, yNearRanges[r - 1].range.second)) {
        yNearRanges[r - 1].range.second = yNearRanges[r].range.second;
        yNearRanges.erase(yNearRanges.begin() + r - 1);
        r -= 1;
      }
    }

    RangeType yMinExteriorRange;
    yMinExteriorRange.range.first = yMin;
    yMinExteriorRange.range.second = yNearRanges[0].range.first;
    yMinExteriorRange.type = RangeType::MIN_EXTERIOR;
    yRanges.ranges.push_back(yMinExteriorRange);

    for (std::size_t r = 0; r < yNearRanges.size(); r++) {
      if (r == 0) {
        RangeType yNearRange;
        yNearRange.range.first = yNearRanges[r].range.first;
        yNearRange.range.second = yNearRanges[r].range.second;
        yNearRange.type = RangeType::NEAR;
        yRanges.ranges.push_back(yNearRange);
      } else {
        RangeType yInteriorRange;
        yInteriorRange.range.first = yNearRanges[r - 1].range.second;
        yInteriorRange.range.second = yNearRanges[r].range.first;
        yInteriorRange.type = RangeType::MID_INTERIOR;
        yRanges.ranges.push_back(yInteriorRange);

        RangeType yNearRange;
        yNearRange.range.first = yNearRanges[r].range.first;
        yNearRange.range.second = yNearRanges[r].range.second;
        yNearRange.type = RangeType::NEAR;
        yRanges.ranges.push_back(yNearRange);
      }
    }

    RangeType yMaxExteriorRange;
    yMaxExteriorRange.range.first = yNearRanges[yNearRanges.size() - 1].range.second;
    yMaxExteriorRange.range.second = yMax;
    yMaxExteriorRange.type = RangeType::MAX_EXTERIOR;
    yRanges.ranges.push_back(yMaxExteriorRange);

  } else if (numberOfDimensions == 3 && useSymmetry) {
    // TODO: 3D Symmetric
    isXSymm = isXSymmetric(polygon);
    isYSymm = isYSymmetric(polygon);

    // Change polygon to minimum symmetric unit
    Polygon symmetricPoly;
    symmetricPoly = symmetricUnit(polygon);

    nV = symmetricPoly.outer().size();

    if (isXSymm)
      xMin = xMinBB;
    else
      xMin = xMinBB - farFieldWidth;

    if (isYSymm)
      yMin = yMinBB;
    else
      yMin = yMinBB - farFieldWidth;

    xMax = xMaxBB + farFieldWidth;
    yMax = yMaxBB + farFieldWidth;

    Box domainBox(Point(xMin, yMin), Point(xMax, yMax));

    // Symmetry & Far Field Surfaces
    {
      // X Min
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMin;
      surface.xMax = xMin;
      surface.yMin = yMin;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::X_NEG;
      surfaces.push_back(surface);
    }

    {
      // X Max
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMax;
      surface.xMax = xMax;
      surface.yMin = yMin;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::X_POS;
      surfaces.push_back(surface);
    }

    {
      // Y Min
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMin;
      surface.xMax = xMax;
      surface.yMin = yMin;
      surface.yMax = yMin;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::Y_NEG;
      surfaces.push_back(surface);
    }

    {
      // Y Max
      Surface surface;
      surface.type = Surface::ST_FAR_FIELD;
      surface.xMin = xMin;
      surface.xMax = xMax;
      surface.yMin = yMax;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::ZERO_FLUX;
      surface.orientation = Surface::Y_POS;
      surfaces.push_back(surface);
    }

    // Deep ground surface
    {
      Surface surface;
      surface.type = Surface::ST_DEEP_GROUND;
      surface.xMin = xMin;
      surface.xMax = xMax;
      surface.yMin = yMin;
      surface.yMax = yMax;
      surface.setSquarePolygon();
      surface.zMin = zMin;
      surface.zMax = zMin;
      if (deepGroundBoundary == DGB_FIXED_TEMPERATURE) {
        surface.boundaryConditionType = Surface::CONSTANT_TEMPERATURE;
      } else {
        surface.boundaryConditionType = Surface::ZERO_FLUX;
      }
      surface.orientation = Surface::Z_NEG;
      surfaces.push_back(surface);
    }

    // Slab
    {
      Polygon tempPoly;
      tempPoly = offset(polygon, xySlabNear);
      MultiPolygon poly;
      boost::geometry::intersection(domainBox, tempPoly, poly);

      Surface surface;
      surface.type = Surface::ST_SLAB_CORE;
      surface.polygon = poly[0];
      surface.zMin = zSlab;
      surface.zMax = zSlab;
      surface.boundaryConditionType = Surface::INTERIOR_FLUX;
      surface.orientation = Surface::Z_POS;
      surface.propPtr = &slab.interior;
      surfaces.push_back(surface);
    }

    // Grade
    {
      Polygon tempPoly;
      tempPoly = offset(polygon, xyGradeNear);

      MultiPolygon poly;
      boost::geometry::difference(domainBox, tempPoly, poly);

      Surface surface;
      surface.type = Surface::ST_GRADE;
      surface.polygon = poly[0];
      surface.zMin = zGrade;
      surface.zMax = zGrade;
      surface.boundaryConditionType = Surface::EXTERIOR_FLUX;
      surface.orientation = Surface::Z_POS;
      surface.propPtr = &grade;
      surfaces.push_back(surface);
    }

    if (foundationDepth > 0.0) {
      // Interior Air Top Surface
      Polygon tempPoly;
      tempPoly = offset(polygon, xyWallTopInterior);
      MultiPolygon poly;
      boost::geometry::intersection(domainBox, tempPoly, poly);

      Surface surface;
      surface.type = Surface::ST_TOP_AIR_INT;
      surface.polygon = poly[0];
      surface.zMin = zMax;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::INTERIOR_TEMPERATURE;
      surface.orientation = Surface::Z_POS;
      surfaces.push_back(surface);
    }

    if (zMax > zGrade) {
      // Exterior Air Top Surface
      Polygon tempPoly;
      tempPoly = offset(polygon, xyWallTopExterior);

      MultiPolygon poly;
      boost::geometry::difference(domainBox, tempPoly, poly);

      Surface surface;
      surface.type = Surface::ST_TOP_AIR_EXT;
      surface.polygon = poly[0];
      surface.zMin = zMax;
      surface.zMax = zMax;
      surface.boundaryConditionType = Surface::EXTERIOR_TEMPERATURE;
      surface.orientation = Surface::Z_POS;
      surfaces.push_back(surface);
    }

    if (wallTopBoundary == WTB_LINEAR_DT) {
      // Wall Top
      if (hasWall) {
        double position = 0.0;
        double &Tin = wallTopInteriorTemperature;
        double &Tout = wallTopExteriorTemperature;
        double Tdiff = (Tin - Tout);
        std::size_t N =
            (std::size_t)((xyWallTopExterior - xyWallTopInterior + DBL_EPSILON) / mesh.minCellDim);
        double temperature = Tin - (1.0 / N) / 2 * Tdiff;

        for (std::size_t n = 1; n <= N; n++) {

          Polygon tempPoly;
          tempPoly = offset(polygon, position + (xyWallTopExterior - xyWallTopInterior) / N);

          Polygon temp;
          temp = offset(polygon, position);
          Ring ring;
          boost::geometry::convert(temp, ring);
          boost::geometry::reverse(ring);

          tempPoly.inners().push_back(ring);

          MultiPolygon poly;
          boost::geometry::intersection(domainBox, tempPoly, poly);

          Surface surface;
          surface.type = Surface::ST_WALL_TOP;
          surface.polygon = poly[0];
          surface.zMin = zMax;
          surface.zMax = zMax;
          surface.boundaryConditionType = Surface::CONSTANT_TEMPERATURE;
          surface.orientation = Surface::Z_POS;
          surface.temperature = temperature;
          surfaces.push_back(surface);

          position += (xyWallTopExterior - xyWallTopInterior) / N;
          temperature -= (1.0 / N) * Tdiff;
        }
      }
    } else {
      // Wall Top
      if (hasWall) {
        Polygon tempPoly;
        tempPoly = offset(polygon, xyWallTopExterior);

        Polygon temp;
        temp = offset(polygon, xyWallTopInterior);
        Ring ring;
        boost::geometry::convert(temp, ring);
        boost::geometry::reverse(ring);

        tempPoly.inners().push_back(ring);

        MultiPolygon poly;
        boost::geometry::intersection(domainBox, tempPoly, poly);

        Surface surface;
        surface.type = Surface::ST_WALL_TOP;
        surface.polygon = poly[0];
        surface.zMin = zMax;
        surface.zMax = zMax;
        surface.boundaryConditionType = Surface::ZERO_FLUX;
        surface.orientation = Surface::Z_POS;
        surfaces.push_back(surface);
      }
    }

    // General surfaces
    for (auto &s : surf2D) {
      if (s.orientation == Surface::X_POS || s.orientation == Surface::X_NEG) {
        Polygon tempPoly;
        tempPoly = offset(polygon, s.xMin);
        MultiPolygon poly;
        boost::geometry::intersection(domainBox, tempPoly, poly);

        for (std::size_t v = 0; v < nV; v++) {

          if (!((isEqual(getXmin(poly[0], v), xMin) && isEqual(getXmax(poly[0], v), xMin)) ||
                (isEqual(getYmin(poly[0], v), yMin) && isEqual(getYmax(poly[0], v), yMin)))) {

            Surface surface;
            surface.type = s.type;
            surface.xMin = getXmin(poly[0], v);
            surface.xMax = getXmax(poly[0], v);
            surface.yMin = getYmin(poly[0], v);
            surface.yMax = getYmax(poly[0], v);
            surface.setSquarePolygon();
            surface.zMin = s.zMin;
            surface.zMax = s.zMax;
            surface.boundaryConditionType = s.boundaryConditionType;
            switch (getDirectionOut(poly[0], v)) {
            case geom::Y_POS:
              surface.orientation = Surface::X_POS;
              break;
            case geom::X_POS:
              surface.orientation = Surface::Y_NEG;
              break;
            case geom::Y_NEG:
              surface.orientation = Surface::X_NEG;
              break;
            case geom::X_NEG:
              surface.orientation = Surface::Y_POS;
              break;
            }
            surface.propPtr = s.propPtr;
            surfaces.push_back(surface);
          }
        }

      } else {

        Polygon tempPoly;
        tempPoly = offset(polygon, s.xMax);

        Polygon temp;
        temp = offset(polygon, s.xMax);
        Ring ring;
        boost::geometry::convert(temp, ring);
        boost::geometry::reverse(ring);

        tempPoly.inners().push_back(ring);

        MultiPolygon poly;
        boost::geometry::intersection(domainBox, tempPoly, poly);

        poly[0].inners().push_back(ring);

        Surface surface;
        surface.type = s.type;
        surface.polygon = poly[0];
        surface.zMin = s.zMin;
        surface.zMax = s.zMax;
        surface.boundaryConditionType = s.boundaryConditionType;
        surface.orientation = s.orientation;
        surface.propPtr = s.propPtr;
        surfaces.push_back(surface);
      }
    }

    // BLOCKS

    if (hasSlab) {
      // Foundation Slab
      double zPosition = zSlabBottom;

      Polygon tempPoly;
      tempPoly = polygon;

      MultiPolygon poly;
      boost::geometry::intersection(domainBox, tempPoly, poly);

      for (size_t n = 0; n < slab.layers.size(); n++) {
        Block block;
        block.material = slab.layers[n].material;
        block.blockType = Block::SOLID;
        block.polygon = poly[0];
        block.zMin = zPosition;
        block.zMax = zPosition + slab.layers[n].thickness;
        blocks.push_back(block);
        zPosition = block.zMax;
      }
    }

    // Indoor Air
    {
      Polygon tempPoly;
      tempPoly = offset(polygon, xyWallInterior);

      MultiPolygon poly;
      boost::geometry::intersection(domainBox, tempPoly, poly);

      Block block;
      block.material = air;
      block.blockType = Block::INTERIOR_AIR;
      block.polygon = poly[0];
      block.zMin = zSlab;
      block.zMax = zMax;
      blocks.push_back(block);
    }

    if (hasWall) {
      double xyPosition = 0.0;

      // Foundation Wall
      for (size_t n = wall.layers.size(); n > 0; n--) {
        size_t index = n - 1;
        Polygon tempPoly;
        tempPoly = offset(polygon, xyPosition + wall.layers[index].thickness);

        Polygon temp;
        temp = offset(polygon, xyPosition);
        Ring ring;
        boost::geometry::convert(temp, ring);
        boost::geometry::reverse(ring);

        tempPoly.inners().push_back(ring);

        MultiPolygon poly;
        boost::geometry::intersection(domainBox, tempPoly, poly);

        Block block;
        block.material = wall.layers[index].material;
        block.blockType = Block::SOLID;
        block.polygon = poly[0];
        block.zMin = zWall;
        block.zMax = zMax;
        xyPosition += wall.layers[index].thickness;
        blocks.push_back(block);
      }
    }

    // Exterior Air
    {
      Polygon tempPoly;
      tempPoly = offset(polygon, xyWallExterior);

      MultiPolygon poly;
      boost::geometry::difference(domainBox, tempPoly, poly);

      Block block;
      block.material = air;
      block.blockType = Block::EXTERIOR_AIR;
      block.polygon = poly[0];
      block.zMin = zGrade;
      block.zMax = zMax;
      blocks.push_back(block);
    }

    // General blocks
    for (auto &b : inputBlocks) {
      Polygon tempPoly;
      tempPoly = offset(polygon, b.box.max_corner().get<0>());

      Polygon temp;
      temp = offset(polygon, b.box.min_corner().get<0>());
      Ring ring;
      boost::geometry::convert(temp, ring);
      boost::geometry::reverse(ring);

      tempPoly.inners().push_back(ring);

      MultiPolygon poly;
      boost::geometry::intersection(domainBox, tempPoly, poly);

      Block block;
      block.material = b.material;
      block.blockType = Block::SOLID;
      block.polygon = poly[0];
      block.zMin = b.box.min_corner().get<1>();
      block.zMax = b.box.max_corner().get<1>();
      blocks.push_back(block);
    }

    // Set x and y near ranges
    std::vector<RangeType> xNearRanges;
    std::vector<RangeType> yNearRanges;

    for (std::size_t v = 0; v < symmetricPoly.outer().size(); v++) {
      if (!((isEqual(getXmin(symmetricPoly, v), xMin) &&
             isEqual(getXmax(symmetricPoly, v), xMin)) ||
            (isEqual(getYmin(symmetricPoly, v), yMin) &&
             isEqual(getYmax(symmetricPoly, v), yMin)))) {
        double x = symmetricPoly.outer()[v].get<0>();
        double y = symmetricPoly.outer()[v].get<1>();

        switch (getDirectionOut(symmetricPoly, v)) {
        case geom::Y_POS: {
          RangeType range;
          range.range.first = x - xyNearExt;
          range.range.second = x - xyNearInt;
          range.type = RangeType::NEAR;
          xNearRanges.push_back(range);
        } break;

        case geom::Y_NEG: {
          RangeType range;
          range.range.first = x + xyNearInt;
          range.range.second = x + xyNearExt;
          range.type = RangeType::NEAR;
          xNearRanges.push_back(range);
        } break;
        case geom::X_POS: {
          RangeType range;
          range.range.first = y + xyNearInt;
          range.range.second = y + xyNearExt;
          range.type = RangeType::NEAR;
          yNearRanges.push_back(range);
        } break;
        case geom::X_NEG: {
          RangeType range;
          range.range.first = y - xyNearExt;
          range.range.second = y - xyNearInt;
          range.type = RangeType::NEAR;
          yNearRanges.push_back(range);
        } break;
        }
      }
    }

    // Set X range types
    sort(xNearRanges.begin(), xNearRanges.end(), compareRanges);

    // Merge overlapping ranges
    for (std::size_t r = 1; r < xNearRanges.size(); r++) {
      if (isLessOrEqual(xNearRanges[r].range.first, xNearRanges[r - 1].range.second)) {
        xNearRanges[r - 1].range.second = xNearRanges[r].range.second;
        xNearRanges.erase(xNearRanges.begin() + r - 1);
        r -= 1;
      }
    }

    if (isXSymm) {
      RangeType xMinInteriorRange;
      xMinInteriorRange.range.first = xMin;
      xMinInteriorRange.range.second = xNearRanges[0].range.first;
      xMinInteriorRange.type = RangeType::MIN_INTERIOR;
      xRanges.ranges.push_back(xMinInteriorRange);
    } else {
      RangeType xMinExteriorRange;
      xMinExteriorRange.range.first = xMin;
      xMinExteriorRange.range.second = xNearRanges[0].range.first;
      xMinExteriorRange.type = RangeType::MIN_EXTERIOR;
      xRanges.ranges.push_back(xMinExteriorRange);
    }

    for (std::size_t r = 0; r < xNearRanges.size(); r++) {
      if (r == 0) {
        RangeType xNearRange;
        xNearRange.range.first = xNearRanges[r].range.first;
        xNearRange.range.second = xNearRanges[r].range.second;
        xNearRange.type = RangeType::NEAR;
        xRanges.ranges.push_back(xNearRange);
      } else {
        RangeType xInteriorRange;
        xInteriorRange.range.first = xNearRanges[r - 1].range.second;
        xInteriorRange.range.second = xNearRanges[r].range.first;
        xInteriorRange.type = RangeType::MID_INTERIOR;
        xRanges.ranges.push_back(xInteriorRange);

        RangeType xNearRange;
        xNearRange.range.first = xNearRanges[r].range.first;
        xNearRange.range.second = xNearRanges[r].range.second;
        xNearRange.type = RangeType::NEAR;
        xRanges.ranges.push_back(xNearRange);
      }
    }

    RangeType xMaxExteriorRange;
    xMaxExteriorRange.range.first = xNearRanges[xNearRanges.size() - 1].range.second;
    xMaxExteriorRange.range.second = xMax;
    xMaxExteriorRange.type = RangeType::MAX_EXTERIOR;
    xRanges.ranges.push_back(xMaxExteriorRange);

    // Set Y range types
    sort(yNearRanges.begin(), yNearRanges.end(), compareRanges);

    // Merge overlapping ranges
    for (std::size_t r = 1; r < yNearRanges.size(); r++) {
      if (isLessOrEqual(yNearRanges[r].range.first, yNearRanges[r - 1].range.second)) {
        yNearRanges[r - 1].range.second = yNearRanges[r].range.second;
        yNearRanges.erase(yNearRanges.begin() + r - 1);
        r -= 1;
      }
    }

    if (isYSymm) {
      RangeType yMinInteriorRange;
      yMinInteriorRange.range.first = yMin;
      yMinInteriorRange.range.second = yNearRanges[0].range.first;
      yMinInteriorRange.type = RangeType::MIN_INTERIOR;
      yRanges.ranges.push_back(yMinInteriorRange);
    } else {
      RangeType yMinExteriorRange;
      yMinExteriorRange.range.first = yMin;
      yMinExteriorRange.range.second = yNearRanges[0].range.first;
      yMinExteriorRange.type = RangeType::MIN_EXTERIOR;
      yRanges.ranges.push_back(yMinExteriorRange);
    }

    for (std::size_t r = 0; r < yNearRanges.size(); r++) {
      if (r == 0) {
        RangeType yNearRange;
        yNearRange.range.first = yNearRanges[r].range.first;
        yNearRange.range.second = yNearRanges[r].range.second;
        yNearRange.type = RangeType::NEAR;
        yRanges.ranges.push_back(yNearRange);
      } else {
        RangeType yInteriorRange;
        yInteriorRange.range.first = yNearRanges[r - 1].range.second;
        yInteriorRange.range.second = yNearRanges[r].range.first;
        yInteriorRange.type = RangeType::MID_INTERIOR;
        yRanges.ranges.push_back(yInteriorRange);

        RangeType yNearRange;
        yNearRange.range.first = yNearRanges[r].range.first;
        yNearRange.range.second = yNearRanges[r].range.second;
        yNearRange.type = RangeType::NEAR;
        yRanges.ranges.push_back(yNearRange);
      }
    }

    RangeType yMaxExteriorRange;
    yMaxExteriorRange.range.first = yNearRanges[yNearRanges.size() - 1].range.second;
    yMaxExteriorRange.range.second = yMax;
    yMaxExteriorRange.type = RangeType::MAX_EXTERIOR;
    yRanges.ranges.push_back(yMaxExteriorRange);
#else
    showMessage(MSG_ERR, "This version of Kiva cannot calculate 3D heat transfer.");
#endif
  }

  std::vector<double> xPoints;
  std::vector<double> yPoints;
  std::vector<double> zPoints;

  std::vector<double> xSurfaces;
  std::vector<double> ySurfaces;
  std::vector<double> zSurfaces;

  // Create points for mesh data
  for (size_t s = 0; s < surfaces.size(); s++) {
    for (std::size_t v = 0; v < surfaces[s].polygon.outer().size(); v++) {
      // Make sure points are within the domain
      double pointX = std::max(std::min(surfaces[s].polygon.outer()[v].get<0>(), xMax), xMin);
      surfaces[s].polygon.outer()[v].set<0>(pointX);
      double pointY = std::max(std::min(surfaces[s].polygon.outer()[v].get<1>(), yMax), yMin);
      surfaces[s].polygon.outer()[v].set<1>(pointY);

      xPoints.push_back(surfaces[s].polygon.outer()[v].get<0>());
      yPoints.push_back(surfaces[s].polygon.outer()[v].get<1>());
    }
    surfaces[s].zMax = std::max(std::min(surfaces[s].zMax, zMax), zMin);
    surfaces[s].zMin = std::max(std::min(surfaces[s].zMin, zMax), zMin);

    zPoints.push_back(surfaces[s].zMax);
    zPoints.push_back(surfaces[s].zMin);

    if (surfaces[s].orientation == Surface::X_POS || surfaces[s].orientation == Surface::X_NEG) {
      xSurfaces.push_back(surfaces[s].polygon.outer()[0].get<0>());
    } else if (surfaces[s].orientation == Surface::Y_POS ||
               surfaces[s].orientation == Surface::Y_NEG) {
      ySurfaces.push_back(surfaces[s].polygon.outer()[0].get<1>());
    } else // if (surfaces[s].orientation == Surface::Z_POS ||
           //     surfaces[s].orientation == Surface::Z_NEG)
    {
      zSurfaces.push_back(surfaces[s].zMin);
    }
  }

  for (size_t b = 0; b < blocks.size(); b++) {
    for (std::size_t v = 0; v < blocks[b].polygon.outer().size(); v++) {
      // Make sure points are within the domain
      double pointX = std::max(std::min(blocks[b].polygon.outer()[v].get<0>(), xMax), xMin);
      blocks[b].polygon.outer()[v].set<0>(pointX);
      double pointY = std::max(std::min(blocks[b].polygon.outer()[v].get<1>(), yMax), yMin);
      blocks[b].polygon.outer()[v].set<1>(pointY);

      xPoints.push_back(blocks[b].polygon.outer()[v].get<0>());
      yPoints.push_back(blocks[b].polygon.outer()[v].get<1>());
    }
    blocks[b].zMax = std::max(std::min(blocks[b].zMax, zMax), zMin);
    blocks[b].zMin = std::max(std::min(blocks[b].zMin, zMax), zMin);

    zPoints.push_back(blocks[b].zMax);
    zPoints.push_back(blocks[b].zMin);
  }

  // Sort the points vectors
  sort(xPoints.begin(), xPoints.end());
  sort(yPoints.begin(), yPoints.end());
  sort(zPoints.begin(), zPoints.end());

  // erase duplicate elements
  for (size_t i = 1; i < xPoints.size(); i++) {
    if (isEqual(xPoints[i], xPoints[i - 1])) {
      xPoints.erase(xPoints.begin() + i);
      i -= 1;
    }
  }

  for (size_t j = 1; j < yPoints.size(); j++) {
    if (isEqual(yPoints[j], yPoints[j - 1])) {
      yPoints.erase(yPoints.begin() + j);
      j -= 1;
    }
  }

  for (size_t k = 1; k < zPoints.size(); k++) {
    if (isEqual(zPoints[k], zPoints[k - 1])) {
      zPoints.erase(zPoints.begin() + k);
      k -= 1;
    }
  }

  // Sort the surfaces vectors
  sort(xSurfaces.begin(), xSurfaces.end());
  sort(ySurfaces.begin(), ySurfaces.end());
  sort(zSurfaces.begin(), zSurfaces.end());

  // erase (approximately) duplicate elements
  for (size_t i = 1; i < xSurfaces.size(); i++) {
    if (isEqual(xSurfaces[i], xSurfaces[i - 1])) {
      xSurfaces.erase(xSurfaces.begin() + i);
      i -= 1;
    }
  }

  for (size_t j = 1; j < ySurfaces.size(); j++) {
    if (isEqual(ySurfaces[j], ySurfaces[j - 1])) {
      ySurfaces.erase(ySurfaces.begin() + j);
      j -= 1;
    }
  }

  for (size_t k = 1; k < zSurfaces.size(); k++) {
    if (isEqual(zSurfaces[k], zSurfaces[k - 1])) {
      zSurfaces.erase(zSurfaces.begin() + k);
      k -= 1;
    }
  }

  // re-add the extra surface elements to create zero-thickness cells
  for (size_t i = 0; i < xSurfaces.size(); i++) {
    xPoints.push_back(xSurfaces[i]);
  }

  for (size_t j = 0; j < ySurfaces.size(); j++) {
    yPoints.push_back(ySurfaces[j]);
  }

  for (size_t k = 0; k < zSurfaces.size(); k++) {
    zPoints.push_back(zSurfaces[k]);
  }

  // Sort the range vectors again this time it includes doubles for zero thickness cells
  sort(xPoints.begin(), xPoints.end());
  sort(yPoints.begin(), yPoints.end());
  sort(zPoints.begin(), zPoints.end());

  xMeshData.points = xPoints;
  yMeshData.points = yPoints;
  zMeshData.points = zPoints;

  std::vector<Interval> xIntervals;
  std::vector<Interval> yIntervals;
  std::vector<Interval> zIntervals;

  // Make sure ranges are within the domain
  for (size_t r = 0; r < xRanges.ranges.size(); r++) {
    xRanges.ranges[r].range.first = std::max(std::min(xRanges.ranges[r].range.first, xMax), xMin);
    xRanges.ranges[r].range.second = std::max(std::min(xRanges.ranges[r].range.second, xMax), xMin);
  }
  for (size_t r = 0; r < yRanges.ranges.size(); r++) {
    yRanges.ranges[r].range.first = std::max(std::min(yRanges.ranges[r].range.first, yMax), yMin);
    yRanges.ranges[r].range.second = std::max(std::min(yRanges.ranges[r].range.second, yMax), yMin);
  }
  for (size_t r = 0; r < zRanges.ranges.size(); r++) {
    zRanges.ranges[r].range.first = std::max(std::min(zRanges.ranges[r].range.first, zMax), zMin);
    zRanges.ranges[r].range.second = std::max(std::min(zRanges.ranges[r].range.second, zMax), zMin);
  }

  for (size_t i = 1; i < xPoints.size(); i++) {
    if (isEqual(xPoints[i], xPoints[i - 1]))
      xIntervals.push_back(zeroThickness);
    else if (xRanges.isType(xPoints[i], RangeType::MIN_INTERIOR))
      xIntervals.push_back(minInterior);
    else if (xRanges.isType(xPoints[i], RangeType::MID_INTERIOR))
      xIntervals.push_back(midInterior);
    else if (xRanges.isType(xPoints[i], RangeType::NEAR))
      xIntervals.push_back(near);
    else if (xRanges.isType(xPoints[i], RangeType::MIN_EXTERIOR))
      xIntervals.push_back(minExterior);
    else if (xRanges.isType(xPoints[i], RangeType::MAX_EXTERIOR))
      xIntervals.push_back(maxExterior);
  }

  for (size_t j = 1; j < yPoints.size(); j++) {
    if (isEqual(yPoints[j], yPoints[j - 1]))
      yIntervals.push_back(zeroThickness);
    else if (yRanges.isType(yPoints[j], RangeType::MIN_INTERIOR))
      yIntervals.push_back(minInterior);
    else if (yRanges.isType(yPoints[j], RangeType::MID_INTERIOR))
      yIntervals.push_back(midInterior);
    else if (yRanges.isType(yPoints[j], RangeType::NEAR))
      yIntervals.push_back(near);
    else if (yRanges.isType(yPoints[j], RangeType::MIN_EXTERIOR))
      yIntervals.push_back(minExterior);
    else if (yRanges.isType(yPoints[j], RangeType::MAX_EXTERIOR))
      yIntervals.push_back(maxExterior);
  }

  for (size_t k = 1; k < zPoints.size(); k++) {
    if (isEqual(zPoints[k], zPoints[k - 1]))
      zIntervals.push_back(zeroThickness);
    else if (zRanges.isType(zPoints[k], RangeType::DEEP))
      zIntervals.push_back(deep);
    else if (zRanges.isType(zPoints[k], RangeType::NEAR))
      zIntervals.push_back(near);
  }

  xMeshData.intervals = xIntervals;
  yMeshData.intervals = yIntervals;
  zMeshData.intervals = zIntervals;
}

} // namespace Kiva
