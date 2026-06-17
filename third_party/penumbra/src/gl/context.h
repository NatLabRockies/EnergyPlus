/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef CONTEXT_H_
#define CONTEXT_H_

// Standard
#include <vector>
#include <array>
#include <limits>
#include <unordered_map>
#include <memory>
#include <string>

// Vendor
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <courierr/courierr.h>
#include <linmath.h> // Part of GLFW

// Penumbra
#include "gl/model.h"
#include "gl/shader.h"
#include "gl/program.h"

#define MAX_FLOAT std::numeric_limits<float>::max()

namespace Penumbra {

class Context {

public:
  Context(GLint size, Courierr::Courierr *logger);
  ~Context();
  void show_rendering(unsigned int surface_index, mat4x4 sun_view);
  void set_model(const std::vector<float> &vertices,
                 const std::vector<SurfaceBuffer> &surface_buffers);
  float set_scene(mat4x4 sun_view, const SurfaceBuffer *surface_buffer = nullptr,
                  bool clip_far = true);
  void submit_pssa(unsigned int surface_index, mat4x4 sun_view);
  void submit_pssas(const std::vector<unsigned int> &surface_indices, mat4x4 sun_view);
  void submit_pssa(mat4x4 sun_view);
  float retrieve_pssa(unsigned int surface_index);
  std::vector<float> retrieve_pssas(const std::vector<unsigned int> &surface_indices);
  std::vector<float> retrieve_pssa();

  std::unordered_map<unsigned int, float>
  calculate_interior_pssas(const std::vector<unsigned int> &hidden_surface_indices,
                           const std::vector<unsigned int> &interior_surface_indices,
                           mat4x4 sun_view);
  void show_interior_rendering(const std::vector<unsigned int> &hidden_surface_indices,
                               unsigned int interior_surface_index, mat4x4 sun_view);
  void clear_model();
  static std::string get_vendor_name();

private:
  GLFWwindow *window{nullptr};
  GLuint framebuffer_object{}, renderbuffer_object{};
  static const char *render_vertex_shader_source;
  static const char *render_fragment_shader_source;
  static const char *calculation_vertex_shader_source;
  GLint size;
  GLModel model;
  std::unique_ptr<GLProgram> render_program;
  std::unique_ptr<GLProgram> calculation_program;
  bool model_is_set{false};
  float model_bounding_box[8][4] = {};
  mat4x4 projection = {}, view = {}, mvp = {};
  mat4x4 camera_view = {};
  GLint mvp_location{}, vertex_color_location{};
  bool is_wire_frame_mode{false};
  bool is_camera_mode{false};
  float left{0}, right{0}, bottom{0}, top{0}, near_{0}, far_{0};
  float view_scale{1.f};
  double previous_x_position, previous_y_position;
  float camera_x_rotation_angle{0.f}, camera_y_rotation_angle{0.f};
  bool left_mouse_button_pressed{true};
  std::vector<GLuint> queries;
  std::vector<float> pixel_areas;
  std::vector<GLint> pixel_counts;
  Courierr::Courierr *logger;

  void submit_pssa(const SurfaceBuffer &surface_buffer, mat4x4 sun_view);
  void draw_model();
  void draw_except(const std::vector<SurfaceBuffer> &hidden_surfaces);
  void set_mvp();
  void set_camera_mvp();
  void calculate_camera_view();
  void toggle_wire_frame_mode();
  void toggle_camera_mode();
  void initialize_off_screen_mode();
  void initialize_render_mode();
};

} // namespace Penumbra

#endif // CONTEXT_H_
