/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef MODEL_H_
#define MODEL_H_

// Standard
#include <vector>

// Vendor
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Penumbra {

class SurfaceBuffer {
public:
  explicit SurfaceBuffer(GLuint begin = 0u, GLuint count = 0u, GLint index = -1);
  GLuint begin;
  GLuint count;
  GLint index;
};

class GLModel {
public:
  GLModel() = default;
  ~GLModel() = default;
  void set_vertices(const std::vector<float> &vertices);
  void set_surface_buffers(const std::vector<SurfaceBuffer> &surface_buffers);
  static void draw_surface(SurfaceBuffer surface_buffer);
  void draw_all() const;
  void draw_except(std::vector<SurfaceBuffer> hidden_surfaces) const;
  void clear_model();
  std::vector<float> vertex_array;
  std::vector<SurfaceBuffer> surface_buffers;
  unsigned int number_of_points{0u};
  static const int vertex_size{3}; // i.e., 3D
private:
  GLuint vertex_buffer_object{}, vertex_array_object{};
  bool objects_set{false};
};

} // namespace Penumbra

#endif // MODEL_H_
