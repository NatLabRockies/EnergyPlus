/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef PROGRAM_H_
#define PROGRAM_H_

// Vendor
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <courierr/courierr.h>

namespace Penumbra {

class GLProgram {
public:
  GLProgram(const char *vertex_source, const char *fragment_source, Courierr::Courierr *logger);
  ~GLProgram();
  [[nodiscard]] GLuint get() const;

private:
  GLuint program;
};

} // namespace Penumbra

#endif // PROGRAM_H_
