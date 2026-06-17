/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef FOUNDATION_FIXTURE_HPP_
#define FOUNDATION_FIXTURE_HPP_

#include "Foundation.hpp"
#include <gtest/gtest.h>

using namespace Kiva;

Foundation typical_fnd();

class FoundationFixture : public testing::Test {
public:
  void SetUp() { fnd = typical_fnd(); }

  Foundation fnd;
};

#endif /* FOUNDATION_FIXTURE_HPP_ */
