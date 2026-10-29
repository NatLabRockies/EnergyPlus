#ifndef GradientsGTM_hh
#define GradientsGTM_hh

#include <vector>
#include <memory>
#include <string>
#include "EnergyPlus/Data/EnergyPlusData.hh"
#include "EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh"

namespace EnergyPlus {
namespace GroundTemp {

class GradientsGTM : public BaseGroundTempsModel {
public:
    // ---GradientSegments struct (public for unit tests)
    struct GradientSegment {
        Real64 upperDepth;
        Real64 lowerDepth;
        Real64 gradient;
    };
// Constructor and factory
    GradientsGTM() = default;
    static std::unique_ptr<BaseGroundTempsModel> factory(EnergyPlusData &state, std::string const &objectName);

    void setBoreholeDepth(EnergyPlusData &state, Real64 H);

    Real64 getGroundTemp(EnergyPlusData &state) override;
    Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSeconds) override;
    Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int month) override;

    void setSurfaceTemperature(Real64 T_surface);
    void setVerticalGradient(Real64 gradient);
    void initializeSegments(EnergyPlusData &state);
    // --- Testing hooks (used by unit tests only) ---
    
    Real64 testBlendWeight(Real64 depth) const { return blendWeight(depth); }
    Real64 testSegmentTemp(Real64 depth) const { return segmentTemperatureAt(depth); }
// --- Public API needed by tests ---
    void parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object);

private:
 // --- Private helper functions ----   
    void initialize(EnergyPlusData &state,
                    const std::vector<GradientSegment> &idfSegments,
                    Real64 H);

    Real64 getHybridFarfieldTempAtTime(EnergyPlusData &state,
                                       Real64 timeInSeconds,
                                       Real64 depth) const;
    Real64 getHybridFarfieldTemp(EnergyPlusData &state,
                                 int month,
                                 Real64 depth) const;
    Real64 segmentTemperatureAt(Real64 depth) const;
    Real64 blendWeight(Real64 depth) const;

    std::vector<GradientSegment> idfSegments_;
    std::vector<GradientSegment> segments_;
    Real64 boreholeDepth{0.0};
    Real64 referenceTemp{15.0};
    Real64 transitionDepth{15.0};
    Real64 blendWidth{5.0};
    Real64 uniformGradient{0.0};
};

} // namespace GroundTemp
} // namespace EnergyPlus

#endif