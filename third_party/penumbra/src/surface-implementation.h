/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef SURFACE_IMPLEMENTATION_H_
#define SURFACE_IMPLEMENTATION_H_

// Standard
#include <vector>
#include <array>

// Vendor
#include <tesselator.h>
#include <courierr/courierr.h>

namespace Penumbra {

struct TessData {
  TessData(const float *array, unsigned int number_of_vertices);
  std::vector<float> vertices;
  unsigned int number_of_vertices;
  static const int polygon_size{3}; // making triangles
  static const int vertex_size{3};  // i.e., 3D
};

class SurfaceImplementation {
public:
  SurfaceImplementation() = default;
  explicit SurfaceImplementation(Polygon polygon);
  TessData tessellate();
  Polygon polygon;
  std::vector<Polygon> holes;
  std::shared_ptr<Courierr::Courierr> logger;
  std::string name;
};

} // namespace Penumbra
#endif // SURFACE_IMPLEMENTATION_H_
