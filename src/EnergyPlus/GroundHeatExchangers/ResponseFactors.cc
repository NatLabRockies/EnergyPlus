// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/GroundHeatExchangers/BoreholeArray.hh>
#include <EnergyPlus/GroundHeatExchangers/ResponseFactors.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::GroundHeatExchangers {

GLHEResponseFactors::GLHEResponseFactors(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j)
{

    // Check for duplicates
    for (auto &existingObj : state.dataGroundHeatExchanger->vertPropsVector) {
        if (objName == existingObj->name) {
            ShowFatalError(state, format("Invalid input for {} object: Duplicate name found: {}", moduleName, existingObj->name));
        }
    }

    this->name = objName;
    this->props = GLHEVertProps::GetVertProps(
        state,
        Util::makeUPPER(
            j["ghe_vertical_properties_object_name"].get<std::string>())); // TODO: this is one spot a properties object is looked up / assigned
    this->numBoreholes = j["number_of_boreholes"].get<int>();
    this->gRefRatio = j["g_function_reference_ratio"].get<Real64>();
    this->maxSimYears = state.dataEnvrn->MaxNumberSimYears;
    auto const &vars = j.at("g_functions");
    for (auto const &var : vars) {
        this->LNTTS.push_back(var.at("g_function_ln_t_ts_value").get<Real64>());
        this->GFNC.push_back(var.at("g_function_g_value").get<Real64>());
    }
    this->numGFuncPairs = static_cast<int>(this->LNTTS.size());
}

std::shared_ptr<GLHEResponseFactors> GetResponseFactor(EnergyPlusData &state, std::string const &objectName)
{
    // Check if this instance of this model has already been retrieved
    const auto thisObj = std::find_if(state.dataGroundHeatExchanger->responseFactorsVector.begin(),
                                      state.dataGroundHeatExchanger->responseFactorsVector.end(),
                                      [&objectName](const std::shared_ptr<GLHEResponseFactors> &myObj) { return myObj->name == objectName; });
    if (thisObj != state.dataGroundHeatExchanger->responseFactorsVector.end()) {
        return *thisObj;
    }
    ShowSevereError(state, fmt::format("Object=GroundHeatExchanger:ResponseFactors, Name={} - not found.", objectName));
    ShowFatalError(state, "Preceding errors cause program termination");
}

std::shared_ptr<GLHEResponseFactors> BuildAndGetResponseFactorObjectFromArray(EnergyPlusData &state,
                                                                              std::shared_ptr<GLHEVertArray> const &arrayObjectPtr)
{
    // Make new response factor object and store it for later use
    std::shared_ptr<GLHEResponseFactors> thisRF(new GLHEResponseFactors);
    thisRF->name = arrayObjectPtr->name;
    thisRF->props = arrayObjectPtr->props;

    // Build out new instances of the vertical BH objects which correspond to this object
    Real64 xLoc = 0;
    int bhCounter = 0;
    for (int xBH = 1; xBH <= arrayObjectPtr->numBHinXDirection; ++xBH) {
        Real64 yLoc = 0;
        for (int yBH = 1; yBH <= arrayObjectPtr->numBHinYDirection; ++yBH) {
            bhCounter += 1;
            std::shared_ptr<GLHEVertSingle> thisBH(new GLHEVertSingle);
            thisBH->name = format("{} BH {} loc: ({}, {})", thisRF->name, bhCounter, xLoc, yLoc);
            thisBH->props = GLHEVertProps::GetVertProps(state, arrayObjectPtr->props->name);
            thisBH->xLoc = xLoc;
            thisBH->yLoc = yLoc;
            thisRF->myBorholes.push_back(thisBH);
            state.dataGroundHeatExchanger->singleBoreholesVector.push_back(thisBH);
            yLoc += arrayObjectPtr->bhSpacing;
            thisRF->numBoreholes += 1;
        }
        xLoc += arrayObjectPtr->bhSpacing;
    }

    thisRF->SetupBHPointsForResponseFactorsObject();
    state.dataGroundHeatExchanger->responseFactorsVector.push_back(thisRF);
    return thisRF;
}

std::shared_ptr<GLHEResponseFactors>
BuildAndGetResponseFactorsObjectFromSingleBHs(const EnergyPlusData &state, std::vector<std::shared_ptr<GLHEVertSingle>> const &singleBHsForRFVect)
{
    // Make new response factor object and store it for later use
    std::shared_ptr<GLHEResponseFactors> thisRF(new GLHEResponseFactors);
    thisRF->name = format("Response Factor Object Auto Generated No: {}", state.dataGroundHeatExchanger->numAutoGeneratedResponseFactors + 1);

    // Make new props object which has the mean values of the other props objects referenced by the individual BH objects
    std::shared_ptr<GLHEVertProps> thisProps(new GLHEVertProps);
    thisProps->name = format("Response Factor Auto Generated Mean Props No: {}", state.dataGroundHeatExchanger->numAutoGeneratedResponseFactors + 1);
    for (auto &thisBH : singleBHsForRFVect) {
        thisProps->bhDiameter += thisBH->props->bhDiameter;
        thisProps->bhLength += thisBH->props->bhLength;
        thisProps->bhTopDepth += thisBH->props->bhTopDepth;
        thisProps->bhUTubeDist += thisBH->props->bhUTubeDist;

        thisProps->grout.cp += thisBH->props->grout.cp;
        thisProps->grout.diffusivity += thisBH->props->grout.diffusivity;
        thisProps->grout.k += thisBH->props->grout.k;
        thisProps->grout.rho += thisBH->props->grout.rho;
        thisProps->grout.rhoCp += thisBH->props->grout.rhoCp;

        thisProps->pipe.cp += thisBH->props->pipe.cp;
        thisProps->pipe.diffusivity += thisBH->props->pipe.diffusivity;
        thisProps->pipe.k += thisBH->props->pipe.k;
        thisProps->pipe.rho += thisBH->props->pipe.rho;
        thisProps->pipe.rhoCp += thisBH->props->pipe.rhoCp;

        thisProps->pipe.outDia += thisBH->props->pipe.outDia;
        thisProps->pipe.thickness += thisBH->props->pipe.thickness;

        thisProps->pipe.innerDia += (thisBH->props->pipe.outDia - 2 * thisBH->props->pipe.thickness);

        thisRF->myBorholes.push_back(thisBH);
    }

    const int numBH = static_cast<int>(singleBHsForRFVect.size());

    // normalize by number of bh
    thisProps->bhDiameter /= numBH;
    thisProps->bhLength /= numBH;
    thisProps->bhTopDepth /= numBH;
    thisProps->bhUTubeDist /= numBH;

    thisProps->grout.cp /= numBH;
    thisProps->grout.diffusivity /= numBH;
    thisProps->grout.k /= numBH;
    thisProps->grout.rho /= numBH;
    thisProps->grout.rhoCp /= numBH;

    thisProps->pipe.cp /= numBH;
    thisProps->pipe.diffusivity /= numBH;
    thisProps->pipe.k /= numBH;
    thisProps->pipe.rho /= numBH;
    thisProps->pipe.rhoCp /= numBH;

    thisProps->pipe.outDia /= numBH;
    thisProps->pipe.thickness /= numBH;

    thisProps->pipe.innerDia /= numBH;

    thisRF->props = thisProps;
    thisRF->numBoreholes = thisRF->myBorholes.size();
    state.dataGroundHeatExchanger->vertPropsVector.push_back(thisProps);

    thisRF->SetupBHPointsForResponseFactorsObject();

    state.dataGroundHeatExchanger->responseFactorsVector.push_back(thisRF);

    state.dataGroundHeatExchanger->numAutoGeneratedResponseFactors += 1;

    return thisRF;
}

void GLHEResponseFactors::SetupBHPointsForResponseFactorsObject() const
{
    for (auto const &thisBH : this->myBorholes) {

        // Using Simpson's rule the number of points (n+1) must be odd, therefore an even number of panels is required
        // Starting from i = 0 to i <= NumPanels produces an odd number of points
        constexpr int numPanels_i = 50;
        constexpr int numPanels_ii = 50;
        constexpr int numPanels_j = 560;

        thisBH->dl_i = thisBH->props->bhLength / numPanels_i;
        for (int i = 0; i <= numPanels_i; ++i) {
            MyCartesian newPoint;
            newPoint.x = thisBH->xLoc;
            newPoint.y = thisBH->yLoc;
            newPoint.z = thisBH->props->bhTopDepth + (i * thisBH->dl_i);
            thisBH->pointLocations_i.push_back(newPoint);
        }

        thisBH->dl_ii = thisBH->props->bhLength / numPanels_ii;
        for (int i = 0; i <= numPanels_ii; ++i) {
            MyCartesian newPoint;
            // For case when bh is being compared to itself, shift points by 1 radius in the horizontal plane
            newPoint.x = thisBH->xLoc + (thisBH->props->bhDiameter / 2.0) / sqrt(2.0);
            newPoint.y = thisBH->yLoc + (thisBH->props->bhDiameter / 2.0) / (-sqrt(2.0));
            newPoint.z = thisBH->props->bhTopDepth + (i * thisBH->dl_ii);
            thisBH->pointLocations_ii.push_back(newPoint);
        }

        thisBH->dl_j = thisBH->props->bhLength / numPanels_j;
        for (int i = 0; i <= numPanels_j; ++i) {
            MyCartesian newPoint;
            newPoint.x = thisBH->xLoc;
            newPoint.y = thisBH->yLoc;
            newPoint.z = thisBH->props->bhTopDepth + (i * thisBH->dl_j);
            thisBH->pointLocations_j.push_back(newPoint);
        }
    }
}

} // namespace EnergyPlus::GroundHeatExchangers
