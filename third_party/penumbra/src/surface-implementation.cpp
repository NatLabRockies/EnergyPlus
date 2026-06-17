/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <vector>
#include <array>

// Penumbra
#include <penumbra/surface.h>
#include <penumbra/logging.h>
#include "surface-implementation.h"

namespace Penumbra {

TessData::TessData(const float *array, unsigned number_of_vertices)
    : number_of_vertices(number_of_vertices) {
  vertices.insert(vertices.end(), (const float *)array, (const float *)array + number_of_vertices);
}

SurfaceImplementation::SurfaceImplementation(Polygon polygon) : polygon(std::move(polygon)) {}

TessData SurfaceImplementation::tessellate() {
  TESStesselator *tess = tessNewTess(nullptr);

  if (!tess) {
    throw PenumbraException(std::format("Unable to create tessellator for surface, \"{}\".", name),
                            *logger);
  }

  // Add primary polygon
  tessAddContour(tess, TessData::polygon_size, &polygon[0], sizeof(float) * TessData::vertex_size,
                 (int)polygon.size() / TessData::vertex_size);

  // Add holes
  for (auto &hole : holes) {
    tessAddContour(tess, TessData::polygon_size, &hole[0], sizeof(float) * TessData::vertex_size,
                   (int)hole.size() / TessData::vertex_size);
  }

  if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, TessData::polygon_size,
                     TessData::vertex_size, nullptr)) {
    throw PenumbraException(std::format("Unable to tessellate surface, \"{}\".", name), *logger);
  }

  // For now convert to glDrawArrays() style of vertices, sometime may change to glDrawElements
  // (with element buffers)
  std::vector<float> vertex_array;
  const TESSreal *vertices = tessGetVertices(tess);
  const int number_of_elements = tessGetElementCount(tess);
  const TESSindex *elements = tessGetElements(tess);
  for (int i = 0; i < number_of_elements * TessData::polygon_size; ++i) {
    const int vertex = *(elements + i);
    for (int j = 0; j < TessData::vertex_size; ++j) {
      vertex_array.push_back(vertices[vertex * TessData::vertex_size + j]);
    }
  }

  TessData data(&vertex_array[0], static_cast<unsigned int>(vertex_array.size()));

  tessDeleteTess(tess);

  return data;
}

} // namespace Penumbra
