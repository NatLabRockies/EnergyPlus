/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef SHADER_H_
#define SHADER_H_

// Vendor
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <courierr/courierr.h>

namespace Penumbra {

class GLShader {
public:
  GLShader(GLenum type, const char *source, Courierr::Courierr *logger);
  ~GLShader() = default;
  [[nodiscard]] GLuint get() const;

private:
  GLuint shader;
  Courierr::Courierr *logger;
};

} // namespace Penumbra

#endif // SHADER_H_
