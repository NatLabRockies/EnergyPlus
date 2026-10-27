#include "Vector2.hh"
#include <algorithm>
#include <limits>

using namespace ObjexxFCL;

struct BoundingBox2D
{
  double min_x = std::numeric_limits<double>::max();
  double min_y = std::numeric_limits<double>::max();
  double max_x = std::numeric_limits<double>::lowest();
  double max_y = std::numeric_limits<double>::lowest();

  /// add a point to the BoundingBox
  void addPoint(const Vector2& point) {
    min_x = std::min(min_x, point.x);
    min_y = std::min(min_y, point.y);
    max_x = std::max(max_x, point.x);
    max_y = std::max(max_y, point.y);
  }

  bool contains(const Vector2& point) const {
    return (point.x >= min_x && point.x <= max_x && point.y >= min_y && point.y <= max_y);
  }
};
