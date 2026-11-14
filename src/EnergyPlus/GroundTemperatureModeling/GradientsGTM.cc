#include "GradientsGTM.hh"

// C++ Headers
#include <format>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {
namespace GroundTemp {

std::unique_ptr<BaseGroundTempsModel> GradientsGTM::factory(
    EnergyPlusData &state,
    std::string const &objectName)
{
        auto model = std::make_unique<GradientsGTM>();

    auto const &epJSON = state.dataInputProcessing->inputProcessor->epJSON;

    auto it = epJSON.find("Site:GroundTemperature:Undisturbed:GradientSegments");
    if (it != epJSON.end()) {
        for (auto const &item : it.value().items()) {
            if (Util::makeUPPER(item.key()) == Util::makeUPPER(objectName)) {
                model->parseGradientSegments(state, item.value());
                break;
            }
        }
    }

    // Instantiate the paired KusudaAchenbach model (if present) so that
    // getHybridFarfieldTemp* can blend with a real seasonal temperature.
    auto kusudaIt = epJSON.find("Site:GroundTemperature:Undisturbed:KusudaAchenbach");
    if (kusudaIt != epJSON.end() && !kusudaIt.value().empty()) {
        std::string kusudaName = kusudaIt.value().begin().key();
        GroundTemp::GetGroundTempModelAndInit(
            state,
            GroundTemp::ModelType::Kusuda,
            Util::makeUPPER(kusudaName));
    }

    return model;
}

void GradientsGTM::parseGradientSegments(EnergyPlusData &state, nlohmann::json const &object)
{
    idfSegments_.clear();

    if (object.contains("transition_depth")) {
        transitionDepth = object["transition_depth"].get<Real64>();
    }
    if (object.contains("blend_width")) {
        blendWidth = object["blend_width"].get<Real64>();
    }
    if (object.contains("reference_temperature")) {
        referenceTemp = object["reference_temperature"].get<Real64>();
    }

    // Parse the flat extensible fields
    int segNum = 1;
    while (true) {
        std::string upperKey = "upper_depth_" + std::to_string(segNum);
        std::string lowerKey = "lower_depth_" + std::to_string(segNum);
        std::string gradKey  = "gradient_" + std::to_string(segNum);

        if (object.contains(upperKey)) {
            GradientSegment gs;
            gs.upperDepth = object[upperKey].get<Real64>();
            gs.lowerDepth = object[lowerKey].get<Real64>();
            gs.gradient   = object[gradKey].get<Real64>();
            idfSegments_.push_back(gs);
            segNum++;
        } else {
            break;
        }
    }

    // Debug to prove it worked
    if (idfSegments_.empty()) {
        ShowSevereError(state, "GradientsGTM: No segments were parsed from the IDF!");
    } else {
        std::string msg = "GradientsGTM: Successfully parsed " + std::to_string(idfSegments_.size()) + " segment(s).";
        ShowWarningError(state, msg);
    }
}

void GradientsGTM::initialize(EnergyPlusData &state,
                              const std::vector<GradientSegment> &idfSegments,
                              Real64 H)
{
    idfSegments_.clear();
    for (const auto &s : idfSegments) {
        idfSegments_.push_back(s);
    }
    setBoreholeDepth(state, H);
}

void GradientsGTM::setBoreholeDepth(EnergyPlusData &state, Real64 H)
{
    boreholeDepth = H;

    segments_.clear();
    for (const auto &s : idfSegments_) {
        segments_.push_back(s);
    }
    std::sort(segments_.begin(), segments_.end(),
              [](const GradientSegment &a, const GradientSegment &b) {
                  return a.upperDepth < b.upperDepth;
              });

    // --- Check for overlapping segments ---
    for (size_t i = 1; i < segments_.size(); ++i) {
        Real64 const prevLower = segments_[i-1].lowerDepth;
        Real64 const thisUpper = segments_[i].upperDepth;
        if (thisUpper < prevLower - 0.01) {
            ShowWarningError(state,
                "GradientsGTM: Adjacent gradient segments overlap.");
                        ShowContinueError(state,
                "  Segment " + std::to_string(i) + " ends at " + std::to_string(prevLower) +
                " m; Segment " + std::to_string(i+1) + " starts at " + std::to_string(thisUpper) + " m.");            ShowContinueError(state,
                "  The overlapped range will contribute twice to the cumulative temperature.");
        }
    }

    // --- Check for gaps between segments ---
    for (size_t i = 1; i < segments_.size(); ++i) {
        Real64 const prevLower = segments_[i-1].lowerDepth;
        Real64 const thisUpper = segments_[i].upperDepth;
        if (thisUpper > prevLower + 0.01) {
            ShowWarningError(state,
                "GradientsGTM: Gap between adjacent gradient segments.");
                     ShowContinueError(state,
                "  Segment " + std::to_string(i) + " ends at " + std::to_string(prevLower) +
                " m; Segment " + std::to_string(i+1) + " starts at " + std::to_string(thisUpper) + " m.");
                ShowContinueError(state,
                "  The temperature will be discontinuous across this gap.");
        }
    }

    // --- Check that the deepest segment reaches the borehole bottom ---
    if (!segments_.empty() && H > 0.0) {
        Real64 const lastLower = segments_.back().lowerDepth;
        if (lastLower < H - 0.5) {
            ShowWarningError(state,
                "GradientsGTM: The deepest gradient segment ends above the borehole bottom.");
             ShowContinueError(state,
                "  Borehole length H = " + std::to_string(H) +
                " m; last segment lower depth = " + std::to_string(lastLower) + " m.");           
            ShowContinueError(state,
                "  The final segment's gradient will be extrapolated below its lower depth.");
        } else if (lastLower > H + 0.5) {
            ShowWarningError(state,
                "GradientsGTM: The deepest gradient segment extends below the borehole bottom.");
             ShowContinueError(state,
                 "  Borehole length H = " + std::to_string(H) +
                " m; last segment lower depth = " + std::to_string(lastLower) + " m.");
 ShowContinueError(state,
                "  This is permitted but may indicate a data-entry mistake.");
        }
    }
}

Real64 GradientsGTM::segmentTemperatureAt(Real64 depth) const
{  
if (segments_.empty()) {
        return referenceTemp;
    }

    Real64 T = referenceTemp;
    Real64 zRef = segments_.front().upperDepth;

    if (depth < zRef) {
        return referenceTemp + segments_.front().gradient * (depth - zRef);
    }

    for (const auto &seg : segments_) {
        if (depth < seg.upperDepth) break;
        Real64 zTop    = seg.upperDepth;
        Real64 zBottom = seg.lowerDepth;
        Real64 zEff    = std::min(depth, zBottom);
        if (zEff > zTop) {
            T += seg.gradient * (zEff - zTop);
        }
        if (depth <= zBottom) {
            return T;
        }
    }

    const auto &last = segments_.back();
    T += last.gradient * (depth - last.lowerDepth);
    return T;
}

Real64 GradientsGTM::blendWeight(Real64 depth) const
{
    Real64 low  = transitionDepth - blendWidth / 2.0;
    Real64 high = transitionDepth + blendWidth / 2.0;
    if (depth <= low)  return 0.0;
    if (depth >= high) return 1.0;
    Real64 w = (depth - low) / (high - low);
    return std::clamp(w, 0.0, 1.0);
}

Real64 GradientsGTM::getHybridFarfieldTempAtTime(EnergyPlusData &state,
                                                 Real64 timeInSeconds,
                                                 Real64 depth) const
{
    Real64 kaTemp = referenceTemp;
    if (state.dataGrndTempModelMgr) {
        for (const auto &m : state.dataGrndTempModelMgr->groundTempModels) {
            if (m && m != this && m->modelType == ModelType::Kusuda) {
                kaTemp = m->getGroundTempAtTimeInSeconds(state, depth, timeInSeconds);
                break;
            }
        }
    }

    Real64 gradTemp = segmentTemperatureAt(depth);
    Real64 w = blendWeight(depth);

    // --- START OF DEBUG BLOCK ---
    if (state.dataGlobal && state.dataGlobal->HourOfDay == 12 && state.dataGlobal->TimeStep == 1) {
        std::vector<Real64> depths = {1.0, 26.0, 51.0, 76.0, 101.0};
        std::ostringstream debugMsg;
        debugMsg << "GTM DEBUG Day:" << state.dataEnvrn->DayOfYear 
                 << " | RefT:" << referenceTemp 
                 << " | TransD:" << transitionDepth 
                 << " | BlendW:" << blendWidth
                 << " | NumSegments:" << segments_.size() << " | ";
        
        for (Real64 z : depths) {
            Real64 kaT = referenceTemp;
            if (state.dataGrndTempModelMgr) {
                for (const auto &m : state.dataGrndTempModelMgr->groundTempModels) {
                    if (m && m != this && m->modelType == ModelType::Kusuda) {
                        kaT = m->getGroundTempAtTimeInSeconds(state, z, timeInSeconds);
                        break;
                    }
                }
            }
            Real64 gradT = segmentTemperatureAt(z);
            Real64 weight = blendWeight(z);
            Real64 finalT = (1.0 - weight) * kaT + weight * gradT;
            
            debugMsg << std::fixed << std::setprecision(2) 
                     << " " << z << "m:" << finalT << "C (K:" << kaT << ", G:" << gradT << ", W:" << std::setprecision(1) << weight << ") |";
        }
        ShowWarningError(state, debugMsg.str());
    }
    // --- END OF DEBUG BLOCK ---

    return (1.0 - w) * kaTemp + w * gradTemp; 
 }

Real64 GradientsGTM::getHybridFarfieldTemp(EnergyPlusData &state,
                                           int month,
                                           Real64 depth) const
{
    Real64 kaTemp = referenceTemp;
    if (state.dataGrndTempModelMgr) {
        for (const auto &m : state.dataGrndTempModelMgr->groundTempModels) {
            if (m && m != this && m->modelType == ModelType::Kusuda) {
                kaTemp = m->getGroundTempAtTimeInMonths(state, depth, month);
                break;
            }
        }
    }

    Real64 gradTemp = segmentTemperatureAt(depth);
    Real64 w = blendWeight(depth);
    return (1.0 - w) * kaTemp + w * gradTemp;
}

Real64 GradientsGTM::getGroundTemp(EnergyPlusData &state)
{
    Real64 depth = (boreholeDepth > 0.0) ? (boreholeDepth / 2.0) : 0.0;
    int month = 1;
    if (state.dataEnvrn) month = state.dataEnvrn->Month;
    return getHybridFarfieldTemp(state, month, depth);
}

Real64 GradientsGTM::getGroundTempAtTimeInSeconds(EnergyPlusData &state,
                                                  Real64 depth,
                                                  Real64 timeInSeconds)
{
    return getHybridFarfieldTempAtTime(state, timeInSeconds, depth);
}

Real64 GradientsGTM::getGroundTempAtTimeInMonths(EnergyPlusData &state,
                                                 Real64 depth,
                                                 int month)
{
    return getHybridFarfieldTemp(state, month, depth);
}
void GradientsGTM::setSurfaceTemperature(Real64 T_surface)
{
    referenceTemp = T_surface;
}

void GradientsGTM::setVerticalGradient(Real64 gradient)
{
    uniformGradient = gradient;
}

void GradientsGTM::initializeSegments(EnergyPlusData &state)
{
    if (uniformGradient != 0.0) {
        idfSegments_.clear();
        GradientSegment seg;
        seg.upperDepth = 0.0;
        seg.lowerDepth = 200.0;
        seg.gradient = uniformGradient;
        idfSegments_.push_back(seg);
    }
    if (boreholeDepth > 0.0) {
        initialize(state, idfSegments_, boreholeDepth);
    }
}

} // namespace GroundTemp
} // namespace EnergyPlus