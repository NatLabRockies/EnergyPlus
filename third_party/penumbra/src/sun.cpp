/* Copyright (c) 2017 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Penumbra
#include "sun.h"

namespace Penumbra {

Sun::Sun() : azimuth(0.f), altitude(0.f) {}

void Sun::set_azimuth(float azimuth_in) {
  azimuth = azimuth_in;
}

void Sun::set_altitude(float altitude_in) {
  altitude = altitude_in;
}

void Sun::set_view(float azimuth_in, float altitude_in) {
  set_azimuth(azimuth_in);
  set_altitude(altitude_in);
  set_view();
}

void Sun::set_view() {

  float cosAlt = std::cos(altitude);

  vec3 eye = {cosAlt * std::sin(azimuth), cosAlt * std::cos(azimuth), std::sin(altitude)};
  vec3 center = {0.f, 0.f, 0.f};
  vec3 up = {0.f, 0.f, 1.f};

  mat4x4_look_at(view, eye, center, up);
}

mat4x4_ptr Sun::get_view() {
  return view;
}

float Sun::get_azimuth() const {
  return azimuth;
}

float Sun::get_altitude() const {
  return altitude;
}
} // namespace Penumbra
