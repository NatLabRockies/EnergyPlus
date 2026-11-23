/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>

// Penumbra
#include <penumbra/logging.h>
#include "shader.h"

namespace Penumbra {
GLShader::GLShader(GLenum type, const char *source, Courierr::Courierr *logger_in)
    : logger(logger_in) {
  GLint shader_ok;
  GLsizei log_length;
  char info_log[8192];

  shader = glCreateShader(type);
  if (shader != 0) {
    glShaderSource(shader, 1, (const GLchar **)&source, nullptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (shader_ok != GL_TRUE) {
      glGetShaderInfoLog(shader, 8192, &log_length, info_log);
      glDeleteShader(shader);
      shader = 0;
      std::string shader_type_string = (type == GL_FRAGMENT_SHADER) ? "fragment" : "vertex";
      logger->info(std::format("OpenGL {} shader: {}", shader_type_string, info_log));
      throw PenumbraException(std::format("Unable to compile {} shader.", shader_type_string),
                              *logger);
    }
  }
}

GLuint GLShader::get() const {
  return shader;
}

} // namespace Penumbra
