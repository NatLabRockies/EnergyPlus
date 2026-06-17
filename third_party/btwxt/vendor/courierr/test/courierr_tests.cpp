/* Copyright (c) 2019 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#include <gtest/gtest.h>
#include <courierr/courierr.h>

#define EXPECT_STDOUT(action, ExpectedOut)                                                         \
  {                                                                                                \
    std::stringstream buffer;                                                                      \
    std::streambuf *sbuf = std::cout.rdbuf();                                                      \
    std::cout.rdbuf(buffer.rdbuf());                                                               \
    action std::string capture = buffer.str();                                                     \
    std::cout.rdbuf(sbuf);                                                                         \
    EXPECT_STREQ(ExpectedOut.c_str(), buffer.str().c_str());                                       \
  }

TEST(Courierr, Warning)
{
    Courierr::SimpleCourierr courier;
    courier.warning("This is a warning.");
}

TEST(Courierr, Error)
{
    Courierr::SimpleCourierr courier;
}

TEST(CourierrException, Error)
{
    auto courier = Courierr::SimpleCourierr();
    std::string expected_output{"[ERROR] This is an error!"};
    try {
        throw Courierr::CourierrException("This is an error!", courier);
        EXPECT_STDOUT(int i = 0;, expected_output);
    }
    catch (Courierr::CourierrException &) {
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
