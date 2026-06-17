/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef SURFACE_H_
#define SURFACE_H_

// Standard
#include <vector>
#include <array>
#include <memory>
#include <string>

namespace Penumbra {

typedef std::vector<float> Polygon; // Implicit list of 3D coordinates

class SurfaceImplementation;

class Surface {
  friend class PenumbraImplementation;

public:
  Surface();
  explicit Surface(const Polygon &polygon, const std::string &name = "");
  Surface(const Surface &surface);
  ~Surface();
  void add_hole(const Polygon &hole); // Defined in counter-clockwise order when facing front

private:
  std::shared_ptr<SurfaceImplementation> surface;
};

} // namespace Penumbra
#endif // SURFACE_H_
