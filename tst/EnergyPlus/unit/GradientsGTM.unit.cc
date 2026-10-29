// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the
// University of Illinois, The Regents of the University of California,
// through Ernest Orlando Lawrence Berkeley National Laboratory, and
// other contributors. All rights reserved.
//
// EnergyPlus is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/GroundTemperatureModeling/GradientsGTM.hh>

// EnergyPlus Test Helpers
#include "Fixtures/EnergyPlusFixture.hh"

#include <nlohmann/json.hpp>

using namespace EnergyPlus;
using namespace EnergyPlus::GroundTemp;

// ---------------------------------------------------------------------------
// Test fixture
//
// Derives from EnergyPlusFixture so that state, dataErrTracking, and
// compare_err_stream() are available.  EnergyPlusFixture::SetUp() must be
// called before any test uses state.
// ---------------------------------------------------------------------------
class GradientsGTMTest : public EnergyPlusFixture
{
protected:
    void SetUp() override
    {
        EnergyPlusFixture::SetUp();
        gtm = std::make_unique<GradientsGTM>();
    }

    std::unique_ptr<GradientsGTM> gtm;
};
// ---------------------------------------------------------------------------
// 1. Parser: single segment
// ---------------------------------------------------------------------------
TEST_F(GradientsGTMTest, ParsesSingleSegment)
{
    nlohmann::json j = {
        {"transition_depth", 15.0},
        {"blend_width",       5.0},
        {"upper_depth_1",    15.0},
        {"lower_depth_1",   100.0},
        {"gradient_1",      0.057}
    };

    gtm->parseGradientSegments(*state, j);
    gtm->setBoreholeDepth(*state, 100.0);

    // Segment temperature at z=50: 15 + 0.057 * (50 - 15) = 16.995
    EXPECT_NEAR(gtm->testSegmentTemp(50.0), 16.995, 1e-6);
    // At the top of the first segment: T == referenceTemp (no Kusuda → 15.0)
    EXPECT_NEAR(gtm->testSegmentTemp(15.0), 15.0, 1e-6);
}

// ---------------------------------------------------------------------------
// 2. Cumulative temperature across two segments
// ---------------------------------------------------------------------------
TEST_F(GradientsGTMTest, TwoSegmentCumulative)
{
    nlohmann::json j = {
        {"transition_depth", 15.0},
        {"blend_width",       5.0},
        {"upper_depth_1",    15.0},
        {"lower_depth_1",    50.0},
        {"gradient_1",      0.057},
        {"upper_depth_2",    50.0},
        {"lower_depth_2",   100.0},
        {"gradient_2",      0.030}
    };

    gtm->parseGradientSegments(*state, j);
    gtm->setBoreholeDepth(*state, 100.0);

    // At z=50: 15 + 0.057*35 = 16.995
    EXPECT_NEAR(gtm->testSegmentTemp(50.0),  16.995, 1e-6);
    // At z=100: 16.995 + 0.030*50 = 18.495
    EXPECT_NEAR(gtm->testSegmentTemp(100.0), 18.495, 1e-6);
}

// ---------------------------------------------------------------------------
// 3. Blend weight ramp
//    transitionDepth=15, blendWidth=5  →  ramp from 12.5 to 17.5
// ---------------------------------------------------------------------------
TEST_F(GradientsGTMTest, BlendWeightProfile)
{
    nlohmann::json j = {
        {"transition_depth", 15.0},
        {"blend_width",       5.0},
        {"upper_depth_1",    15.0},
        {"lower_depth_1",   100.0},
        {"gradient_1",      0.057}
    };
    gtm->parseGradientSegments(*state, j);
    gtm->setBoreholeDepth(*state, 100.0);

    EXPECT_NEAR(gtm->testBlendWeight(10.0), 0.0, 1e-9);   // below ramp
    EXPECT_NEAR(gtm->testBlendWeight(12.5), 0.0, 1e-9);   // bottom of ramp
    EXPECT_NEAR(gtm->testBlendWeight(15.0), 0.5, 1e-9);   // midpoint
    EXPECT_NEAR(gtm->testBlendWeight(17.5), 1.0, 1e-9);   // top of ramp
    EXPECT_NEAR(gtm->testBlendWeight(30.0), 1.0, 1e-9);   // above ramp
}

// ---------------------------------------------------------------------------
// 4. Overlap warning fires
// ---------------------------------------------------------------------------
TEST_F(GradientsGTMTest, OverlapWarning)
{
    nlohmann::json j = {
        {"transition_depth", 15.0},
        {"blend_width",       5.0},
        {"upper_depth_1",    15.0},
        {"lower_depth_1",    60.0},
        {"gradient_1",      0.057},
        {"upper_depth_2",    50.0},   // overlaps 50..60
        {"lower_depth_2",   100.0},
        {"gradient_2",      0.030}
    };
    gtm->parseGradientSegments(*state, j);
    gtm->setBoreholeDepth(*state, 100.0);

    EXPECT_TRUE(compare_err_stream_substring("GradientsGTM: Adjacent gradient segments overlap", true));
}

// ---------------------------------------------------------------------------
// 5. Gap warning fires
// ---------------------------------------------------------------------------
TEST_F(GradientsGTMTest, GapWarning)
{
    nlohmann::json j = {
        {"transition_depth", 15.0},
        {"blend_width",       5.0},
        {"upper_depth_1",    15.0},
        {"lower_depth_1",    50.0},
        {"gradient_1",      0.057},
        {"upper_depth_2",    60.0},   // gap 50..60
        {"lower_depth_2",   100.0},
        {"gradient_2",      0.030}
    };
    gtm->parseGradientSegments(*state, j);
    gtm->setBoreholeDepth(*state, 100.0);

    EXPECT_TRUE(compare_err_stream_substring("GradientsGTM: Gap between adjacent gradient segments", true));
}

// ---------------------------------------------------------------------------
// 6. Borehole depth mismatch warning
// ---------------------------------------------------------------------------
TEST_F(GradientsGTMTest, BoreholeDepthMismatchWarning)
{
    nlohmann::json j = {
        {"transition_depth", 15.0},
        {"blend_width",       5.0},
        {"upper_depth_1",    15.0},
        {"lower_depth_1",    80.0},   // ends 20 m above H=100
        {"gradient_1",      0.057}
    };
    gtm->parseGradientSegments(*state, j);
    gtm->setBoreholeDepth(*state, 100.0);

    EXPECT_TRUE(compare_err_stream_substring("deepest gradient segment ends above the borehole bottom", true));
}

// ---------------------------------------------------------------------------
// 7. Hybrid equals reference at transition depth (no Kusuda)
// ---------------------------------------------------------------------------
TEST_F(GradientsGTMTest, HybridAtTransitionDepth)
{
    nlohmann::json j = {
        {"transition_depth", 15.0},
        {"blend_width",       5.0},
        {"upper_depth_1",    15.0},
        {"lower_depth_1",   100.0},
        {"gradient_1",      0.057}
    };
    gtm->parseGradientSegments(*state, j);
    gtm->setBoreholeDepth(*state, 100.0);

    // At z = transitionDepth = 15, w = 0.5, both branches equal 15.0
    EXPECT_NEAR(gtm->getGroundTempAtTimeInMonths(*state, 15.0, 1),
                gtm->testSegmentTemp(15.0), 1e-6);
}