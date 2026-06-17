/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Penumbra
#include "program.h"
#include "shader.h"

namespace Penumbra {

GLProgram::GLProgram(const char *vertex_source, const char *fragment_source,
                     Courierr::Courierr *logger) {
  program = glCreateProgram();
  GLShader vertex(GL_VERTEX_SHADER, vertex_source, logger);
  glAttachShader(program, vertex.get());
  if (fragment_source) {
    GLShader fragment(GL_FRAGMENT_SHADER, fragment_source, logger);
    glAttachShader(program, fragment.get());
  }
  glLinkProgram(program);
}

GLProgram::~GLProgram() = default;

GLuint GLProgram::get() const {
  return program;
}

} // namespace Penumbra
