/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <algorithm>

// Penumbra
#include "model.h"

#ifdef __APPLE__
#define glGenVertexArraysX glGenVertexArraysAPPLE
#define glBindVertexArrayX glBindVertexArrayAPPLE
#define glDeleteVertexArraysX glDeleteVertexArraysAPPLE
#else
#define glGenVertexArraysX glGenVertexArrays
#define glBindVertexArrayX glBindVertexArray
#define glDeleteVertexArraysX glDeleteVertexArrays
#endif

namespace Penumbra {

SurfaceBuffer::SurfaceBuffer(GLuint begin, GLuint count, GLint index)
    : begin(begin), count(count), index(index) {}

void GLModel::clear_model() {
  if (objects_set) {
    glDeleteVertexArraysX(1, &vertex_array_object);
    glDeleteBuffers(1, &vertex_buffer_object);
  }
  surface_buffers.clear();
}

void GLModel::set_vertices(const std::vector<float> &vertices) {

  vertex_array = vertices;
  number_of_points = static_cast<unsigned int>(vertices.size()) / vertex_size;
  // Set up vertex array object
  glGenVertexArraysX(1, &vertex_array_object);
  glBindVertexArrayX(vertex_array_object);

  // Set up array buffer to store vertex information
  glGenBuffers(1, &vertex_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(sizeof(float) * vertices.size()), &vertices[0],
               GL_STATIC_DRAW);

  // Set drawing pointers for current vertex buffer
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);

  objects_set = true;
}

void GLModel::set_surface_buffers(const std::vector<SurfaceBuffer> &surface_buffers_in) {
  this->surface_buffers = surface_buffers_in;
}

void GLModel::draw_surface(SurfaceBuffer surface_buffer) {
  glDrawArrays(GL_TRIANGLES, static_cast<GLint>(surface_buffer.begin),
               static_cast<GLsizei>(surface_buffer.count));
}

void GLModel::draw_all() const {
  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(number_of_points));
}

void GLModel::draw_except(std::vector<SurfaceBuffer> hidden_surfaces) const {

  if (hidden_surfaces.empty()) { // draw all if no hidden surfaces
    draw_all();
    return;
  }

  // Sort vector
  std::sort(
      hidden_surfaces.begin(), hidden_surfaces.end(),
      [](const SurfaceBuffer &a, const SurfaceBuffer &b) -> bool { return a.begin > b.begin; });

  // Begin (if first hidden surface isn't first surface)
  if (hidden_surfaces[0].begin != 0u) {
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(hidden_surfaces[0].begin));
  }

  GLuint nextBegin = hidden_surfaces[0].begin + hidden_surfaces[0].count;

  // Loop through all exceptions
  for (std::size_t i = 1; i < hidden_surfaces.size(); ++i) {
    if (nextBegin == number_of_points) {
      // This is the last surface
      return;
    }
    if (nextBegin == hidden_surfaces[i].begin) {
      // Next surface is also hidden
      nextBegin = hidden_surfaces[i].begin + hidden_surfaces[i].count;
      break;
    }
    glDrawArrays(GL_TRIANGLES, static_cast<GLsizei>(nextBegin),
                 static_cast<GLsizei>(hidden_surfaces[i + 1].begin - 1));
  }

  if (nextBegin < number_of_points) {
    glDrawArrays(GL_TRIANGLES, static_cast<GLsizei>(nextBegin),
                 static_cast<GLsizei>(number_of_points - nextBegin));
  }
}

} // namespace Penumbra
