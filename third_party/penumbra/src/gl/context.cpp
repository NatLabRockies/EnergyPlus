/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef NDEBUG
#ifdef __unix__
#include <cfenv>
#endif
#endif

// Penumbra
#include <penumbra/logging.h>
#include "context.h"

namespace Penumbra {

const char *Context::render_vertex_shader_source =
    R"src(
  #version 120
  uniform mat4 MVP;
  uniform vec3 vCol;
  attribute vec3 vPos;
  varying vec3 color;
  void main()
  {
    gl_Position = MVP * vec4(vPos, 1.0);
    color = vCol;
  }
)src";

const char *Context::render_fragment_shader_source =
    R"src(
  #version 120
  varying vec3 color;
  void main()
  {
    gl_FragColor = vec4(color, 1.0);
  }
)src";

const char *Context::calculation_vertex_shader_source =
    R"src(
  #version 120
  uniform mat4 MVP;
  attribute vec3 vPos;
  void main()
  {
    gl_Position = MVP * vec4(vPos, 1.0);
  }
)src";

thread_local static Courierr::Courierr *glfw_logger{nullptr};

static void glfw_error_callback(int, const char *description) {
  if (glfw_logger) {
    glfw_logger->info(std::format("GLFW message: {}", description));
  }
}

Context::Context(GLint size_in, Courierr::Courierr *logger_in) : size(size_in), logger(logger_in) {

  glfw_logger = logger;
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit()) {
    throw PenumbraException(
        "Unable to initialize GLFW. Either there is no GPU, libraries are missing, or "
        "some other error happened.",
        *logger);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
#ifndef NDEBUG
#ifdef __unix__
  // Temporarily Disable floating point exceptions
  fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
#endif
  window = glfwCreateWindow(1, 1, "Penumbra", nullptr, nullptr);
#ifndef NDEBUG
#ifdef __unix__
  feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
#endif
  glfwMakeContextCurrent(window);
  if (!window) {
    throw PenumbraException(
        "Unable to create OpenGL context. OpenGL 2.1+ is required to perform GPU "
        "accelerated shading calculations.",
        *logger);
  }

  // OpenGL extension loader
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw PenumbraException("Failed to load required OpenGL extensions.", *logger);
  }

  if (!glfwExtensionSupported("GL_ARB_vertex_array_object") &&
      !glfwExtensionSupported("GL_APPLE_vertex_array_object")) {
    throw PenumbraException("The current version of OpenGL does not support vertex array objects.",
                            *logger);
  }
  if (!glfwExtensionSupported("GL_EXT_framebuffer_object")) {
    throw PenumbraException("The current version of OpenGL does not support framebuffer objects.",
                            *logger);
  }

  GLint max_view_size[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &max_view_size[0]);
  GLint max_res = std::min(GL_MAX_RENDERBUFFER_SIZE_EXT, max_view_size[0]);
  if (size >= max_res) {
    logger->warning(
        std::format("The selected resolution, {}, is larger than the maximum allowable by your "
                    "hardware, {}. The size will be reset to be equal to the maximum allowable.",
                    size, max_res));
    size = max_res;
  }

  glViewport(0, 0, size, size);

  // Input callbacks for orbit mode
  glfwSetWindowUserPointer(window, this);
#define glfwWPtr(w) static_cast<Context *>(glfwGetWindowUserPointer(w))

  auto key_callback = [](GLFWwindow *w, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
      glfwWPtr(w)->toggle_wire_frame_mode();
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
      glfwWPtr(w)->toggle_camera_mode();
    }
  };

  auto scroll_callback = [](GLFWwindow *w, double /*xOffset*/, double yOffset) {
    glfwWPtr(w)->view_scale += static_cast<float>(0.1 * yOffset);

    if (glfwWPtr(w)->is_camera_mode) {
      glfwWPtr(w)->set_camera_mvp();
    }
  };

  auto mouse_callback = [](GLFWwindow *w, int button, int action, int /*mods*/) {
    // Set a boolean to tell if the left button is down. And at the actual press, initialize the
    // cursor position.  Note that you only come in here the moments the button is pressed and
    // released, not between.

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (GLFW_PRESS == action) {
        glfwWPtr(w)->left_mouse_button_pressed = true;
        glfwGetCursorPos(w, &glfwWPtr(w)->previous_x_position, &glfwWPtr(w)->previous_y_position);
      } else if (GLFW_RELEASE == action) {
        glfwWPtr(w)->left_mouse_button_pressed = false;
      }
    }
  };

  auto cursor_position_callback = [](GLFWwindow *w, double x_position, double y_position) {
    // Constantly monitored cursor position.

    if (glfwWPtr(w)->left_mouse_button_pressed && glfwWPtr(w)->is_camera_mode) {

      static constexpr double rotation_speed = 1. / 300.;

      glfwWPtr(w)->camera_x_rotation_angle =
          static_cast<float>(-(y_position - glfwWPtr(w)->previous_y_position) *
                             rotation_speed); // Y motion should produce x rotation
      glfwWPtr(w)->camera_y_rotation_angle =
          static_cast<float>((x_position - glfwWPtr(w)->previous_x_position) *
                             rotation_speed); // X motion should produce -y rotation

      glfwWPtr(w)->previous_x_position = x_position;
      glfwWPtr(w)->previous_y_position = y_position;

      glfwWPtr(w)->calculate_camera_view();
      glfwWPtr(w)->set_camera_mvp();
    }
  };

#undef glfwWPtr

  glfwSetKeyCallback(window, key_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetMouseButtonCallback(window, mouse_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);

  glfwSwapInterval(1);

  glEnable(GL_DEPTH_TEST);

  // Shader programs

  // Program for off-screen calculation
  calculation_program =
      std::make_unique<GLProgram>(calculation_vertex_shader_source, nullptr, logger);

  glBindAttribLocation(calculation_program->get(), 0, "vPos");

  // Program for on-screen rendering (mostly for debugging)
  render_program = std::make_unique<GLProgram>(render_vertex_shader_source,
                                               render_fragment_shader_source, logger);
  glBindAttribLocation(render_program->get(), 0, "vPos");
  vertex_color_location = glGetUniformLocation(render_program->get(), "vCol");

  // Frame and render buffers
  glGenFramebuffersEXT(1, &framebuffer_object);
  glGenRenderbuffersEXT(1, &renderbuffer_object);

  // Start in off-screen mode
  initialize_off_screen_mode();
}

Context::~Context() {
  glDeleteQueries(static_cast<GLsizei>(queries.size()), queries.data());
  glDeleteFramebuffersEXT(1, &framebuffer_object);
  glDeleteRenderbuffersEXT(1, &renderbuffer_object);
  glDeleteProgram(calculation_program->get());
  glDeleteProgram(render_program->get());
  model.clear_model();
  glfwTerminate();
}
void Context::toggle_wire_frame_mode() {
  is_wire_frame_mode = !is_wire_frame_mode;
  if (is_wire_frame_mode) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}
void Context::toggle_camera_mode() {
  is_camera_mode = !is_camera_mode;
  left_mouse_button_pressed =
      false; // There are things, like killing the last window, that may have left this true.
  if (is_camera_mode) {
    view_scale = 1.0;
    mat4x4_dup(camera_view, view);
    set_camera_mvp();
  } else {
    set_mvp();
  }
}

std::string Context::get_vendor_name() {
  return reinterpret_cast<const char *>(glGetString(GL_VENDOR));
}

void Context::clear_model() {
  model.clear_model();
  glDeleteQueries(static_cast<GLsizei>(queries.size()), queries.data());
  model_is_set = false;
}

void Context::set_model(const std::vector<float> &vertices,
                        const std::vector<SurfaceBuffer> &surface_buffers) {
  if (model_is_set) {
    clear_model();
  }

  // set model vertices
  model.set_vertices(vertices);
  model.set_surface_buffers(surface_buffers);
  queries.resize(surface_buffers.size());
  pixel_areas.resize(surface_buffers.size());
  pixel_counts = std::vector<GLint>(surface_buffers.size(), -1);

  glGenQueries(static_cast<GLsizei>(queries.size()), queries.data());

  float box_left = MAX_FLOAT, box_bottom = MAX_FLOAT, box_front = MAX_FLOAT;
  float box_right = -MAX_FLOAT, box_top = -MAX_FLOAT, box_back = -MAX_FLOAT;

  // calculate bounding box
  for (int i = 0; i < (int)vertices.size(); i += GLModel::vertex_size) {
    float x = vertices[i];
    float y = vertices[i + 1];
    float z = vertices[i + 2];
    box_left = std::min(x, box_left);
    box_right = std::max(x, box_right);
    box_front = std::min(y, box_front);
    box_back = std::max(y, box_back);
    box_bottom = std::min(z, box_bottom);
    box_top = std::max(z, box_top);
  }

  float tempBox[8][4] = {
      {box_left, box_front, box_bottom, 0.0},  {box_left, box_front, box_top, 0.0},
      {box_left, box_back, box_bottom, 0.0},   {box_left, box_back, box_top, 0.0},
      {box_right, box_front, box_bottom, 0.0}, {box_right, box_front, box_top, 0.0},
      {box_right, box_back, box_bottom, 0.0},  {box_right, box_back, box_top, 0.0}};

  for (std::size_t i = 0; i < 8; i++) {
    for (std::size_t j = 0; j < 4; j++) {
      model_bounding_box[i][j] = tempBox[i][j];
    }
  }

  model_is_set = true;
}

float Context::set_scene(mat4x4 sun_view, const SurfaceBuffer *surface_buffer, bool clip_far) {

  if (!model_is_set) {
    throw PenumbraException("Model has not been set. Cannot set OpenGL scene.", *logger);
  }

  mat4x4_dup(view, sun_view);

  // calculate clipping planes in rendered coordinates
  left = MAX_FLOAT;
  right = -MAX_FLOAT;
  bottom = MAX_FLOAT;
  top = -MAX_FLOAT;
  near_ = -MAX_FLOAT;
  far_ = MAX_FLOAT;

  // If surface buffer has not been set use entire model instead.
  GLuint beg = surface_buffer ? surface_buffer->begin * GLModel::vertex_size : 0;
  GLuint end = surface_buffer ? surface_buffer->begin * GLModel::vertex_size +
                                    surface_buffer->count * GLModel::vertex_size
                              : static_cast<GLuint>(model.vertex_array.size());

  for (GLuint i = beg; i < end; i += GLModel::vertex_size) {
    vec4 translation;
    vec4 point = {model.vertex_array[i], model.vertex_array[i + 1], model.vertex_array[i + 2], 0};
    mat4x4_mul_vec4(translation, view, point);
    left = std::min(translation[0], left);
    right = std::max(translation[0], right);
    bottom = std::min(translation[1], bottom);
    top = std::max(translation[1], top);
    // near_ = min(translation[2], near_);
    far_ = std::min(translation[2], far_);
  }

  // Use model box to determine near clipping plane (and far if looking interior)
  for (auto const coordinate : model_bounding_box) {
    vec4 translation;
    mat4x4_mul_vec4(translation, view, coordinate);
    near_ = std::max(translation[2], near_);
    if (!clip_far) {
      far_ = std::min(translation[2], far_);
    }
  }

  // account for camera position
  near_ -= 0.999f; // For some reason, -1. is too tight when sun is perpendicular to the surface.
  far_ -= 1.001f;  // For some reason, -1. is too tight when sun is perpendicular to the surface.

  // Grow horizontal extents of view by one pixel on each side

  const float inverse_size = 1.f / static_cast<float>(size);

  const float delta_x = (right - left) * inverse_size;
  left -= delta_x;
  right += delta_x;

  // Grow vertical extents of view by one pixel on each side
  const float delta_y = (top - bottom) * inverse_size;
  bottom -= delta_y;
  top += delta_y;

  // calculate pixel area (A[i]*cos(theta) for each pixel of the surface)
  // multiplies by the number of pixels to get projected sunlit surface area

  auto const pixel_area = (right - left) * (top - bottom) * inverse_size * inverse_size;

  if (pixel_area > 0.0) {
    mat4x4_ortho(projection, left, right, bottom, top, -near_, -far_);
    mat4x4_mul(mvp, projection, view);

    set_mvp();
  }

  // TODO: Consider what to do with the camera if pixel_area happens to be zero

  return pixel_area;
}

void Context::calculate_camera_view() {
  // Transpose changes the affects of consecutive rotations from local to global space.
  mat4x4 temporary_matrix;
  mat4x4_transpose(temporary_matrix, camera_view);
  mat4x4_rotate_X(camera_view, temporary_matrix, camera_x_rotation_angle);
  mat4x4_rotate_Y(temporary_matrix, camera_view, camera_y_rotation_angle);
  mat4x4_transpose(camera_view, temporary_matrix); // Transpose back.
}

void Context::set_mvp() {
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *)mvp);
}

void Context::set_camera_mvp() {
  float delta_width, delta_height;
  float camera_right = right;
  float camera_left = left;
  float camera_top = top;
  float camera_bottom = bottom;
  float camera_near = 100.; // Set near and far to something now and make it tighter later.
  float camera_far = -100.; // To tighten, look at the sphere of possible rotations.

  delta_width = (camera_right - camera_left) / 2.f;
  delta_height = (camera_top - camera_bottom) / 2.f;

  if (delta_width > delta_height) {
    camera_top += (delta_width - delta_height);
    camera_bottom -= (delta_width - delta_height);
  } else {
    camera_left -= (delta_height - delta_width);
    camera_right += (delta_width - delta_height);
  }

  mat4x4 camera_projection;
  mat4x4 camera_mvp;

  mat4x4_ortho(camera_projection, view_scale * camera_left, view_scale * camera_right,
               view_scale * camera_bottom, view_scale * camera_top, -camera_near, -camera_far);
  mat4x4_mul(camera_mvp, camera_projection, camera_view);
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *)camera_mvp);
}

void Context::draw_model() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#ifndef NDEBUG
#ifdef __unix__
  // Temporarily Disable floating point exceptions
  fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LESS);
  model.draw_all();
  glDepthFunc(GL_EQUAL);
#ifndef NDEBUG
#ifdef __unix__
  feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
#endif
}

void Context::draw_except(const std::vector<SurfaceBuffer> &hidden_surfaces) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#ifndef NDEBUG
#ifdef __unix__
  // Temporarily Disable floating point exceptions
  fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
#endif
  glClear(GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LESS);
  model.draw_except(hidden_surfaces);
  glDepthFunc(GL_EQUAL);
#ifndef NDEBUG
#ifdef __unix__
  feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
#endif
}

void Context::show_rendering(const unsigned int surface_index, mat4x4 sun_view) {
  glfwSetWindowSize(window, size, size);
  glfwShowWindow(window);

  initialize_render_mode();

  auto const &surface_buffer = model.surface_buffers[surface_index];
  set_scene(sun_view, &surface_buffer);

  while (!glfwWindowShouldClose(window)) {
    glUniform3f(vertex_color_location, 0.5f, 0.5f, 0.5f);
    draw_model();
    glUniform3f(vertex_color_location, 1.f, 1.f, 1.f);
    GLModel::draw_surface(surface_buffer);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwSetWindowShouldClose(window, 0);
  glfwHideWindow(window);

  initialize_off_screen_mode();
}

void Context::show_interior_rendering(const std::vector<unsigned int> &hidden_surface_indices,
                                      const unsigned interior_surface_index, mat4x4 sun_view) {
  glfwSetWindowSize(window, size, size);
  glfwShowWindow(window);

  initialize_render_mode();

  auto const &interior_surface = model.surface_buffers[interior_surface_index];
  std::vector<SurfaceBuffer> hidden_surfaces;
  hidden_surfaces.reserve(hidden_surface_indices.size());
  for (auto const hidden_surface : hidden_surface_indices) {
    hidden_surfaces.push_back(model.surface_buffers[hidden_surface]);
  }

  set_scene(sun_view, &model.surface_buffers[hidden_surface_indices.at(0)], false);

  while (!glfwWindowShouldClose(window)) {
    glUniform3f(vertex_color_location, 0.5f, 0.5f, 0.5f);
    draw_except(hidden_surfaces);
    glUniform3f(vertex_color_location, 1.f, 1.f, 1.f);
    GLModel::draw_surface(interior_surface);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwSetWindowShouldClose(window, 0);
  glfwHideWindow(window);

  initialize_off_screen_mode();
}

void Context::submit_pssa(const SurfaceBuffer &surface_buffer, mat4x4 sun_view) {
  auto const pixel_area = set_scene(sun_view, &surface_buffer);
  draw_model();
  glBeginQuery(GL_SAMPLES_PASSED, queries.at(surface_buffer.index));
  GLModel::draw_surface(surface_buffer);
  glEndQuery(GL_SAMPLES_PASSED);
  pixel_areas.at(surface_buffer.index) = pixel_area;
}

void Context::submit_pssa(const unsigned int surface_index, mat4x4 sun_view) {
  submit_pssa(model.surface_buffers[surface_index], sun_view);
}

void Context::submit_pssas(const std::vector<unsigned int> &surface_indices, mat4x4 sun_view) {
  for (auto const surface_index : surface_indices) {
    submit_pssa(surface_index, sun_view);
  }
}

void Context::submit_pssa(mat4x4 sun_view) {
  for (auto const &surface_buffer : model.surface_buffers) {
    submit_pssa(surface_buffer, sun_view);
  }
}

float Context::retrieve_pssa(const unsigned int surface_index) {
  glGetQueryObjectiv(queries[surface_index], GL_QUERY_RESULT, &(pixel_counts.at(surface_index)));
  return static_cast<float>(pixel_counts[surface_index]) * pixel_areas[surface_index];
}

std::vector<float> Context::retrieve_pssas(const std::vector<unsigned int> &surface_indices) {
  std::vector<float> pssas;
  pssas.reserve(surface_indices.size());
  for (const unsigned int surface_index : surface_indices) {
    pssas.push_back(retrieve_pssa(surface_index));
  }
  return pssas;
}

std::vector<float> Context::retrieve_pssa() {
  std::vector<float> pssas;
  pssas.reserve(model.surface_buffers.size());
  for (auto const &surface_buffer : model.surface_buffers) {
    pssas.push_back(retrieve_pssa(surface_buffer.index));
  }
  return pssas;
}

std::unordered_map<unsigned int, float>
Context::calculate_interior_pssas(const std::vector<unsigned int> &hidden_surface_indices,
                                  const std::vector<unsigned int> &interior_surface_indices,
                                  mat4x4 sun_view) {

  std::vector<GLuint> interior_queries(interior_surface_indices.size());
  std::unordered_map<unsigned int, float> pssas;

  glGenQueries(static_cast<GLsizei>(interior_queries.size()), interior_queries.data());

  auto const pixel_area =
      set_scene(sun_view, &model.surface_buffers[hidden_surface_indices.at(0)], false);

  std::vector<SurfaceBuffer> hidden_surfaces;
  hidden_surfaces.reserve(hidden_surface_indices.size());
  for (auto const hidden_surface : hidden_surface_indices) {
    hidden_surfaces.push_back(model.surface_buffers[hidden_surface]);
  }
  std::vector<SurfaceBuffer> interior_surfaces;
  interior_surfaces.reserve(interior_surface_indices.size());
  for (auto const interior_surface : interior_surface_indices) {
    interior_surfaces.push_back(model.surface_buffers[interior_surface]);
  }

  draw_except(hidden_surfaces);

  for (size_t i = 0; i < interior_surfaces.size(); ++i) {
    glBeginQuery(GL_SAMPLES_PASSED, interior_queries[i]);
    GLModel::draw_surface(interior_surfaces[i]);
    glEndQuery(GL_SAMPLES_PASSED);
  }

  for (size_t i = 0; i < interior_surfaces.size(); ++i) {
    GLint pixel_count;
    glGetQueryObjectiv(interior_queries[i], GL_QUERY_RESULT, &pixel_count);

    pssas[interior_surfaces[i].index] = static_cast<float>(pixel_count) * pixel_area;
  }

  glDeleteQueries(static_cast<GLsizei>(interior_queries.size()), interior_queries.data());
  return pssas;
}

void Context::initialize_off_screen_mode() {
  glUseProgram(calculation_program->get());
  mvp_location = glGetUniformLocation(calculation_program->get(), "MVP");
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer_object);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer_object);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, size, size);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT,
                               renderbuffer_object);

  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
    std::string reason;
    switch (status) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: {
      reason = "Incomplete attachment.";
    } break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: {
      reason = "Incomplete or missing attachment.";
    } break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: {
      reason = "Incomplete formats.";
    } break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: {
      reason = "Incomplete draw buffer.";
    } break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: {
      reason = "Incomplete read buffer.";
    } break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT: {
      reason = "Framebuffers are not supported.";
    } break;
    default: {
      reason = "Reason unknown.";
    }
    }
    throw PenumbraException(std::format("Unable to create framebuffer. {}", reason), *logger);
  }

  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void Context::initialize_render_mode() {
  // set to default framebuffer and renderbuffer
  glUseProgram(render_program->get());
  mvp_location = glGetUniformLocation(render_program->get(), "MVP");
  set_mvp();
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

} // namespace Penumbra
