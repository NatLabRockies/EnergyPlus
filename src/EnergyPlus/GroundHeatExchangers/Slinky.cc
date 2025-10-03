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
#include <EnergyPlus/GroundHeatExchangers/Slinky.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::GroundHeatExchangers {
GLHESlinky::GLHESlinky(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j)
{
    // Check for duplicates
    for (const auto &existingObj : state.dataGroundHeatExchanger->singleBoreholesVector) {
        if (objName == existingObj->name) {
            ShowFatalError(state, format("Invalid input for {} object: Duplicate name found: {}", this->moduleName, existingObj->name));
        }
    }

    bool errorsFound = false;

    this->name = objName;

    std::string inletNodeName = Util::makeUPPER(j["inlet_node_name"].get<std::string>());
    std::string outletNodeName = Util::makeUPPER(j["outlet_node_name"].get<std::string>());

    // get inlet node num
    this->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                             inletNodeName,
                                                             errorsFound,
                                                             DataLoopNode::ConnectionObjectType::GroundHeatExchangerSlinky,
                                                             this->name,
                                                             DataLoopNode::NodeFluidType::Water,
                                                             DataLoopNode::ConnectionType::Inlet,
                                                             NodeInputManager::CompFluidStream::Primary,
                                                             DataLoopNode::ObjectIsNotParent);

    // get outlet node num
    this->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                              outletNodeName,
                                                              errorsFound,
                                                              DataLoopNode::ConnectionObjectType::GroundHeatExchangerSlinky,
                                                              this->name,
                                                              DataLoopNode::NodeFluidType::Water,
                                                              DataLoopNode::ConnectionType::Outlet,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              DataLoopNode::ObjectIsNotParent);

    this->available = true;
    this->on = true;

    BranchNodeConnections::TestCompSet(state, this->moduleName, this->name, inletNodeName, outletNodeName, "Condenser Water Nodes");

    // load data
    this->designFlow = j["design_flow_rate"].get<Real64>();
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->inletNodeNum, this->designFlow);

    this->soil.k = j["soil_thermal_conductivity"].get<Real64>();
    this->soil.rho = j["soil_density"].get<Real64>();
    this->soil.cp = j["soil_specific_heat"].get<Real64>();
    this->soil.rhoCp = this->soil.rho * this->soil.cp;
    this->pipe.k = j["pipe_thermal_conductivity"].get<Real64>();
    this->pipe.rho = j["pipe_density"].get<Real64>();
    this->pipe.cp = j["pipe_specific_heat"].get<Real64>();
    this->pipe.outDia = j["pipe_outer_diameter"].get<Real64>();
    this->pipe.outRadius = this->pipe.outDia / 2.0;
    this->pipe.thickness = j["pipe_thickness"].get<Real64>();

    std::string const hxConfig = Util::makeUPPER(j["heat_exchanger_configuration"].get<std::string>());
    if (Util::SameString(hxConfig, "VERTICAL")) {
        this->verticalConfig = true;
    } else if (Util::SameString(hxConfig, "HORIZONTAL")) {
        this->verticalConfig = false;
    }

    this->coilDiameter = j["coil_diameter"].get<Real64>();
    this->coilPitch = j["coil_pitch"].get<Real64>();
    this->trenchDepth = j["trench_depth"].get<Real64>();
    this->trenchLength = j["trench_length"].get<Real64>();
    this->numTrenches = j["number_of_trenches"].get<int>();
    this->trenchSpacing = j["horizontal_spacing_between_pipes"].get<Real64>();
    this->maxSimYears = j["maximum_length_of_simulation"].get<Real64>();

    // Need to add a response factor object for the slinky model
    std::shared_ptr<GLHEResponseFactors> thisRF(new GLHEResponseFactors);
    thisRF->name = "Response Factor Object Auto Generated No: " + fmt::to_string(state.dataGroundHeatExchanger->numAutoGeneratedResponseFactors + 1);
    this->myRespFactors = thisRF;
    state.dataGroundHeatExchanger->responseFactorsVector.push_back(thisRF);

    // Number of coils
    this->numCoils = static_cast<int>(this->trenchLength / this->coilPitch);

    // Total tube length
    this->totalTubeLength = Constant::Pi * this->coilDiameter * this->trenchLength * this->numTrenches / this->coilPitch;

    // Get g function data
    this->SubAGG = 15;
    this->AGG = 192;

    // Average coil depth
    if (this->verticalConfig) {
        // Vertical configuration
        if (this->trenchDepth - this->coilDiameter < 0.0) {
            // Error: part of the coil is above ground
            ShowSevereError(state, format("{}=\"{}\", invalid value in field.", this->moduleName, this->name));
            ShowContinueError(state, format("...{}=[{:.3R}].", "Trench Depth", this->trenchDepth));
            ShowContinueError(state, format("...{}=[{:.3R}].", "Coil Depth", this->coilDepth));
            ShowContinueError(state, "...Part of coil will be above ground.");
            errorsFound = true;

        } else {
            // Entire coil is below ground
            this->coilDepth = this->trenchDepth - (this->coilDiameter / 2.0);
        }

    } else {
        // Horizontal configuration
        this->coilDepth = this->trenchDepth;
    }

    // Thermal diffusivity of the ground
    this->soil.diffusivity = this->soil.k / this->soil.rhoCp;

    this->prevTimeSteps.allocate(static_cast<int>((this->SubAGG + 1) * maxTSinHr + 1));
    this->prevTimeSteps = 0.0;

    if (this->pipe.thickness >= this->pipe.outDia / 2.0) {
        ShowSevereError(state, format("{}=\"{}\", invalid value in field.", this->moduleName, this->name));
        ShowContinueError(state, format("...{}=[{:.3R}].", "Pipe Thickness", this->pipe.thickness));
        ShowContinueError(state, format("...{}=[{:.3R}].", "Pipe Outer Diameter", this->pipe.outDia));
        ShowContinueError(state, "...Radius will be <=0.");
        errorsFound = true;
    }

    // Initialize ground temperature model and get pointer reference
    GroundTemp::ModelType gtmType = static_cast<GroundTemp::ModelType>(
        getEnumValue(GroundTemp::modelTypeNamesUC, Util::makeUPPER(j["undisturbed_ground_temperature_model_type"].get<std::string>())));
    assert(gtmType != GroundTemp::ModelType::Invalid);

    std::string const gtmName = Util::makeUPPER(j["undisturbed_ground_temperature_model_name"].get<std::string>());
    this->groundTempModel = GroundTemp::GetGroundTempModelAndInit(state, gtmType, gtmName);

    // Check for Errors
    if (errorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", this->moduleName));
    }
}

void GLHESlinky::getAnnualTimeConstant()
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // calculate annual time constant for ground conduction

    this->timeSSFactor = 1.0;
}

Real64 GLHESlinky::doubleIntegral(int const m, int const n, int const m1, int const n1, Real64 const t, int const I0, int const J0)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Integrates the temperature response at one point based on
    // input from other points

    // METHODOLOGY EMPLOYED:
    // Simpson's 1/3 rule of integration

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    constexpr Real64 eta1 = 0.0;
    constexpr Real64 eta2 = 2 * Constant::Pi;

    std::vector<Real64> g;

    Real64 h = (eta2 - eta1) / (I0 - 1);

    // Calculates the value of the function at various equally spaced values
    for (int i = 0; i < I0; ++i) {
        Real64 eta = eta1 + i * h;
        g.push_back(integral(m, n, m1, n1, t, eta, J0));
    }

    for (size_t i = 1; i < g.size() - 1; ++i) {
        if (!isEven(i)) {
            g[i] = 4 * g[i];
        } else {
            g[i] = 2 * g[i];
        }
    }

    return (h / 3) * std::reduce(g.begin(), g.end());
}

void GLHESlinky::calcGFunctions(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // calculates g-functions for the slinky ground heat exchanger model

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    constexpr Real64 tLg_min = -2.0;
    constexpr Real64 tLg_grid = 0.25;
    constexpr Real64 ts = 3600.0;
    constexpr Real64 convertYearsToSeconds = 356.0 * 24.0 * 60.0 * 60.0;
    Real64 fraction;
    Array2D<Real64> valStored({0, this->numTrenches}, {0, this->numCoils}, -1.0);
    int I0;
    int J0;

    DisplayString(state, "Initializing GroundHeatExchanger:Slinky: " + this->name);

    this->X0.allocate(this->numCoils);
    this->Y0.allocate(this->numTrenches);

    // Calculate the number of g-functions required
    Real64 tLg_max = std::log10(this->maxSimYears * convertYearsToSeconds / ts);
    int NPairs = static_cast<int>((tLg_max - tLg_min) / (tLg_grid) + 1);

    // Allocate and setup g-function arrays
    this->myRespFactors->GFNC = std::vector<Real64>(NPairs, 0.0);
    this->myRespFactors->LNTTS = std::vector<Real64>(NPairs, 0.0);
    this->QnMonthlyAgg.allocate(static_cast<int>(this->maxSimYears * 12));
    this->QnHr.allocate(730 + this->AGG + this->SubAGG);
    this->QnSubHr.allocate(static_cast<int>((this->SubAGG + 1) * maxTSinHr + 1));
    this->LastHourN.allocate(this->SubAGG + 1);

    // Calculate the number of loops (per trench) and number of trenches to be involved
    // Due to the symmetry of a slinky GHX field, we need only calculate about
    // on quarter of the rings' tube wall temperature perturbation to get the
    // mean wall temperature perturbation of the entire slinky GHX field.
    int numLC = std::ceil(this->numCoils / 2.0);
    int numRC = std::ceil(this->numTrenches / 2.0);

    // Calculate coordinates (X0, Y0, Z0) of a ring's center
    for (int coil = 1; coil <= this->numCoils; ++coil) {
        this->X0(coil) = coilPitch * (coil - 1);
    }
    for (int trench = 1; trench <= this->numTrenches; ++trench) {
        this->Y0(trench) = (trench - 1) * this->trenchSpacing;
    }
    this->Z0 = this->coilDepth;

    // If number of trenches is greater than 1, one quarter of the rings are involved.
    // If number of trenches is 1, one half of the rings are involved.
    if (this->numTrenches > 1) {
        fraction = 0.25;
    } else {
        fraction = 0.5;
    }

    // Calculate the corresponding time of each temperature response factor
    for (int NT = 1; NT <= NPairs; ++NT) {
        Real64 tLg = tLg_min + tLg_grid * (NT - 1);
        Real64 t = std::pow(10, tLg) * ts;

        // Set the average temperature response of the whole field to zero
        Real64 gFunc = 0;

        valStored = -1.0;

        for (int m1 = 1; m1 <= numRC; ++m1) {
            for (int n1 = 1; n1 <= numLC; ++n1) {
                for (int m = 1; m <= this->numTrenches; ++m) {
                    for (int n = 1; n <= this->numCoils; ++n) {

                        // Zero out val after each iteration
                        Real64 doubleIntegralVal = 0.0;
                        Real64 midFieldVal = 0.0;

                        // Calculate the distance between ring centers
                        Real64 disRing = distToCenter(m, n, m1, n1);

                        // Save mm1 and nn1
                        int mm1 = std::abs(m - m1);
                        int nn1 = std::abs(n - n1);

                        // If we're calculating a ring's temperature response to itself as a ring source,
                        // then we need some extra effort in calculating the double integral
                        if (m1 == m && n1 == n) {
                            I0 = 33;
                            J0 = 1089;
                        } else {
                            I0 = 33;
                            J0 = 561;
                        }

                        Real64 gFuncin;

                        // if the ring(n1, m1) is the near-field ring of the ring(n,m)
                        if (disRing <= 2.5 + this->coilDiameter) {
                            // if no calculated value has been stored
                            if (valStored(mm1, nn1) < 0) {
                                doubleIntegralVal = doubleIntegral(m, n, m1, n1, t, I0, J0);
                                valStored(mm1, nn1) = doubleIntegralVal;
                                // else: if a stored value is found for the combination of (m, n, m1, n1)
                            } else {
                                doubleIntegralVal = valStored(mm1, nn1);
                            }

                            // due to symmetry, the temperature response of ring(n1, m1) should be 0.25, 0.5, or 1 times its calculated value
                            if (!isEven(this->numTrenches) && !isEven(this->numCoils) && m1 == numRC && n1 == numLC && this->numTrenches > 1.5) {
                                gFuncin = 0.25 * doubleIntegralVal;
                            } else if (!isEven(this->numTrenches) && m1 == numRC && this->numTrenches > 1.5) {
                                gFuncin = 0.5 * doubleIntegralVal;
                            } else if (!isEven(this->numCoils) && n1 == numLC) {
                                gFuncin = 0.5 * doubleIntegralVal;
                            } else {
                                gFuncin = doubleIntegralVal;
                            }

                            // if the ring(n1, m1) is in the far-field or the ring(n,m)
                        } else if (disRing > (10 + this->coilDiameter)) {
                            gFuncin = 0;

                            // else the ring(n1, m1) is in the middle-field of the ring(n,m)
                        } else {
                            // if no calculated value have been stored
                            if (valStored(mm1, nn1) < 0.0) {
                                midFieldVal = midFieldResponseFunction(m, n, m1, n1, t);
                                valStored(mm1, nn1) = midFieldVal;
                                // if a stored value is found for the combination of (m, n, m1, n1), then
                            } else {
                                midFieldVal = valStored(mm1, nn1);
                            }

                            // due to symmetry, the temperature response of ring(n1, m1) should be 0.25, 0.5, or 1 times its calculated value
                            if (!isEven(this->numTrenches) && !isEven(this->numCoils) && m1 == numRC && n1 == numLC && this->numTrenches > 1.5) {
                                gFuncin = 0.25 * midFieldVal;
                            } else if (!isEven(this->numTrenches) && m1 == numRC && this->numTrenches > 1.5) {
                                gFuncin = 0.5 * midFieldVal;
                            } else if (!isEven(this->numCoils) && n1 == numLC) {
                                gFuncin = 0.5 * midFieldVal;
                            } else {
                                gFuncin = midFieldVal;
                            }
                        }

                        gFunc += gFuncin;

                    } // n
                } // m
            } // n1
        } // m1

        this->myRespFactors->GFNC[NT - 1] = (gFunc * (this->coilDiameter / 2.0)) / (4 * Constant::Pi * fraction * this->numTrenches * this->numCoils);
        this->myRespFactors->LNTTS[NT - 1] = tLg;

    } // NT time
}

Real64
GLHESlinky::nearFieldResponseFunction(int const m, int const n, int const m1, int const n1, Real64 const eta, Real64 const theta, Real64 const t)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the temperature response of from one near-field point to another

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 distance1 = distance(m, n, m1, n1, eta, theta);
    Real64 sqrtAlphaT = std::sqrt(this->soil.diffusivity * t);

    if (!verticalConfig) {

        Real64 sqrtDistDepth = std::sqrt(pow_2(distance1) + 4 * pow_2(this->coilDepth));
        Real64 errFunc1 = std::erfc(0.5 * distance1 / sqrtAlphaT);
        Real64 errFunc2 = std::erfc(0.5 * sqrtDistDepth / sqrtAlphaT);

        return errFunc1 / distance1 - errFunc2 / sqrtDistDepth;

    } else {

        Real64 distance2 = distanceToFictRing(m, n, m1, n1, eta, theta);
        Real64 errFunc1 = std::erfc(0.5 * distance1 / sqrtAlphaT);
        Real64 errFunc2 = std::erfc(0.5 * distance2 / sqrtAlphaT);

        return errFunc1 / distance1 - errFunc2 / distance2;
    }
}

//******************************************************************************

Real64 GLHESlinky::midFieldResponseFunction(int const m, int const n, int const m1, int const n1, Real64 const t)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the temperature response of from one mid-field point to another

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 sqrtAlphaT = std::sqrt(this->soil.diffusivity * t);

    Real64 distance = distToCenter(m, n, m1, n1);
    Real64 sqrtDistDepth = std::sqrt(pow_2(distance) + 4 * pow_2(this->coilDepth));

    Real64 errFunc1 = std::erfc(0.5 * distance / sqrtAlphaT);
    Real64 errFunc2 = std::erfc(0.5 * sqrtDistDepth / sqrtAlphaT);

    return 4 * pow_2(Constant::Pi) * (errFunc1 / distance - errFunc2 / sqrtDistDepth);
}

//******************************************************************************

Real64 GLHESlinky::distance(int const m, int const n, int const m1, int const n1, Real64 const eta, Real64 const theta)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the distance between any two points on any two loops

    Real64 const cos_theta = std::cos(theta);
    Real64 const sin_theta = std::sin(theta);
    Real64 const cos_eta = std::cos(eta);
    Real64 const sin_eta = std::sin(eta);

    Real64 x = this->X0(n) + cos_theta * (this->coilDiameter / 2.0);
    Real64 y = this->Y0(m) + sin_theta * (this->coilDiameter / 2.0);

    Real64 xIn = this->X0(n1) + cos_eta * (this->coilDiameter / 2.0 - this->pipe.outRadius);
    Real64 yIn = this->Y0(m1) + sin_eta * (this->coilDiameter / 2.0 - this->pipe.outRadius);

    Real64 xOut = this->X0(n1) + cos_eta * (this->coilDiameter / 2.0 + this->pipe.outRadius);
    Real64 yOut = this->Y0(m1) + sin_eta * (this->coilDiameter / 2.0 + this->pipe.outRadius);

    if (!verticalConfig) {

        return 0.5 * std::sqrt(pow_2(x - xIn) + pow_2(y - yIn)) + 0.5 * std::sqrt(pow_2(x - xOut) + pow_2(y - yOut));

    } else {

        Real64 z = this->Z0 + sin_theta * (this->coilDiameter / 2.0);

        Real64 zIn = this->Z0 + sin_eta * (this->coilDiameter / 2.0 - this->pipe.outRadius);
        Real64 zOut = this->Z0 + sin_eta * (this->coilDiameter / 2.0 + this->pipe.outRadius);

        return 0.5 * std::sqrt(pow_2(x - xIn) + pow_2(this->Y0(m1) - this->Y0(m)) + pow_2(z - zIn)) +
               0.5 * std::sqrt(pow_2(x - xOut) + pow_2(this->Y0(m1) - this->Y0(m)) + pow_2(z - zOut));
    }
}

//******************************************************************************

Real64 GLHESlinky::distanceToFictRing(int const m, int const n, int const m1, int const n1, Real64 const eta, Real64 const theta)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the distance between any two points between real and fictitious rings

    Real64 const sin_theta = std::sin(theta);
    Real64 const cos_theta = std::cos(theta);
    Real64 const sin_eta = std::sin(eta);
    Real64 const cos_eta = std::cos(eta);

    Real64 x = this->X0(n) + cos_theta * (this->coilDiameter / 2.0);
    // Real64 y = Y0( m ) + sin_theta * (coilDiameter / 2.0);
    Real64 z = this->Z0 + sin_theta * (this->coilDiameter / 2.0) + 2 * this->coilDepth;

    Real64 xIn = this->X0(n1) + cos_eta * (this->coilDiameter / 2.0 - this->pipe.outRadius);
    // Real64 yIn = Y0( m1 ) + sin_eta * (coilDiameter / 2.0 - pipe.outRadius);
    Real64 zIn = this->Z0 + sin_eta * (this->coilDiameter / 2.0 - this->pipe.outRadius);

    Real64 xOut = this->X0(n1) + cos_eta * (this->coilDiameter / 2.0 + this->pipe.outRadius);
    // Real64 yOut = Y0( m1 ) + sin_eta * (coilDiameter / 2.0 + outRadius);
    Real64 zOut = this->Z0 + sin_eta * (this->coilDiameter / 2.0 + this->pipe.outRadius);

    return 0.5 * std::sqrt(pow_2(x - xIn) + pow_2(this->Y0(m1) - this->Y0(m)) + pow_2(z - zIn)) +
           0.5 * std::sqrt(pow_2(x - xOut) + pow_2(this->Y0(m1) - this->Y0(m)) + pow_2(z - zOut));
}

//******************************************************************************

Real64 GLHESlinky::distToCenter(int const m, int const n, int const m1, int const n1)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the center-to-center distance between rings

    return std::sqrt(pow_2(this->X0(n) - this->X0(n1)) + pow_2(this->Y0(m) - this->Y0(m1)));
}

//******************************************************************************

Real64 GLHESlinky::integral(int const m, int const n, int const m1, int const n1, Real64 const t, Real64 const eta, int const J0)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Integrates the temperature response at one point based on
    // input from other points

    // METHODOLOGY EMPLOYED:
    // Simpson's 1/3 rule of integration

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 theta = 0.0;
    constexpr Real64 theta1 = 0.0;
    constexpr Real64 theta2 = 2 * Constant::Pi;
    std::vector<Real64> f;

    Real64 h = (theta2 - theta1) / (J0 - 1);

    // Calculate the function at various equally spaced x values
    for (int j = 0; j < J0; ++j) {
        theta = theta1 + j * h;
        f.push_back(nearFieldResponseFunction(m, n, m1, n1, eta, theta, t));
    }

    for (int j = 1; j < J0 - 1; ++j) {
        if (!isEven(j)) {
            f[j] = 4 * f[j];
        } else {
            f[j] = 2 * f[j];
        }
    }

    return (h / 3) * std::reduce(f.begin(), f.end());
}

Real64 GLHESlinky::calcHXResistance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Matt Mitchell
    //       DATE WRITTEN   February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    //    Calculates the resistance of the slinky HX from the fluid to the outer tube wall.

    // SUBROUTINE PARAMETER DEFINITIONS:
    std::string_view const RoutineName = "CalcSlinkyGroundHeatExchanger";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 nusseltNum;
    Real64 Rconv;
    constexpr Real64 A = 3150;
    constexpr Real64 B = 350;
    constexpr Real64 laminarNusseltNo = 4.364;

    Real64 cpFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->inletTemp, RoutineName);
    Real64 kFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getConductivity(state, this->inletTemp, RoutineName);
    Real64 fluidDensity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, this->inletTemp, RoutineName);
    Real64 fluidViscosity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getViscosity(state, this->inletTemp, RoutineName);

    // calculate mass flow rate
    Real64 singleSlinkyMassFlowRate = this->massFlowRate / this->numTrenches;

    Real64 pipeInnerRad = this->pipe.outRadius - this->pipe.thickness;
    Real64 pipeInnerDia = 2.0 * pipeInnerRad;

    if (singleSlinkyMassFlowRate == 0.0) {
        Rconv = 0.0;
    } else {
        // Re=Rho*V*D/Mu
        Real64 reynoldsNum =
            fluidDensity * pipeInnerDia * (singleSlinkyMassFlowRate / fluidDensity / (Constant::Pi * pow_2(pipeInnerRad))) / fluidViscosity;
        Real64 prandtlNum = (cpFluid * fluidViscosity) / (kFluid);
        //   Convection Resistance
        if (reynoldsNum <= 2300) {
            nusseltNum = laminarNusseltNo;
        } else if (reynoldsNum > 2300 && reynoldsNum <= 4000) {
            Real64 sf = 0.5 + 0.5 * std::tanh((reynoldsNum - A) / B);
            Real64 turbulentNusseltNo = 0.023 * std::pow(reynoldsNum, 0.8) * std::pow(prandtlNum, 0.35);
            nusseltNum = laminarNusseltNo * (1 - sf) + turbulentNusseltNo * sf;
        } else {
            nusseltNum = 0.023 * std::pow(reynoldsNum, 0.8) * std::pow(prandtlNum, 0.35);
        }
        Real64 hci = nusseltNum * kFluid / pipeInnerDia;
        Rconv = 1.0 / (2.0 * Constant::Pi * pipeInnerDia * hci);
    }

    //   Conduction Resistance
    Real64 Rcond = std::log(this->pipe.outRadius / pipeInnerRad) / (2.0 * Constant::Pi * this->pipe.k) / 2.0; // pipe in parallel so /2

    return Rcond + Rconv;
}

Real64 GLHESlinky::getGFunc(Real64 const time)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February, 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Gets the g-function for slinky GHXs Note: Base 10 here.

    Real64 LNTTS = std::log10(time);
    return interpGFunc(LNTTS);
}

void GLHESlinky::initGLHESimVars(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    August, 2000
    //       MODIFIED         Arun Murugappan

    Real64 CurTime = ((state.dataGlobal->DayOfSim - 1) * Constant::rHoursInDay + (state.dataGlobal->HourOfDay - 1) +
                      (state.dataGlobal->TimeStep - 1) * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed) *
                     Constant::rSecsInHour;

    // Init more variables
    if (this->myEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
        this->initEnvironment(state, CurTime);
    }

    this->tempGround = this->groundTempModel->getGroundTempAtTimeInSeconds(state, this->coilDepth, CurTime);

    this->massFlowRate = PlantUtilities::RegulateCondenserCompFlowReqOp(state, this->plantLoc, this->designMassFlow);

    PlantUtilities::SetComponentFlowRate(state, this->massFlowRate, this->inletNodeNum, this->outletNodeNum, this->plantLoc);

    // Reset local environment init flag
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->myEnvrnFlag = true;
    }
}

//******************************************************************************

void GLHESlinky::initEnvironment(EnergyPlusData &state, Real64 const CurTime)
{

    std::string_view const RoutineName = "initEnvironment";
    this->myEnvrnFlag = false;

    Real64 fluidDensity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, 20.0, RoutineName);
    this->designMassFlow = this->designFlow * fluidDensity;
    PlantUtilities::InitComponentNodes(state, 0.0, this->designMassFlow, this->inletNodeNum, this->outletNodeNum);

    this->lastQnSubHr = 0.0;
    state.dataLoopNodes->Node(this->inletNodeNum).Temp = this->groundTempModel->getGroundTempAtTimeInSeconds(state, this->coilDepth, CurTime);
    state.dataLoopNodes->Node(this->outletNodeNum).Temp = this->groundTempModel->getGroundTempAtTimeInSeconds(state, this->coilDepth, CurTime);

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

//******************************************************************************

void GLHESlinky::oneTimeInit_new(EnergyPlusData &state)
{
    // Locate the hx on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(
        state, this->name, DataPlant::PlantEquipmentType::GrndHtExchgSlinky, this->plantLoc, errFlag, _, _, _, _, _);
    if (errFlag) {
        ShowFatalError(state, "initGLHESimVars: Program terminated due to previous condition(s).");
    }
}
void GLHESlinky::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}

} // namespace EnergyPlus::GroundHeatExchangers
