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

#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/GroundHeatExchangers/BoreholeArray.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <EnergyPlus/GroundHeatExchangers/Vertical.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::GroundHeatExchangers {
GLHEVert::GLHEVert(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j)
{
    // Check for duplicates
    for (auto &existingObj : state.dataGroundHeatExchanger->verticalGLHE) {
        if (objName == existingObj.name) {
            ShowFatalError(state,
                           format("Invalid input for {} object: Duplicate name found: {}",
                                  EnergyPlus::GroundHeatExchangers::GLHEVert::moduleName,
                                  existingObj.name));
        }
    }

    bool errorsFound = false;

    this->name = objName;

    // get inlet node num
    std::string const inletNodeName = Util::makeUPPER(j["inlet_node_name"].get<std::string>());

    this->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                             inletNodeName,
                                                             errorsFound,
                                                             DataLoopNode::ConnectionObjectType::GroundHeatExchangerSystem,
                                                             objName,
                                                             DataLoopNode::NodeFluidType::Water,
                                                             DataLoopNode::ConnectionType::Inlet,
                                                             NodeInputManager::CompFluidStream::Primary,
                                                             DataLoopNode::ObjectIsNotParent);

    // get outlet node num
    std::string const outletNodeName = Util::makeUPPER(j["outlet_node_name"].get<std::string>());

    this->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                              outletNodeName,
                                                              errorsFound,
                                                              DataLoopNode::ConnectionObjectType::GroundHeatExchangerSystem,
                                                              objName,
                                                              DataLoopNode::NodeFluidType::Water,
                                                              DataLoopNode::ConnectionType::Outlet,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              DataLoopNode::ObjectIsNotParent);
    this->available = true;
    this->on = true;

    BranchNodeConnections::TestCompSet(state, this->moduleName, objName, inletNodeName, outletNodeName, "Condenser Water Nodes");

    this->designFlow = j["design_flow_rate"].get<Real64>();
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->inletNodeNum, this->designFlow);

    this->soil.k = j["ground_thermal_conductivity"].get<Real64>();
    this->soil.rhoCp = j["ground_thermal_heat_capacity"].get<Real64>();

    if (j.find("ghe_vertical_responsefactors_object_name") != j.end()) {
        // Response factors come from IDF object
        this->myRespFactors = GetResponseFactor(state, Util::makeUPPER(j["ghe_vertical_responsefactors_object_name"].get<std::string>()));
        this->gFunctionsExist = true;
    }

    // no g-functions in the input file, so they need to be calculated
    if (!this->gFunctionsExist) {

        // g-function calculation method
        if (j.find("g_function_calculation_method") != j.end()) {
            std::string gFunctionMethodStr = Util::makeUPPER(j["g_function_calculation_method"].get<std::string>());
            if (gFunctionMethodStr == "UHFCALC") {
                this->gFuncCalcMethod = GFuncCalcMethod::UniformHeatFlux;
            } else if (gFunctionMethodStr == "UBHWTCALC") {
                this->gFuncCalcMethod = GFuncCalcMethod::UniformBoreholeWallTemp;
            } else {
                errorsFound = true;
                ShowSevereError(state, fmt::format("g-Function Calculation Method: \"{}\" is invalid", gFunctionMethodStr));
            }
        }

        // get borehole data from array or individual borehole instance objects
        if (j.find("ghe_vertical_array_object_name") != j.end()) {
            // Response factors come from array object
            this->myRespFactors = BuildAndGetResponseFactorObjectFromArray(
                state, GLHEVertArray::GetVertArray(state, Util::makeUPPER(j["ghe_vertical_array_object_name"].get<std::string>())));
        } else {
            if (j.find("vertical_well_locations") == j.end()) {
                // No ResponseFactors, GHEArray, or SingleBH object are referenced
                ShowSevereError(state, "No GHE:ResponseFactors, GHE:Vertical:Array, or GHE:Vertical:Single objects found");
                ShowFatalError(state, format("Check references to these objects for GHE:System object: {}", this->name));
            }

            auto const &vars = j.at("vertical_well_locations");

            // Calculate response factors from individual boreholes
            std::vector<std::shared_ptr<GLHEVertSingle>> tempVectOfBHObjects;

            for (auto const &var : vars) {
                if (!var.at("ghe_vertical_single_object_name").empty()) {
                    std::shared_ptr<GLHEVertSingle> tempBHptr =
                        GLHEVertSingle::GetSingleBH(state, Util::makeUPPER(var.at("ghe_vertical_single_object_name").get<std::string>()));
                    tempVectOfBHObjects.push_back(tempBHptr);
                } else {
                    break;
                }
            }

            this->myRespFactors = BuildAndGetResponseFactorsObjectFromSingleBHs(state, tempVectOfBHObjects);

            if (!this->myRespFactors) {
                errorsFound = true;
                ShowSevereError(state, "GroundHeatExchanger:Vertical:Single objects not found.");
            }
        }
    }

    this->bhDiameter = this->myRespFactors->props->bhDiameter;
    this->bhRadius = this->bhDiameter / 2.0;
    this->bhLength = this->myRespFactors->props->bhLength;
    this->bhUTubeDist = this->myRespFactors->props->bhUTubeDist;

    // pull pipe and grout data up from response factor struct for simplicity
    this->pipe.outDia = this->myRespFactors->props->pipe.outDia;
    this->pipe.innerDia = this->myRespFactors->props->pipe.innerDia;
    this->pipe.outRadius = this->pipe.outDia / 2;
    this->pipe.innerRadius = this->pipe.innerDia / 2;
    this->pipe.thickness = this->myRespFactors->props->pipe.thickness;
    this->pipe.k = this->myRespFactors->props->pipe.k;
    this->pipe.rhoCp = this->myRespFactors->props->pipe.rhoCp;

    this->grout.k = this->myRespFactors->props->grout.k;
    this->grout.rhoCp = this->myRespFactors->props->grout.rhoCp;

    this->myRespFactors->gRefRatio = this->bhRadius / this->bhLength;

    // Number of simulation years from RunPeriod
    this->myRespFactors->maxSimYears = state.dataEnvrn->MaxNumberSimYears;

    // total tube length
    this->totalTubeLength = this->myRespFactors->numBoreholes * this->myRespFactors->props->bhLength;

    // ground thermal diffusivity
    this->soil.diffusivity = this->soil.k / this->soil.rhoCp;

    // multipole method constants
    this->theta_1 = this->bhUTubeDist / (2 * this->bhRadius);
    this->theta_2 = this->bhRadius / this->pipe.outRadius;
    this->theta_3 = 1 / (2 * this->theta_1 * this->theta_2);
    this->sigma = (this->grout.k - this->soil.k) / (this->grout.k + this->soil.k);

    this->SubAGG = 15;
    this->AGG = 192;

    // Allocation of all the dynamic arrays
    this->QnMonthlyAgg.dimension(static_cast<int>(this->myRespFactors->maxSimYears * 12), 0.0);
    this->QnHr.dimension(730 + this->AGG + this->SubAGG, 0.0);
    this->QnSubHr.dimension(static_cast<int>((this->SubAGG + 1) * maxTSinHr + 1), 0.0);
    this->LastHourN.dimension(this->SubAGG + 1, 0);

    this->prevTimeSteps.allocate(static_cast<int>((this->SubAGG + 1) * maxTSinHr + 1));
    this->prevTimeSteps = 0.0;

    GroundTemp::ModelType modelType = static_cast<GroundTemp::ModelType>(
        getEnumValue(GroundTemp::modelTypeNamesUC, Util::makeUPPER(j["undisturbed_ground_temperature_model_type"].get<std::string>())));
    assert(modelType != GroundTemp::ModelType::Invalid);

    // Initialize ground temperature model and get pointer reference
    this->groundTempModel =
        GroundTemp::GetGroundTempModelAndInit(state, modelType, Util::makeUPPER(j["undisturbed_ground_temperature_model_name"].get<std::string>()));

    // Check for Errors
    if (errorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", this->moduleName));
    }
}

void GLHEVert::getAnnualTimeConstant()
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February 2015

    // PURPOSE OF THIS SUBROUTINE:
    // calculate annual time constant for ground conduction

    constexpr Real64 hrInYear = 8760;

    this->timeSS = (pow_2(this->bhLength) / (9.0 * this->soil.diffusivity)) / Constant::rSecsInHour / hrInYear; // Excuse me?
    this->timeSSFactor = this->timeSS * 8760.0;
}

void GLHEVert::combineShortAndLongTimestepGFunctions() const
{
    std::vector<Real64> GFNC_combined;
    std::vector<Real64> LNTTS_combined;

    Real64 const t_s = pow_2(this->bhLength) / (9.0 * this->soil.diffusivity);

    // Nothing to do. Just put the short time step g-functions on the combined vector
    const unsigned int num_shortTimestepGFunctions = GFNC_shortTimestep.size();
    for (size_t index_shortTS = 0; index_shortTS < num_shortTimestepGFunctions; ++index_shortTS) {
        GFNC_combined.push_back(GFNC_shortTimestep[index_shortTS]);
        LNTTS_combined.push_back(LNTTS_shortTimestep[index_shortTS]);
    }

    // the LTS may calculate small values, but let's favor the STS ones up to the high limit of STS calculation
    Real64 const highest_lntts_from_sts = LNTTS_shortTimestep.back();

    // Add the rest of the long time-step g-functions to the combined curve
    for (size_t index_longTS = 0; index_longTS < this->myRespFactors->GFNC.size(); ++index_longTS) {
        if (this->myRespFactors->LNTTS[index_longTS] <= highest_lntts_from_sts) {
            continue;
        }
        GFNC_combined.push_back(this->myRespFactors->GFNC[index_longTS]);
        LNTTS_combined.push_back(this->myRespFactors->LNTTS[index_longTS]);
    }

    this->myRespFactors->time = LNTTS_combined;
    std::transform(this->myRespFactors->time.begin(), this->myRespFactors->time.end(), this->myRespFactors->time.begin(), [&t_s](auto const &c) {
        return exp(c) * t_s;
    });

    this->myRespFactors->LNTTS = LNTTS_combined;
    this->myRespFactors->GFNC = GFNC_combined;
}

std::vector<Real64> GLHEVert::distances(MyCartesian const &point_i, MyCartesian const &point_j)
{
    std::vector<Real64> sumVals;

    // Calculate the distance between points
    sumVals.push_back(pow_2(point_i.x - point_j.x));
    sumVals.push_back(pow_2(point_i.y - point_j.y));
    sumVals.push_back(pow_2(point_i.z - point_j.z));

    Real64 sumTot = 0.0;
    std::vector<Real64> retVals;
    std::for_each(sumVals.begin(), sumVals.end(), [&](Real64 n) { sumTot += n; });
    retVals.push_back(std::sqrt(sumTot));

    // Calculate distance to mirror point
    sumVals.pop_back();
    sumVals.push_back(pow_2(point_i.z - (-point_j.z)));

    sumTot = 0.0;
    std::for_each(sumVals.begin(), sumVals.end(), [&](Real64 n) { sumTot += n; });
    retVals.push_back(std::sqrt(sumTot));

    return retVals;
}

Real64 GLHEVert::calcResponse(std::vector<Real64> const &dists, Real64 const currTime) const
{
    const Real64 pointToPointResponse = erfc(dists[0] / (2 * sqrt(this->soil.diffusivity * currTime))) / dists[0];
    const Real64 pointToReflectedResponse = erfc(dists[1] / (2 * sqrt(this->soil.diffusivity * currTime))) / dists[1];
    return pointToPointResponse - pointToReflectedResponse;
}

Real64 GLHEVert::integral(MyCartesian const &point_i, std::shared_ptr<GLHEVertSingle> const &bh_j, Real64 const currTime) const
{

    // This code could be optimized in a number of ways.
    // The first, most simple way would be to precompute the distances from point i to point j, then store them for reuse.
    // The second, more intensive method would be to break the calcResponse calls out into four different parts.
    // The first point, last point, odd points, and even points. Then multiply the odd/even points by their respective coefficient for the
    // Simpson's method. After that, all points are summed together and divided by 3.

    Real64 sum_f = 0.0;
    int i = 0;
    int const lastIndex_j = static_cast<int>(bh_j->pointLocations_j.size() - 1u);
    for (auto const &point_j : bh_j->pointLocations_j) {
        std::vector<Real64> dists = distances(point_i, point_j);
        Real64 const f = calcResponse(dists, currTime);

        // Integrate using Simpson's
        if (i == 0 || i == lastIndex_j) {
            sum_f += f;
        } else if (isEven(i)) {
            sum_f += 2 * f;
        } else {
            sum_f += 4 * f;
        }

        ++i;
    }

    return (bh_j->dl_j / 3.0) * sum_f;
}

Real64 GLHEVert::doubleIntegral(std::shared_ptr<GLHEVertSingle> const &bh_i, std::shared_ptr<GLHEVertSingle> const &bh_j, Real64 const currTime) const
{

    // Similar optimizations as discussed above could happen here

    if (bh_i == bh_j) {

        Real64 sum_f = 0;
        int i = 0;
        int const lastIndex = static_cast<int>(bh_i->pointLocations_ii.size() - 1u);
        for (auto &thisPoint : bh_i->pointLocations_ii) {

            Real64 f = integral(thisPoint, bh_j, currTime);

            // Integrate using Simpson's
            if (i == 0 || i == lastIndex) {
                sum_f += f;
            } else if (isEven(i)) {
                sum_f += 2 * f;
            } else {
                sum_f += 4 * f;
            }

            ++i;
        }

        return (bh_i->dl_ii / 3.0) * sum_f;

    } else {

        Real64 sum_f = 0;
        int i = 0;
        int const lastIndex = static_cast<int>(bh_i->pointLocations_i.size() - 1u);
        for (auto const &thisPoint : bh_i->pointLocations_i) {

            Real64 f = integral(thisPoint, bh_j, currTime);

            // Integrate using Simpson's
            if (i == 0 || i == lastIndex) {
                sum_f += f;
            } else if (isEven(i)) {
                sum_f += 2 * f;
            } else {
                sum_f += 4 * f;
            }

            ++i;
        }

        return (bh_i->dl_i / 3.0) * sum_f;
    }
}

void GLHEVert::calcLongTimestepGFunctions(EnergyPlusData &state) const
{
    switch (this->gFuncCalcMethod) {
    case GFuncCalcMethod::UniformHeatFlux:
        this->calcUniformHeatFluxGFunctions(state);
        break;
    case GFuncCalcMethod::UniformBoreholeWallTemp:
        this->calcUniformBHWallTempGFunctionsWithGHEDesigner(state);
        break;
    default:
        assert(false);
    }
}

void GLHEVert::calcUniformBHWallTempGFunctionsWithGHEDesigner(EnergyPlusData &state) const
{
    nlohmann::json gheDesignerInputs;
    gheDesignerInputs["version"] = 2; // If you update GHEDesigner, you may need to use a new input version here
    gheDesignerInputs["topology"] = {{{"type", "ground_heat_exchanger"}, {"name", "ghe1"}}};

    std::string const p = fmt::format("[G-Function Calculation for GHE Named: {}] ", this->name);

    // set up the fluid to use in GHEDesigner, note that the concentration is more restrictive than in EnergyPlus
    nlohmann::json fluidObject;
    if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).FluidName == "WATER") {
        gheDesignerInputs["fluid"] = {{"fluid_name", "WATER"}, {"concentration_percent", 0}, {"temperature", 20}};
    } else if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).FluidName == "STEAM") {
        ShowFatalError(state, p + "Detected steam loop, but GHEDesigner cannot run for a steam fluid loop, aborting.");
    } else {
        auto thisGlycol = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol;
        if (auto const &n = thisGlycol->GlycolName; n == "WATER" || n == "ETHYLENEGLYCOL" || n == "PROPYLENEGLYCOL") {
            Real64 c = thisGlycol->Concentration;
            if (c > 0.6) {
                ShowWarningMessage(state, p + "EnergyPlus fluid concentration > 60% (GHEDesigner max), reducing to 60%, continuing");
                c = 0.6;
            }
            gheDesignerInputs["fluid"] = {{"fluid_name", n},
                                          {"concentration_percent", c * 100.0}, // E+ uses 0.0 to 1.0, GHEDesigner uses 0.0 to 100.0
                                          {"temperature", 20}};
        } else {
            ShowFatalError(state, p + "Could not identify glycol for setting up GHEDesigner run");
        }
    }

    // check the heights of the EnergyPlus boreholes to make sure they don't vary
    auto const &bhs = this->myRespFactors->myBorholes;
    auto const &reference = bhs.front();
    bool const allMatch = std::all_of(bhs.begin(), bhs.end(), [reference](const std::shared_ptr<GLHEVertSingle> &bh) {
        return std::fabs(bh->props->bhLength - reference->props->bhLength) <= 0.01;
    });
    if (!allMatch) {
        ShowFatalError(state, p + "Multiple borehole heights in EnergyPlus inputs, g-function generation requires uniform, aborting");
    }
    Real64 const height = this->myRespFactors->myBorholes[0]->props->bhLength;

    // grab thermal and borehole properties
    nlohmann::json grout = {{"conductivity", this->grout.k}, {"rho_cp", this->grout.rhoCp}};
    nlohmann::json soil = {
        {"conductivity", this->soil.k},
        {"rho_cp", this->soil.rhoCp},
        {"undisturbed_temp", this->tempGround},
    };
    Real64 const shankSpacingForGHEDesigner = this->bhUTubeDist - this->pipe.outDia;
    nlohmann::json pipe = {{"inner_diameter", this->pipe.innerDia},
                           {"outer_diameter", this->pipe.outDia},
                           {"shank_spacing", shankSpacingForGHEDesigner},
                           {"roughness", 0.000001}, // TODO: This doesn't seem to be available on inputs
                           {"conductivity", this->pipe.k},
                           {"rho_cp", this->pipe.rhoCp},
                           {"arrangement", "SINGLEUTUBE"}};
    nlohmann::json borehole = {{"buried_depth", this->myRespFactors->props->bhTopDepth}, // TODO: Confirm this is ready for access
                               {"diameter", this->bhDiameter}};

    // generate lists of x, y locations for the boreholes
    std::vector<Real64> x, y;
    for (const auto &bh : this->myRespFactors->myBorholes) {
        x.push_back(bh->xLoc);
        y.push_back(bh->yLoc);
    }
    nlohmann::json preDesigned = {{"arrangement", "MANUAL"}, {"H", height}, {"x", x}, {"y", y}};

    // set up the final borehole structure to fill out the input file
    nlohmann::json ghe1 = {{"flow_rate", this->designMassFlow},
                           {"flow_type", "SYSTEM"},
                           {"grout", grout},
                           {"soil", soil},
                           {"pipe", pipe},
                           {"borehole", borehole},
                           {"pre_designed", preDesigned}};
    gheDesignerInputs["ground_heat_exchanger"] = {{"ghe1", ghe1}};

    // we'll drop the ghedesigner input file and output directory in the same folder as the input file
    auto ghe_designer_input_file_path = state.dataStrGlobals->inputDirPath / "eplus_ghedesigner_input.json";
    auto ghe_designer_output_directory = state.dataStrGlobals->inputDirPath / "eplus_ghedesigner_outputs";
    try {
        // If file already exists, try removing it
        if (fs::exists(ghe_designer_input_file_path)) {
            std::error_code ec;
            if (!fs::remove(ghe_designer_input_file_path, ec)) {
                if (ec) {
                    ShowFatalError(state, "Failed to remove existing GHEDesigner input: " + ec.message());
                }
                // If remove returned false but no error, it wasn't a regular file
                ShowFatalError(state, "Path exists but is not a removable file.");
            }
        }

        // Now create the file fresh
        std::ofstream ghe_designer_input_file(ghe_designer_input_file_path, std::ios::out | std::ios::trunc);
        if (!ghe_designer_input_file) {
            ShowFatalError(state, "Failed to create file: " + ghe_designer_input_file_path.string());
        }
        if (!ghe_designer_input_file.is_open()) {
            ShowFatalError(state, "Failed to open output file");
        }
        ghe_designer_input_file << gheDesignerInputs;
        ghe_designer_input_file.close();

    } catch (const fs::filesystem_error &) {
        ShowFatalError(state, "Filesystem error");
    }

    DisplayString(state, "Starting up GHEDesigner");
    fs::path exePath;
    if (state.dataGlobal->installRootOverride) {
        exePath = state.dataStrGlobals->exeDirectoryPath / "energyplus";
    } else {
        exePath = FileSystem::getAbsolutePath(FileSystem::getProgramPath()); // could be /path/to/energyplus(.exe) or /path/to/energyplus_tests(.exe)
        exePath = exePath.parent_path() / ("energyplus" + FileSystem::exeExtension);
    }
    std::string const cmd = fmt::format(R"("{}" auxiliary ghedesigner "{}" "{}")",
                                        FileSystem::toString(exePath),
                                        FileSystem::toGenericString(ghe_designer_input_file_path),
                                        FileSystem::toGenericString(ghe_designer_output_directory));
    int const status = FileSystem::systemCall(cmd);
    if (status != 0) {
        ShowFatalError(state, "GHEDesigner failed to calculate G-functions.");
    }
    DisplayString(state, "GHEDesigner complete");
    auto output_json_file = ghe_designer_output_directory / "SimulationSummary.json";
    if (!exists(output_json_file)) {
        ShowFatalError(state, "Although GHEDesigner appeared successful, the output file was not found, aborting ");
    }

    std::ifstream file(output_json_file);
    try {
        nlohmann::json data = nlohmann::json::parse(file);
        std::vector<double> t = data["log_time"];
        std::vector<double> g = data["g_values"];
        std::vector<double> gbhw = data["g_bhw_values"];
        this->myRespFactors->time = t;
        this->myRespFactors->LNTTS = t;
        this->myRespFactors->GFNC = g;
    } catch (const nlohmann::json::exception &) {
        ShowFatalError(state, "GHEDesigner completed, and output file found, but could not parse JSON");
    }
}

void GLHEVert::calcGFunctions(EnergyPlusData &state)
{

    // No other choice than to calculate the g-functions here
    // this->calcUniformBHWallTempGFunctions(state);
    this->setupTimeVectors();
    this->calcShortTimestepGFunctions(state);
    this->calcLongTimestepGFunctions(state);
    this->combineShortAndLongTimestepGFunctions();
}

void GLHEVert::setupTimeVectors() const
{
    // Minimum simulation time for which finite line source method is applicable
    constexpr Real64 lntts_min_for_long_timestep = -8.5;

    // Time scale constant
    Real64 const t_s = pow_2(this->bhLength) / (9 * this->soil.diffusivity);

    // Temporary vector for holding the LNTTS vals
    std::vector<Real64> tempLNTTS;

    tempLNTTS.push_back(lntts_min_for_long_timestep);

    // Determine how many g-function pairs to generate based on user defined maximum simulation time
    while (true) {
        const Real64 maxPossibleSimTime = exp(tempLNTTS.back()) * t_s;
        constexpr int numDaysInYear = 365;
        if (maxPossibleSimTime < this->myRespFactors->maxSimYears * numDaysInYear * Constant::rHoursInDay * Constant::rSecsInHour) {
            constexpr Real64 lnttsStepSize = 0.5;
            tempLNTTS.push_back(tempLNTTS.back() + lnttsStepSize);
        } else {
            break;
        }
    }

    this->myRespFactors->LNTTS = tempLNTTS;
    this->myRespFactors->time = tempLNTTS;
    std::transform(this->myRespFactors->time.begin(), this->myRespFactors->time.end(), this->myRespFactors->time.begin(), [&t_s](auto const &c) {
        return exp(c) * t_s;
    });
    this->myRespFactors->GFNC = std::vector<Real64>(tempLNTTS.size(), 0.0);
}

void GLHEVert::calcUniformHeatFluxGFunctions(EnergyPlusData &state) const
{
    DisplayString(state, "Initializing GroundHeatExchanger:System: " + this->name);

    // Calculate the g-functions
    for (size_t lntts_index = 0; lntts_index < this->myRespFactors->LNTTS.size(); ++lntts_index) {
        for (auto const &bh_i : this->myRespFactors->myBorholes) {
            Real64 sum_T_ji = 0;
            for (auto const &bh_j : this->myRespFactors->myBorholes) {
                sum_T_ji += doubleIntegral(bh_i, bh_j, this->myRespFactors->time[lntts_index]);
            }
            this->myRespFactors->GFNC[lntts_index] += sum_T_ji;
        }
        this->myRespFactors->GFNC[lntts_index] /= (2 * this->totalTubeLength);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << static_cast<Real64>(lntts_index) / static_cast<Real64>(this->myRespFactors->LNTTS.size()) * 100.0;

        DisplayString(state, "...progress: " + ss.str() + "%");
    }
}

void GLHEVert::calcShortTimestepGFunctions(EnergyPlusData &state)
{
    // SUBROUTINE PARAMETER DEFINITIONS:
    std::string_view constexpr RoutineName = "calcShortTimestepGFunctions";

    enum class CellType
    {
        Invalid = -1,
        FLUID,
        CONVECTION,
        PIPE,
        GROUT,
        SOIL,
        Num
    };

    struct Cell
    {
        // NOTE, in the previous code, this was value-initialized in the initializer list for the
        // struct constructor like this:  Cell() : type() ... {}
        // During value initialization, the underlying integral memory will be initialized to zero
        // This will result in the default value being set to FLUID based on the way the enum is
        // set up above.  I'm putting fluid here, and maybe it doesn't matter, but maybe it should
        // be invalid?
        CellType type = CellType::FLUID;
        Real64 radius_center = 0.0;
        Real64 radius_outer = 0.0;
        Real64 radius_inner = 0.0;
        Real64 thickness = 0.0;
        Real64 vol = 0.0;
        Real64 conductivity = 0.0;
        Real64 rhoCp = 0.0;
        Real64 temperature = 0.0;
        Real64 temperature_prev_ts = 0.0;
    };

    // vector to hold 1-D cells
    std::vector<Cell> Cells; // It may be worthwhile to make this a stack Array since we appear to know the size ahead of time (535?)

    // setup pipe, convection, and fluid layer geometries
    constexpr int num_pipe_cells = 4;
    constexpr int num_conv_cells = 1;
    constexpr int num_fluid_cells = 3;
    Real64 const pipe_thickness = this->pipe.thickness;
    Real64 const pcf_cell_thickness = pipe_thickness / num_pipe_cells;
    Real64 const radius_pipe_out = std::sqrt(2) * this->pipe.outRadius;
    Real64 const radius_pipe_in = radius_pipe_out - pipe_thickness;
    Real64 const radius_conv = radius_pipe_in - num_conv_cells * pcf_cell_thickness;
    Real64 const radius_fluid = radius_conv - (num_fluid_cells - 0.5) * pcf_cell_thickness; // accounts for half thickness of boundary cell

    // setup grout layer geometry
    constexpr int num_grout_cells = 27;
    Real64 const radius_grout = this->bhRadius;
    Real64 const grout_cell_thickness = (radius_grout - radius_pipe_out) / num_grout_cells;

    // setup soil layer geometry
    constexpr int num_soil_cells = 500;
    constexpr Real64 radius_soil = 10.0;
    Real64 const soil_cell_thickness = (radius_soil - radius_grout) / num_soil_cells;

    // use design flow rate
    this->massFlowRate = this->designMassFlow;

    // calculate equivalent thermal resistance between borehole wall and fluid
    Real64 bhResistance = calcBHAverageResistance(state);
    Real64 bhConvectionResistance = calcPipeConvectionResistance(state);
    Real64 bh_equivalent_resistance_tube_grout = bhResistance - bhConvectionResistance / 2.0;
    Real64 bh_equivalent_resistance_convection = bhResistance - bh_equivalent_resistance_tube_grout;

    Real64 initial_temperature = this->inletTemp;
    Real64 cpFluid_init = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, initial_temperature, RoutineName);
    Real64 fluidDensity_init = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, initial_temperature, RoutineName);

    // initialize the fluid cells
    for (int i = 0; i < num_fluid_cells; ++i) {
        Cell thisCell;
        thisCell.type = CellType::FLUID;
        thisCell.thickness = pcf_cell_thickness;
        thisCell.radius_center = radius_fluid + i * thisCell.thickness;

        // boundary cell is only half thickness
        if (i == 0) {
            thisCell.radius_inner = thisCell.radius_center;
        } else {
            thisCell.radius_inner = thisCell.radius_center - thisCell.thickness / 2.0;
        }

        thisCell.radius_outer = thisCell.radius_center + thisCell.thickness / 2.0;
        thisCell.conductivity = 200;
        thisCell.rhoCp = 2.0 * cpFluid_init * fluidDensity_init * pow_2(this->pipe.innerRadius) / (pow_2(radius_conv) - pow_2(radius_fluid));
        Cells.push_back(thisCell);
    }

    // initialize the convection cells
    for (int i = 0; i < num_conv_cells; ++i) {
        Cell thisCell;
        thisCell.thickness = pcf_cell_thickness;
        thisCell.radius_inner = radius_conv + i * thisCell.thickness;
        thisCell.radius_center = thisCell.radius_inner + thisCell.thickness / 2.0;
        thisCell.radius_outer = thisCell.radius_inner + thisCell.thickness;
        thisCell.conductivity = log(radius_pipe_in / radius_conv) / (2 * Constant::Pi * bh_equivalent_resistance_convection);
        thisCell.rhoCp = 1;
        Cells.push_back(thisCell);
    }

    // initialize pipe cells
    for (int i = 0; i < num_pipe_cells; ++i) {
        Cell thisCell;
        thisCell.type = CellType::PIPE;
        thisCell.thickness = pcf_cell_thickness;
        thisCell.radius_inner = radius_pipe_in + i * thisCell.thickness;
        thisCell.radius_center = thisCell.radius_inner + thisCell.thickness / 2.0;
        thisCell.radius_outer = thisCell.radius_inner + thisCell.thickness;
        thisCell.conductivity = log(radius_grout / radius_pipe_in) / (2 * Constant::Pi * bh_equivalent_resistance_tube_grout);
        thisCell.rhoCp = this->pipe.rhoCp;
        Cells.push_back(thisCell);
    }

    // initialize grout cells
    for (int i = 0; i < num_grout_cells; ++i) {
        Cell thisCell;
        thisCell.type = CellType::GROUT;
        thisCell.thickness = grout_cell_thickness;
        thisCell.radius_inner = radius_pipe_out + i * thisCell.thickness;
        thisCell.radius_center = thisCell.radius_inner + thisCell.thickness / 2.0;
        thisCell.radius_outer = thisCell.radius_inner + thisCell.thickness;
        thisCell.conductivity = log(radius_grout / radius_pipe_in) / (2 * Constant::Pi * bh_equivalent_resistance_tube_grout);
        thisCell.rhoCp = grout.rhoCp;
        Cells.push_back(thisCell);
    }

    // initialize soil cells
    for (int i = 0; i < num_soil_cells; ++i) {
        Cell thisCell;
        thisCell.type = CellType::SOIL;
        thisCell.thickness = soil_cell_thickness;
        thisCell.radius_inner = radius_grout + i * thisCell.thickness;
        thisCell.radius_center = thisCell.radius_inner + thisCell.thickness / 2.0;
        thisCell.radius_outer = thisCell.radius_inner + thisCell.thickness;
        thisCell.conductivity = this->soil.k;
        thisCell.rhoCp = this->soil.rhoCp;
        Cells.push_back(thisCell);
    }

    // other non-geometric specific setup
    for (auto &thisCell : Cells) {
        thisCell.vol = Constant::Pi * (pow_2(thisCell.radius_outer) - pow_2(thisCell.radius_inner));
        thisCell.temperature = initial_temperature;
    }

    // set upper limit of time for the short time-step g-function calcs so there is some overlap
    Real64 constexpr lntts_max_for_short_timestep = -8.6;
    Real64 const t_s = pow_2(this->bhLength) / (9.0 * this->soil.diffusivity);

    Real64 const time_max_for_short_timestep = exp(lntts_max_for_short_timestep) * t_s;
    Real64 total_time = 0.0;

    // time step loop
    while (total_time < time_max_for_short_timestep) {
        Real64 constexpr heat_flux = 1.0; // 40.4;
        Real64 constexpr time_step = 120; // 500.0;

        for (auto &thisCell : Cells) {
            thisCell.temperature_prev_ts = thisCell.temperature;
        }

        std::vector<Real64> a;
        std::vector<Real64> b;
        std::vector<Real64> c;
        std::vector<Real64> d;

        // setup tdma matrices
        unsigned int num_cells = Cells.size();
        for (size_t cell_index = 0; cell_index < num_cells; ++cell_index) {
            if (cell_index == 0) {
                // heat flux BC

                auto const &thisCell = Cells[cell_index];
                auto const &eastCell = Cells[cell_index + 1];

                Real64 FE1 = log(thisCell.radius_outer / thisCell.radius_center) / (2 * Constant::Pi * thisCell.conductivity);
                Real64 FE2 = log(eastCell.radius_center / eastCell.radius_inner) / (2 * Constant::Pi * eastCell.conductivity);
                Real64 AE = 1 / (FE1 + FE2);

                Real64 AD = thisCell.rhoCp * thisCell.vol / time_step;

                a.push_back(0);
                b.push_back(-AE / AD - 1);
                c.push_back(AE / AD);
                d.push_back(-thisCell.temperature_prev_ts - heat_flux / AD);

            } else if (cell_index == num_cells - 1) {
                // const ground temp bc

                auto &thisCell = Cells[cell_index];

                a.push_back(0);
                b.push_back(1);
                c.push_back(0);
                d.push_back(thisCell.temperature_prev_ts);

            } else {
                // all other cells

                auto const &westCell = Cells[cell_index - 1];
                auto const &eastCell = Cells[cell_index + 1];
                auto const &thisCell = Cells[cell_index];

                Real64 FE1 = log(thisCell.radius_outer / thisCell.radius_center) / (2 * Constant::Pi * thisCell.conductivity);
                Real64 FE2 = log(eastCell.radius_center / eastCell.radius_inner) / (2 * Constant::Pi * eastCell.conductivity);
                Real64 AE = 1 / (FE1 + FE2);

                Real64 FW1 = log(westCell.radius_outer / westCell.radius_center) / (2 * Constant::Pi * westCell.conductivity);
                Real64 FW2 = log(thisCell.radius_center / thisCell.radius_inner) / (2 * Constant::Pi * thisCell.conductivity);
                Real64 AW = -1 / (FW1 + FW2);

                Real64 AD = thisCell.rhoCp * thisCell.vol / time_step;

                a.push_back(-AW / AD);
                b.push_back(AW / AD - AE / AD - 1);
                c.push_back(AE / AD);
                d.push_back(-thisCell.temperature_prev_ts);
            }
        } // end tdma setup

        // solve for new temperatures
        std::vector<Real64> new_temps = TDMA(a, b, c, d);

        for (size_t cell_index = 0; cell_index < num_cells; ++cell_index) {
            Cells[cell_index].temperature = new_temps[cell_index];
        }

        // The T_bhWall variable was only ever calculated, ever actually used.  I'm commenting it out for now.
        // // calculate bh wall temp
        // Real64 T_bhWall = 0.0;
        // for (int cell_index = 0; cell_index < num_cells; ++cell_index) {
        //     auto const &leftCell = Cells[cell_index];
        //     auto const &rightCell = Cells[cell_index + 1];
        //
        //     if (leftCell.type == CellType::GROUT && rightCell.type == CellType::SOIL) {
        //
        //         Real64 left_conductance = 2 * Constant::Pi * leftCell.conductivity / log(leftCell.radius_outer / leftCell.radius_inner);
        //         Real64 right_conductance = 2 * Constant::Pi * rightCell.conductivity / log(rightCell.radius_center / leftCell.radius_inner);
        //
        //         T_bhWall =
        //             (left_conductance * leftCell.temperature + right_conductance * rightCell.temperature) / (left_conductance + right_conductance);
        //         break;
        //     }
        // }

        total_time += time_step;

        GFNC_shortTimestep.push_back(2 * Constant::Pi * this->soil.k * ((Cells[0].temperature - initial_temperature) / heat_flux - bhResistance));
        LNTTS_shortTimestep.push_back(log(total_time / t_s));
    } // end timestep loop

    std::ostringstream l, g;
    for (auto const val : LNTTS_shortTimestep) {
        l << val << '\n';
    }
    for (auto const val : GFNC_shortTimestep) {
        g << val << '\n';
    }
    std::string l2 = l.str();
    std::string g2 = g.str();
}

Real64 GLHEVert::calcBHAverageResistance(EnergyPlusData &state)
{
    // Calculates the average thermal resistance of the borehole using the first-order multipole method.
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' Applied Energy.187:790-806.
    // Equation 13

    Real64 const beta = 2 * Constant::Pi * this->grout.k * calcPipeResistance(state);

    Real64 const final_term_1 = log(this->theta_2 / (2 * this->theta_1 * pow(1 - pow_4(this->theta_1), this->sigma)));
    Real64 const num_final_term_2 = pow_2(this->theta_3) * pow_2(1 - (4 * this->sigma * pow_4(this->theta_1)) / (1 - pow_4(this->theta_1)));
    Real64 const den_final_term_2_pt_1 = (1 + beta) / (1 - beta);
    Real64 const den_final_term_2_pt_2 = pow_2(this->theta_3) * (1 + (16 * this->sigma * pow_4(this->theta_1)) / pow_2(1 - pow_4(this->theta_1)));
    Real64 const den_final_term_2 = den_final_term_2_pt_1 + den_final_term_2_pt_2;
    Real64 const final_term_2 = num_final_term_2 / den_final_term_2;

    return 1 / (4 * Constant::Pi * this->grout.k) * (beta + final_term_1 - final_term_2);
}

Real64 GLHEVert::calcBHTotalInternalResistance(EnergyPlusData &state)
{
    // Calculates the total internal thermal resistance of the borehole using the first-order multipole method.
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' Applied Energy. 187:790-806.
    // Equation 26

    const Real64 beta = 2 * Constant::Pi * this->grout.k * calcPipeResistance(state);

    const Real64 final_term_1 = log(pow(1 + pow_2(this->theta_1), this->sigma) / (this->theta_3 * pow(1 - pow_2(this->theta_1), this->sigma)));
    const Real64 num_term_2 = pow_2(this->theta_3) * pow_2(1 - pow_4(this->theta_1) + 4 * this->sigma * pow_2(this->theta_1));
    const Real64 den_term_2_pt_1 = (1 + beta) / (1 - beta) * pow_2(1 - pow_4(this->theta_1));
    const Real64 den_term_2_pt_2 = pow_2(this->theta_3) * pow_2(1 - pow_4(this->theta_1));
    const Real64 den_term_2_pt_3 = 8 * this->sigma * pow_2(this->theta_1) * pow_2(this->theta_3) * (1 + pow_4(this->theta_1));
    const Real64 den_term_2 = den_term_2_pt_1 - den_term_2_pt_2 + den_term_2_pt_3;
    const Real64 final_term_2 = num_term_2 / den_term_2;

    return 1 / (Constant::Pi * this->grout.k) * (beta + final_term_1 - final_term_2);
}

Real64 GLHEVert::calcBHGroutResistance(EnergyPlusData &state)
{
    // Calculates grout resistance. Use for validation.
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' Applied Energy. 187:790-806.
    // Equation 3

    return calcBHAverageResistance(state) - calcPipeResistance(state) / 2.0;
}

Real64 GLHEVert::calcHXResistance(EnergyPlusData &state)
{
    // Calculates the effective thermal resistance of the borehole assuming a uniform heat flux.
    // Javed, S. & Spitler, J.D. Calculation of Borehole Thermal Resistance. In 'Advances in
    // Ground-Source Heat Pump Systems,' pp. 84. Rees, S.J. ed. Cambridge, MA. Elsevier Ltd. 2016.
    // Eq: 3-67

    if (this->massFlowRate <= 0.0) {
        return 0;
    }
    constexpr std::string_view RoutineName = "calcBHResistance";
    Real64 const cpFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->inletTemp, RoutineName);
    return calcBHAverageResistance(state) + 1 / (3 * calcBHTotalInternalResistance(state)) * pow_2(this->bhLength / (this->massFlowRate * cpFluid));
}

Real64 GLHEVert::calcPipeConductionResistance() const
{
    // Calculates the thermal resistance of a pipe, in [K/(W/m)].
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' Applied Energy. 187:790-806.

    return log(this->pipe.outDia / this->pipe.innerDia) / (2 * Constant::Pi * this->pipe.k);
}

Real64 GLHEVert::calcPipeConvectionResistance(EnergyPlusData &state)
{
    // Calculates the convection resistance using Gnielinski and Petukov, in [K/(W/m)]
    // Gneilinski, V. 1976. 'New equations for heat and mass transfer in turbulent pipe and channel flow.'
    // International Chemical Engineering 16(1976), pp. 359-368.

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr std::string_view RoutineName("calcPipeConvectionResistance");

    // Get fluid props
    this->inletTemp = state.dataLoopNodes->Node(this->inletNodeNum).Temp;

    Real64 const cpFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->inletTemp, RoutineName);
    Real64 const kFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getConductivity(state, this->inletTemp, RoutineName);
    Real64 const fluidViscosity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getViscosity(state, this->inletTemp, RoutineName);

    // Smoothing fit limits
    constexpr Real64 lower_limit = 2000;
    constexpr Real64 upper_limit = 4000;

    Real64 const bhMassFlowRate = this->massFlowRate / this->myRespFactors->numBoreholes;
    Real64 const reynoldsNum = 4 * bhMassFlowRate / (fluidViscosity * Constant::Pi * this->pipe.innerDia);

    Real64 nusseltNum = 0.0;
    if (reynoldsNum < lower_limit) {
        nusseltNum = 4.01; // laminar mean(4.36, 3.66)
    } else if (reynoldsNum < upper_limit) {
        Real64 constexpr nu_low = 4.01;               // laminar
        Real64 const f = frictionFactor(reynoldsNum); // turbulent
        Real64 const prandtlNum = (cpFluid * fluidViscosity) / (kFluid);
        Real64 const nu_high = (f / 8) * (reynoldsNum - 1000) * prandtlNum / (1 + 12.7 * std::sqrt(f / 8) * (pow(prandtlNum, 2.0 / 3.0) - 1));
        Real64 const sf = 1 / (1 + std::exp(-(reynoldsNum - 3000) / 150.0)); // smoothing function
        nusseltNum = (1 - sf) * nu_low + sf * nu_high;
    } else {
        Real64 const f = frictionFactor(reynoldsNum);
        Real64 const prandtlNum = (cpFluid * fluidViscosity) / (kFluid);
        nusseltNum = (f / 8) * (reynoldsNum - 1000) * prandtlNum / (1 + 12.7 * std::sqrt(f / 8) * (pow(prandtlNum, 2.0 / 3.0) - 1));
    }

    const Real64 h = nusseltNum * kFluid / this->pipe.innerDia;

    return 1 / (h * Constant::Pi * this->pipe.innerDia);
}

Real64 GLHEVert::frictionFactor(Real64 const reynoldsNum)
{
    // Calculates the friction factor in smooth tubes
    // Petukov, B.S. 1970. 'Heat transfer and friction in turbulent pipe flow with variable physical properties.'
    // In Advances in Heat Transfer, ed. T.F. Irvine and J.P. Hartnett, Vol. 6. New York Academic Press.

    // limits picked to be within about 1% of actual values
    constexpr Real64 lower_limit = 1500;
    constexpr Real64 upper_limit = 5000;

    if (reynoldsNum < lower_limit) {
        return 64.0 / reynoldsNum; // pure laminar flow
    }
    if (reynoldsNum < upper_limit) {
        // pure laminar flow
        Real64 const f_low = 64.0 / reynoldsNum;
        // pure turbulent flow
        Real64 const f_high = pow(0.79 * log(reynoldsNum) - 1.64, -2.0);
        Real64 const sf = 1 / (1 + exp(-(reynoldsNum - 3000.0) / 450.0)); // smoothing function
        return (1 - sf) * f_low + sf * f_high;
    }
    return pow(0.79 * log(reynoldsNum) - 1.64, -2.0); // pure turbulent flow
}

Real64 GLHEVert::calcPipeResistance(EnergyPlusData &state)
{
    // Calculates the combined conduction and convection pipe resistance
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' J. Energy Engineering. Draft in progress.
    // Equation 3

    return calcPipeConductionResistance() + calcPipeConvectionResistance(state);
}

Real64 GLHEVert::getGFunc(Real64 const time)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Gets the g-function for vertical GHXs Note: Base e here.

    const Real64 LNTTS = std::log(time);
    Real64 gFuncVal = interpGFunc(LNTTS);
    const Real64 RATIO = this->bhRadius / this->bhLength;

    if (RATIO != this->myRespFactors->gRefRatio) {
        gFuncVal -= std::log(this->bhRadius / (this->bhLength * this->myRespFactors->gRefRatio));
    }

    return gFuncVal;
}

void GLHEVert::initGLHESimVars(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    August, 2000
    //       MODIFIED         Arun Murugappan

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 currTime = ((state.dataGlobal->DayOfSim - 1) * 24 + (state.dataGlobal->HourOfDay - 1) +
                       (state.dataGlobal->TimeStep - 1) * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed) *
                      Constant::rSecsInHour;

    if (this->myEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
        this->initEnvironment(state, currTime);
    }

    // Calculate the average ground temperature over the depth of the borehole
    Real64 minDepth = this->myRespFactors->props->bhTopDepth;
    Real64 maxDepth = this->myRespFactors->props->bhLength + minDepth;
    Real64 oneQuarterDepth = minDepth + (maxDepth - minDepth) * 0.25;
    Real64 halfDepth = minDepth + (maxDepth - minDepth) * 0.5;
    Real64 threeQuarterDepth = minDepth + (maxDepth - minDepth) * 0.75;

    this->tempGround = 0;

    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, minDepth, currTime);
    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, maxDepth, currTime);
    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, oneQuarterDepth, currTime);
    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, halfDepth, currTime);
    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, threeQuarterDepth, currTime);

    this->tempGround /= 5;

    this->massFlowRate = PlantUtilities::RegulateCondenserCompFlowReqOp(state, this->plantLoc, this->designMassFlow);

    PlantUtilities::SetComponentFlowRate(state, this->massFlowRate, this->inletNodeNum, this->outletNodeNum, this->plantLoc);

    // Reset local environment init flag
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->myEnvrnFlag = true;
    }
}

void GLHEVert::initEnvironment(EnergyPlusData &state, [[maybe_unused]] Real64 const CurTime)
{
    constexpr std::string_view RoutineName = "initEnvironment";
    this->myEnvrnFlag = false;

    Real64 fluidDensity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, 20.0, RoutineName);
    this->designMassFlow = this->designFlow * fluidDensity;
    PlantUtilities::InitComponentNodes(state, 0.0, this->designMassFlow, this->inletNodeNum, this->outletNodeNum);

    this->lastQnSubHr = 0.0;
    state.dataLoopNodes->Node(this->inletNodeNum).Temp = this->tempGround;
    state.dataLoopNodes->Node(this->outletNodeNum).Temp = this->tempGround;

    // zero out all history arrays
    this->QnHr = 0.0;
    this->QnMonthlyAgg = 0.0;
    this->QnSubHr = 0.0;
    this->LastHourN = 0;
    this->prevTimeSteps = 0.0;
    this->currentSimTime = 0.0;
    this->QGLHE = 0.0;
    this->prevHour = 1;
}

void GLHEVert::oneTimeInit_new(EnergyPlusData &state)
{
    // Locate the hx on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(
        state, this->name, DataPlant::PlantEquipmentType::GrndHtExchgSystem, this->plantLoc, errFlag, _, _, _, _, _);
    if (errFlag) {
        ShowFatalError(state, "initGLHESimVars: Program terminated due to previous condition(s).");
    }
}

void GLHEVert::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}

} // namespace EnergyPlus::GroundHeatExchangers
