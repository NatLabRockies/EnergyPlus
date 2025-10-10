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

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/SurfaceGroundHeatExchanger.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace SurfaceGroundHeatExchanger {

    // Module containing the routines dealing with surface/panel ground heat exchangers

    // MODULE INFORMATION:
    //       AUTHOR         Simon Rees
    //       DATE WRITTEN   August 2002
    //       MODIFIED       Brent Griffith, Sept 2010, plant upgrades
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // The purpose of this module is to simulate hydronic Surface Ground Heat
    // Exchangers. This includes pavement surfaces with embedded pipes for snow-
    // melting or heat rejection from hybrid ground source heat pump systems.
    // The heat exchanger may be gound coupled or not. In the latter case the
    // bottom surface is exposed to the wind but not solar gains.

    // METHODOLOGY EMPLOYED:
    // This model is based on the QTF formulation of heat transfer through
    // building elements with embedded heat sources/sinks. The model uses
    // a heat exchanger analogy to relate the inlet fluid temperature to the
    // net heat transfer rate and consequently outlet temperature. The model
    // is entirely passive i.e. it does not set any flow rates or incorporate
    // any controls. In order to deal with the non-linear boundary conditions
    // at the top surface due to the presence of ice/snow fluxes have to be
    // calculated by the QTF model and temperature calculated from the surface
    // heat balance. This requires some iteration.
    // Note: top surface variables correspond to 'outside' variables in standard
    // CTF/QTF definition. Bottom surface variables correspond to 'inside' variables.

    // REFERENCES:
    // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
    //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
    //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
    //   Engineering.
    // Seem, J.E. 1986. "Heat Transfer in Buildings", Ph.D. dissertation, University
    //   of Wisconsin-Madison.

    // OTHER NOTES: none

    // USE STATEMENTS:
    // Use statements for data only modules
    // Using/Aliasing
    using namespace DataLoopNode;

    // Use statements for access to subroutines in other modules

    // Data
    // MODULE PARAMETER DEFINITIONS
    Real64 constexpr SmallNum(1.0e-30);         // Very small number to avoid div0 errors
    Real64 constexpr StefBoltzmann(5.6697e-08); // Stefan-Boltzmann constant
    Real64 constexpr SurfaceHXHeight(0.0);      // Surface Height above ground -- used in height dependent calcs.

    int constexpr SurfCond_Ground(1);
    int constexpr SurfCond_Exposed(2);

    PlantComponent *SurfaceGroundHeatExchangerData::factory(EnergyPlusData &state,
                                                            [[maybe_unused]] DataPlant::PlantEquipmentType objectType,
                                                            std::string const objectName)
    {
        if (state.dataSurfaceGroundHeatExchangers->GetInputFlag) {
            GetSurfaceGroundHeatExchanger(state);
            state.dataSurfaceGroundHeatExchangers->GetInputFlag = false;
        }
        // Now look for this particular pipe in the list
        for (auto &ghx : state.dataSurfaceGroundHeatExchangers->SurfaceGHE) {
            if (ghx.Name == objectName) {
                return &ghx;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError(state, format("Surface Ground Heat Exchanger: Error getting inputs for pipe named: {}", objectName));
        // Shut up the compiler
        return nullptr;
    }

    void SurfaceGroundHeatExchangerData::simulate(EnergyPlusData &state,
                                                  [[maybe_unused]] const PlantLocation &calledFromLocation,
                                                  bool const FirstHVACIteration,
                                                  [[maybe_unused]] Real64 &CurLoad,
                                                  [[maybe_unused]] bool const RunFlag)
    {
        this->InitSurfaceGroundHeatExchanger(state);
        this->CalcSurfaceGroundHeatExchanger(state, FirstHVACIteration);
        this->UpdateSurfaceGroundHeatExchngr(state);
        this->ReportSurfaceGroundHeatExchngr(state);
    }

    void GetSurfaceGroundHeatExchanger(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads the input for hydronic Surface Ground Heat Exchangers
        // from the user input file.  This will contain all of the information
        // needed to define and simulate the surface.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        // Using/Aliasing
        using BranchNodeConnections::TestCompSet;
        using NodeInputManager::GetOnlySingleNode;
        using namespace DataLoopNode;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        bool ErrorsFound(false); // Set to true if errors in input,
        // fatal at end of routine
        int IOStatus;   // Used in GetObjectItem
        int Item;       // Item to be "gotten"
        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        // Initializations and allocations
        cCurrentModuleObject = "GroundHeatExchanger:Surface";
        int NumOfSurfaceGHEs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        // allocate data structures
        if (allocated(state.dataSurfaceGroundHeatExchangers->SurfaceGHE)) {
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE.deallocate();
        }

        state.dataSurfaceGroundHeatExchangers->SurfaceGHE.allocate(NumOfSurfaceGHEs);
        state.dataSurfaceGroundHeatExchangers->CheckEquipName.dimension(NumOfSurfaceGHEs, true);

        // initialize data structures
        // surface data
        // Obtain all of the user data related to the surfaces...
        for (Item = 1; Item <= NumOfSurfaceGHEs; ++Item) {

            // get the input data
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Item,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     _,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            // General user input data
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name = state.dataIPShortCut->cAlphaArgs(1);
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).ConstructionName = state.dataIPShortCut->cAlphaArgs(2);
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).ConstructionNum =
                Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataConstruction->Construct);

            if (state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).ConstructionNum == 0) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            // Error checking for surfaces, zones, and construction information
            if (!state.dataConstruction->Construct(state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).ConstructionNum).SourceSinkPresent) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(
                    state, "Construction must have internal source/sink and be referenced by a ConstructionProperty:InternalHeatSource object");
                ErrorsFound = true;
            }

            // get inlet node data
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).InletNode = state.dataIPShortCut->cAlphaArgs(3);
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).InletNodeNum =
                GetOnlySingleNode(state,
                                  state.dataIPShortCut->cAlphaArgs(3),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::GroundHeatExchangerSurface,
                                  state.dataIPShortCut->cAlphaArgs(1),
                                  DataLoopNode::NodeFluidType::Water,
                                  DataLoopNode::ConnectionType::Inlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  ObjectIsNotParent);
            if (state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).InletNodeNum == 0) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            // get outlet node data
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).OutletNode = state.dataIPShortCut->cAlphaArgs(4);
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).OutletNodeNum =
                GetOnlySingleNode(state,
                                  state.dataIPShortCut->cAlphaArgs(4),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::GroundHeatExchangerSurface,
                                  state.dataIPShortCut->cAlphaArgs(1),
                                  DataLoopNode::NodeFluidType::Water,
                                  DataLoopNode::ConnectionType::Outlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  ObjectIsNotParent);
            if (state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).OutletNodeNum == 0) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            TestCompSet(state,
                        cCurrentModuleObject,
                        state.dataIPShortCut->cAlphaArgs(1),
                        state.dataIPShortCut->cAlphaArgs(3),
                        state.dataIPShortCut->cAlphaArgs(4),
                        "Condenser Water Nodes");

            // tube data
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).TubeDiameter = state.dataIPShortCut->rNumericArgs(1);
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).TubeCircuits = state.dataIPShortCut->rNumericArgs(2);
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).TubeSpacing = state.dataIPShortCut->rNumericArgs(3);

            if (state.dataIPShortCut->rNumericArgs(2) == 0) {
                ShowSevereError(state,
                                format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(2), state.dataIPShortCut->rNumericArgs(2)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Value must be greater than 0.0");
                ErrorsFound = true;
            }
            if (state.dataIPShortCut->rNumericArgs(3) == 0.0) {
                ShowSevereError(state,
                                format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(3), state.dataIPShortCut->rNumericArgs(3)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Value must be greater than 0.0");
                ErrorsFound = true;
            }

            // surface geometry data
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).SurfaceLength = state.dataIPShortCut->rNumericArgs(4);
            state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).SurfaceWidth = state.dataIPShortCut->rNumericArgs(5);
            if (state.dataIPShortCut->rNumericArgs(4) <= 0.0) {
                ShowSevereError(state,
                                format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(4), state.dataIPShortCut->rNumericArgs(4)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Value must be greater than 0.0");
                ErrorsFound = true;
            }
            if (state.dataIPShortCut->rNumericArgs(5) <= 0.0) {
                ShowSevereError(state,
                                format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(5), state.dataIPShortCut->rNumericArgs(5)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Value must be greater than 0.0");
                ErrorsFound = true;
            }

            // get lower b.c. type
            if (Util::SameString(state.dataIPShortCut->cAlphaArgs(5), "GROUND")) {
                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).LowerSurfCond = SurfCond_Ground;
            } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(5), "EXPOSED")) {
                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).LowerSurfCond = SurfCond_Exposed;
            } else {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(5), state.dataIPShortCut->cAlphaArgs(5)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Only \"Ground\" or \"Exposed\" is allowed.");
                ErrorsFound = true;
            }

        } // end of input loop

        // final error check
        if (ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing input for {}", cCurrentModuleObject));
        }

        // Set up the output variables
        for (Item = 1; Item <= NumOfSurfaceGHEs; ++Item) {
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Heat Transfer Rate",
                                Constant::Units::W,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).HeatTransferRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Surface Heat Transfer Rate",
                                Constant::Units::W,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).SurfHeatTransferRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Heat Transfer Energy",
                                Constant::Units::J,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).MassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Inlet Temperature",
                                Constant::Units::C,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).InletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Outlet Temperature",
                                Constant::Units::C,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).OutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Top Surface Temperature",
                                Constant::Units::C,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).TopSurfaceTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Bottom Surface Temperature",
                                Constant::Units::C,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).BtmSurfaceTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Top Surface Heat Transfer Energy per Area",
                                Constant::Units::J_m2,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).TopSurfaceFlux,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Bottom Surface Heat Transfer Energy per Area",
                                Constant::Units::J_m2,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).BtmSurfaceFlux,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Surface Heat Transfer Energy",
                                Constant::Units::J,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).SurfEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
            SetupOutputVariable(state,
                                "Ground Heat Exchanger Source Temperature",
                                Constant::Units::C,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).SourceTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurfaceGroundHeatExchangers->SurfaceGHE(Item).Name);
        }

        if (state.dataSurfaceGroundHeatExchangers->NoSurfaceGroundTempObjWarning) {
            if (!state.dataEnvrn->GroundTempInputs[(int)DataEnvironment::GroundTempType::Shallow]) {
                ShowWarningError(state, "GetSurfaceGroundHeatExchanger: No \"Site:GroundTemperature:Shallow\" were input.");
                ShowContinueError(state,
                                  format("Defaults, constant throughout the year of ({:.1R}) will be used.",
                                         state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::Shallow]));
            }
            state.dataSurfaceGroundHeatExchangers->NoSurfaceGroundTempObjWarning = false;
        }
    }

    void SurfaceGroundHeatExchangerData::InitSurfaceGroundHeatExchanger(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine Resets the elements of the data structure as necessary
        // at the first HVAC iteration of each time step. The weather and QTF data
        // is initialized once only.

        // METHODOLOGY EMPLOYED:
        // Check flags and update data structure

        // Using/Aliasing
        using namespace DataEnvironment;
        using PlantUtilities::RegulateCondenserCompFlowReqOp;
        using PlantUtilities::SetComponentFlowRate;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Real64 DesignFlow; // Hypothetical design flow rate
        int Cons;          // construction counter
        int LayerNum;      // material layer number for bottom
        Real64 OutDryBulb; // Height Dependent dry bulb.

        auto &s_mat = state.dataMaterial;

        // get QTF data - only once
        if (this->InitQTF) {
            for (Cons = 1; Cons <= state.dataHeatBal->TotConstructs; ++Cons) {
                if (Util::SameString(state.dataConstruction->Construct(Cons).Name, this->ConstructionName)) {
                    // some error checking ??
                    // CTF stuff
                    LayerNum = state.dataConstruction->Construct(Cons).TotLayers;
                    this->NumCTFTerms = state.dataConstruction->Construct(Cons).NumCTFTerms;
                    this->CTFin = state.dataConstruction->Construct(Cons).CTFInside;   // Z coefficents
                    this->CTFout = state.dataConstruction->Construct(Cons).CTFOutside; // X coefficents
                    this->CTFcross = state.dataConstruction->Construct(Cons).CTFCross; // Y coefficents
                    for (size_t i = 1; i < state.dataConstruction->Construct(Cons).CTFFlux.size(); i++) {
                        this->CTFflux[i] = state.dataConstruction->Construct(Cons).CTFFlux[i]; // F & f coefficents
                    }
                    // QTF stuff
                    this->CTFSourceIn = state.dataConstruction->Construct(Cons).CTFSourceIn;     // Wi coefficents
                    this->CTFSourceOut = state.dataConstruction->Construct(Cons).CTFSourceOut;   // Wo coefficents
                    this->CTFTSourceOut = state.dataConstruction->Construct(Cons).CTFTSourceOut; // y coefficents
                    this->CTFTSourceIn = state.dataConstruction->Construct(Cons).CTFTSourceIn;   // x coefficents
                    this->CTFTSourceQ = state.dataConstruction->Construct(Cons).CTFTSourceQ;     // w coefficents
                    this->ConstructionNum = Cons;
                    // surface properties
                    auto const *thisMaterialLayer = s_mat->materials(state.dataConstruction->Construct(Cons).LayerPoint(LayerNum));
                    assert(thisMaterialLayer != nullptr);
                    this->BtmRoughness = thisMaterialLayer->Roughness;
                    this->TopThermAbs = thisMaterialLayer->AbsorpThermal;
                    auto const *thisMaterial1 = s_mat->materials(state.dataConstruction->Construct(Cons).LayerPoint(1));
                    assert(thisMaterial1 != nullptr);
                    this->TopRoughness = thisMaterial1->Roughness;
                    this->TopThermAbs = thisMaterial1->AbsorpThermal;
                    this->TopSolarAbs = thisMaterial1->AbsorpSolar;
                }
            }
            // set one-time flag
            this->InitQTF = false;
        }

        if (this->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
            OutDryBulb = OutDryBulbTempAt(state, SurfaceHXHeight);
            this->CTFflux[0] = 0.0;
            this->TsrcHistory.fill(OutDryBulb);
            this->TbtmHistory.fill(OutDryBulb);
            this->TtopHistory.fill(OutDryBulb);
            this->TsrcHistory.fill(OutDryBulb);
            this->QbtmHistory.fill(0.0);
            this->QtopHistory.fill(0.0);
            this->QsrcHistory.fill(0.0);
            this->TsrcConstCoef = 0.0;
            this->TsrcVarCoef = 0.0;
            this->QbtmConstCoef = 0.0;
            this->QbtmVarCoef = 0.0;
            this->QtopConstCoef = 0.0;
            this->QtopVarCoef = 0.0;
            this->QSrc = 0.0;
            this->QSrcAvg = 0.0;
            this->LastQSrc = 0.0;
            this->LastSysTimeElapsed = 0.0;
            this->LastTimeStepSys = 0.0;
            // initialize past weather variables
            state.dataSurfaceGroundHeatExchangers->PastBeamSolarRad = state.dataEnvrn->BeamSolarRad;
            state.dataSurfaceGroundHeatExchangers->PastSolarDirCosVert = state.dataEnvrn->SOLCOS(3);
            state.dataSurfaceGroundHeatExchangers->PastDifSolarRad = state.dataEnvrn->DifSolarRad;
            state.dataSurfaceGroundHeatExchangers->PastGroundTemp = state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::Shallow];
            state.dataSurfaceGroundHeatExchangers->PastIsRain = state.dataEnvrn->IsRain;
            state.dataSurfaceGroundHeatExchangers->PastIsSnow = state.dataEnvrn->IsSnow;
            state.dataSurfaceGroundHeatExchangers->PastOutDryBulbTemp = OutDryBulbTempAt(state, SurfaceHXHeight);
            state.dataSurfaceGroundHeatExchangers->PastOutWetBulbTemp = OutWetBulbTempAt(state, SurfaceHXHeight);
            state.dataSurfaceGroundHeatExchangers->PastSkyTemp = state.dataEnvrn->SkyTemp;
            state.dataSurfaceGroundHeatExchangers->PastWindSpeed = DataEnvironment::WindSpeedAt(state, SurfaceHXHeight);
            this->MyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        // always initialize - module variables
        this->SurfaceArea = this->SurfaceLength * this->SurfaceWidth;

        // If loop operation is controlled by an environmental variable (DBtemp, WBtemp, etc)
        // then shut branch down when equipment is not scheduled to run.
        DesignFlow = RegulateCondenserCompFlowReqOp(state, this->plantLoc, this->DesignMassFlowRate);

        SetComponentFlowRate(state, DesignFlow, this->InletNodeNum, this->OutletNodeNum, this->plantLoc);

        // get the current flow rate - module variable
        state.dataSurfaceGroundHeatExchangers->FlowRate = state.dataLoopNodes->Node(this->InletNodeNum).MassFlowRate;
    }

    void SurfaceGroundHeatExchangerData::CalcSurfaceGroundHeatExchanger(
        EnergyPlusData &state, bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    )
    {

        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does all of the stuff that is necessary to simulate
        // a surface ground heat exchanger.  Calls are made to appropriate subroutines
        // either in this module or outside of it.

        // METHODOLOGY EMPLOYED:
        // To update temperature and flux histories it is necessary to make a surface
        // flux/temperature calculation at the begining of each zone time step using the
        // weather data from the previous step, and using the average source flux.
        // Once this has been done a new source flux, and current surface temperatures,
        // are calculated using the current weather data. These surface temperatures and
        // fluxes are used for the rest of the system time steps. During subsequent system
        // time steps only the source flux is updated.

        // Surface fluxes are calculated from the QTF equations using assumed surface
        // temperatures. Surface fluxes are then dependant only on source flux. Constant
        // and terms and terms that multiply the source flux from the QTF equations, are
        // grouped together for convenience. These are calculated in "CalcBottomFluxCoefficents"
        // etc. It is necessary to iterate on these equations, updating the current surface
        // temperatures at each step.

        // REFERENCES:
        // See 'LowTempRadiantSystem' module
        // IBLAST-QTF research program, completed in January 1995 (unreleased)
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.
        // Seem, J.E. 1986. "Heat Transfer in Buildings", Ph.D. dissertation, University
        //   of Wisconsin-Madison.

        // Using/Aliasing
        using namespace DataEnvironment;

        Real64 constexpr SurfFluxTol(0.001); // tolerance on the surface fluxes
        Real64 constexpr SrcFluxTol(0.001);  // tolerance on the source flux
        Real64 constexpr RelaxT(0.1);        // temperature relaxation factor
        int constexpr Maxiter(100);
        int constexpr Maxiter1(100);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 PastFluxTop;    // top surface flux - past value
        Real64 PastFluxBtm;    // bottom surface flux - past value
        Real64 PastTempBtm;    // bottom surface temp - past value
        Real64 PastTempTop;    // top surface temp - past value
        Real64 OldPastFluxTop; // top surface flux - past value used during iteration
        Real64 OldPastFluxBtm; // bottom surface flux - past value used during iteration
        // variables used with current environmental conditions
        auto &FluxTop = state.dataSurfaceGroundHeatExchangers->FluxTop; // top surface flux
        auto &FluxBtm = state.dataSurfaceGroundHeatExchangers->FluxBtm; // bottom surface flux
        auto &TempBtm = state.dataSurfaceGroundHeatExchangers->TempBtm; // bottom surface temp
        auto &TempTop = state.dataSurfaceGroundHeatExchangers->TempTop; // top surface temp
        Real64 TempT;                                                   // top surface temp - used in underrelaxation
        Real64 TempB;                                                   // bottom surface temp - used in underrelaxation
        Real64 OldFluxTop;                                              // top surface flux - value used during iteration
        Real64 OldFluxBtm;                                              // bottom surface flux - value used during iteration
        Real64 OldSourceFlux;                                           // previous value of source flux - used during iteration
        int iter;
        int iter1;

        // check if we are in very first call for this zone time step
        if (FirstHVACIteration && !state.dataHVACGlobal->ShortenTimeStepSys && this->firstTimeThrough) {
            this->firstTimeThrough = false;
            // calc temps and fluxes with past env. conditions and average source flux
            state.dataSurfaceGroundHeatExchangers->SourceFlux = this->QSrcAvg;
            // starting values for the surface temps
            PastTempBtm = this->TbtmHistory[1];
            PastTempTop = this->TtopHistory[1];
            OldPastFluxTop = 1.0e+30;
            OldPastFluxBtm = 1.0e+30;
            TempB = 0.0;
            TempT = 0.0;
            iter = 0;
            while (true) { // iterate to find surface heat balances
                // update coefficients

                ++iter;
                CalcTopFluxCoefficents(PastTempBtm, PastTempTop);
                // calc top surface flux
                PastFluxTop = this->QtopConstCoef + this->QtopVarCoef * state.dataSurfaceGroundHeatExchangers->SourceFlux;

                // calc new top surface temp
                CalcTopSurfTemp(-PastFluxTop,
                                TempT,
                                state.dataSurfaceGroundHeatExchangers->PastOutDryBulbTemp,
                                state.dataSurfaceGroundHeatExchangers->PastOutWetBulbTemp,
                                state.dataSurfaceGroundHeatExchangers->PastSkyTemp,
                                state.dataSurfaceGroundHeatExchangers->PastBeamSolarRad,
                                state.dataSurfaceGroundHeatExchangers->PastDifSolarRad,
                                state.dataSurfaceGroundHeatExchangers->PastSolarDirCosVert,
                                state.dataSurfaceGroundHeatExchangers->PastWindSpeed,
                                state.dataSurfaceGroundHeatExchangers->PastIsRain,
                                state.dataSurfaceGroundHeatExchangers->PastIsSnow);
                // under relax
                PastTempTop = PastTempTop * (1.0 - RelaxT) + RelaxT * TempT;

                // update coefficients
                CalcBottomFluxCoefficents(PastTempBtm, PastTempTop);
                PastFluxBtm = this->QbtmConstCoef + this->QbtmVarCoef * state.dataSurfaceGroundHeatExchangers->SourceFlux;

                if (std::abs((OldPastFluxTop - PastFluxTop) / OldPastFluxTop) <= SurfFluxTol &&
                    std::abs((OldPastFluxBtm - PastFluxBtm) / OldPastFluxBtm) <= SurfFluxTol) {
                    break;
                }

                // calc new surface temps
                CalcBottomSurfTemp(PastFluxBtm,
                                   TempB,
                                   state.dataSurfaceGroundHeatExchangers->PastOutDryBulbTemp,
                                   state.dataSurfaceGroundHeatExchangers->PastWindSpeed,
                                   state.dataSurfaceGroundHeatExchangers->PastGroundTemp);
                // underrelax
                PastTempBtm = PastTempBtm * (1.0 - RelaxT) + RelaxT * TempB;
                // update flux record
                OldPastFluxTop = PastFluxTop;
                OldPastFluxBtm = PastFluxBtm;

                // Check for non-convergence
                if (iter > Maxiter) {
                    if (this->ConvErrIndex1 == 0) {
                        ShowWarningMessage(
                            state, format("CalcSurfaceGroundHeatExchanger=\"{}\", Did not converge (part 1), Iterations={}", this->Name, Maxiter));
                        ShowContinueErrorTimeStamp(state, "");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state, "CalcSurfaceGroundHeatExchanger=\"" + this->Name + "\", Did not converge (part 1)", this->ConvErrIndex1);
                    break;
                }
            }

            if (!state.dataSurfaceGroundHeatExchangers->InitializeTempTop) {
                TempTop = TempT;
                TempBtm = TempB;
                FluxTop = PastFluxTop;
                FluxBtm = PastFluxBtm;
                state.dataSurfaceGroundHeatExchangers->InitializeTempTop = true;
            }

            // update module variables
            state.dataSurfaceGroundHeatExchangers->TopSurfTemp = TempTop;
            state.dataSurfaceGroundHeatExchangers->BtmSurfTemp = TempBtm;
            state.dataSurfaceGroundHeatExchangers->TopSurfFlux = -FluxTop;
            state.dataSurfaceGroundHeatExchangers->BtmSurfFlux = FluxBtm;

            // get source temp for output
            CalcSourceTempCoefficents(PastTempBtm, PastTempTop);
            this->SourceTemp = this->TsrcConstCoef + this->TsrcVarCoef * state.dataSurfaceGroundHeatExchangers->SourceFlux;
            // update histories
            UpdateHistories(PastFluxTop, PastFluxBtm, state.dataSurfaceGroundHeatExchangers->SourceFlux, this->SourceTemp);

            // At the beginning of a time step, reset to zero so average calculation can start again
            this->QSrcAvg = 0.0;
            this->LastSysTimeElapsed = 0.0;
            this->LastTimeStepSys = 0.0;

            // get current env. conditions
            state.dataSurfaceGroundHeatExchangers->PastBeamSolarRad = state.dataEnvrn->BeamSolarRad;
            state.dataSurfaceGroundHeatExchangers->PastSolarDirCosVert = state.dataEnvrn->SOLCOS(3);
            state.dataSurfaceGroundHeatExchangers->PastDifSolarRad = state.dataEnvrn->DifSolarRad;
            state.dataSurfaceGroundHeatExchangers->PastGroundTemp = state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::Shallow];
            state.dataSurfaceGroundHeatExchangers->PastIsRain = state.dataEnvrn->IsRain;
            state.dataSurfaceGroundHeatExchangers->PastIsSnow = state.dataEnvrn->IsSnow;
            state.dataSurfaceGroundHeatExchangers->PastOutDryBulbTemp = OutDryBulbTempAt(state, SurfaceHXHeight);
            state.dataSurfaceGroundHeatExchangers->PastOutWetBulbTemp = OutWetBulbTempAt(state, SurfaceHXHeight);
            state.dataSurfaceGroundHeatExchangers->PastSkyTemp = state.dataEnvrn->SkyTemp;
            state.dataSurfaceGroundHeatExchangers->PastWindSpeed = DataEnvironment::WindSpeedAt(state, SurfaceHXHeight);

            TempBtm = this->TbtmHistory[1];
            TempTop = this->TtopHistory[1];
            OldFluxTop = 1.0e+30;
            OldFluxBtm = 1.0e+30;
            OldSourceFlux = 1.0e+30;
            state.dataSurfaceGroundHeatExchangers->SourceFlux = CalcSourceFlux(state);
            iter = 0;
            while (true) { // iterate to find source flux
                ++iter;
                iter1 = 0;
                while (true) { // iterate to find surface heat balances
                    ++iter1;
                    // update top coefficients
                    CalcTopFluxCoefficents(TempBtm, TempTop);
                    // calc top surface flux
                    FluxTop = this->QtopConstCoef + this->QtopVarCoef * state.dataSurfaceGroundHeatExchangers->SourceFlux;
                    // calc new surface temps
                    CalcTopSurfTemp(-FluxTop,
                                    TempT,
                                    state.dataSurfaceGroundHeatExchangers->PastOutDryBulbTemp,
                                    state.dataSurfaceGroundHeatExchangers->PastOutWetBulbTemp,
                                    state.dataSurfaceGroundHeatExchangers->PastSkyTemp,
                                    state.dataSurfaceGroundHeatExchangers->PastBeamSolarRad,
                                    state.dataSurfaceGroundHeatExchangers->PastDifSolarRad,
                                    state.dataSurfaceGroundHeatExchangers->PastSolarDirCosVert,
                                    state.dataSurfaceGroundHeatExchangers->PastWindSpeed,
                                    state.dataSurfaceGroundHeatExchangers->PastIsRain,
                                    state.dataSurfaceGroundHeatExchangers->PastIsSnow);
                    // under-relax
                    TempTop = TempTop * (1.0 - RelaxT) + RelaxT * TempT;
                    // update bottom coefficients
                    CalcBottomFluxCoefficents(TempBtm, TempTop);
                    FluxBtm = this->QbtmConstCoef + this->QbtmVarCoef * state.dataSurfaceGroundHeatExchangers->SourceFlux;
                    // convergence test on surface fluxes
                    if (std::abs((OldFluxTop - FluxTop) / OldFluxTop) <= SurfFluxTol &&
                        std::abs((OldFluxBtm - FluxBtm) / OldFluxBtm) <= SurfFluxTol) {
                        break;
                    }

                    // calc new surface temps
                    CalcBottomSurfTemp(FluxBtm,
                                       TempB,
                                       state.dataSurfaceGroundHeatExchangers->PastOutDryBulbTemp,
                                       state.dataSurfaceGroundHeatExchangers->PastOutDryBulbTemp,
                                       state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::Shallow]);
                    // under-relax
                    TempBtm = TempBtm * (1.0 - RelaxT) + RelaxT * TempB;
                    // update flux record
                    OldFluxBtm = FluxBtm;
                    OldFluxTop = FluxTop;

                    // Check for non-convergence
                    if (iter1 > Maxiter1) {
                        if (this->ConvErrIndex2 == 0) {
                            ShowWarningMessage(
                                state,
                                format("CalcSurfaceGroundHeatExchanger=\"{}\", Did not converge (part 2), Iterations={}", this->Name, Maxiter));
                            ShowContinueErrorTimeStamp(state, "");
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state, "CalcSurfaceGroundHeatExchanger=\"" + this->Name + "\", Did not converge (part 2)", this->ConvErrIndex2);
                        break;
                    }
                }
                // update the source temp coefficients and update the source flux
                CalcSourceTempCoefficents(TempBtm, TempTop);
                state.dataSurfaceGroundHeatExchangers->SourceFlux = CalcSourceFlux(state);
                // check source flux convergence
                if (std::abs((OldSourceFlux - state.dataSurfaceGroundHeatExchangers->SourceFlux) / (1.0e-20 + OldSourceFlux)) <= SrcFluxTol) {
                    break;
                }
                OldSourceFlux = state.dataSurfaceGroundHeatExchangers->SourceFlux;

                // Check for non-convergence
                if (iter > Maxiter) {
                    if (this->ConvErrIndex3 == 0) {
                        ShowWarningMessage(
                            state, format("CalcSurfaceGroundHeatExchanger=\"{}\", Did not converge (part 3), Iterations={}", this->Name, Maxiter));
                        ShowContinueErrorTimeStamp(state, "");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state, "CalcSurfaceGroundHeatExchanger=\"" + this->Name + "\", Did not converge (part 3)", this->ConvErrIndex3);
                    break;
                }
            } // end surface heat balance iteration

        } else if (!FirstHVACIteration) { // end source flux iteration
            // For the rest of the system time steps ...
            // update source flux from Twi
            this->firstTimeThrough = true;
            state.dataSurfaceGroundHeatExchangers->SourceFlux = this->CalcSourceFlux(state);
        }
    }

    void SurfaceGroundHeatExchangerData::CalcBottomFluxCoefficents(Real64 const Tbottom, // current bottom (lower) surface temperature
                                                                   Real64 const Ttop     // current top (upper) surface temperature
    )
    {

        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates current version of constant variable parts of QTF equations.

        // METHODOLOGY EMPLOYED:
        // For given current surface temperatures the terms of the QTF equations can be
        // grouped into constant terms, and those depending on the current source flux.
        // This routine calculates the current coefficient values for the bottom flux
        // equation.

        // REFERENCES:
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Term;

        // add current surface temperatures to history data
        this->TbtmHistory[0] = Tbottom;
        this->TtopHistory[0] = Ttop;

        // Bottom Surface Coefficients
        this->QbtmConstCoef = 0.0;
        for (Term = 0; Term <= this->NumCTFTerms - 1; ++Term) {

            this->QbtmConstCoef += (-this->CTFin[Term] * this->TbtmHistory[Term]) + (this->CTFcross[Term] * this->TtopHistory[Term]) +
                                   (this->CTFflux[Term] * this->QbtmHistory[Term]) + (this->CTFSourceIn[Term] * this->QsrcHistory[Term]);
        }

        // correct for extra bottom surface flux term
        this->QbtmConstCoef -= this->CTFSourceIn[0] * this->QsrcHistory[0];
        // source flux current coefficient
        this->QbtmVarCoef = this->CTFSourceIn[0];
    }

    void SurfaceGroundHeatExchangerData::CalcTopFluxCoefficents(Real64 const Tbottom, // current bottom (lower) surface temperature
                                                                Real64 const Ttop     // current top (upper) surface temperature
    )
    {

        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates current version of constant variable parts of QTF equations.

        // METHODOLOGY EMPLOYED:
        // For given current surface temperatures the terms of the QTF equations can be
        // grouped into constant terms, and those depending on the current source flux.
        // This routine calculates the current coefficient values for the top flux
        // equation.

        // REFERENCES:
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.

        // add current surface temperatures to history data
        this->TbtmHistory[0] = Tbottom;
        this->TtopHistory[0] = Ttop;

        // Top Surface Coefficients
        this->QtopConstCoef = 0.0;
        for (int Term = 0; Term <= this->NumCTFTerms - 1; ++Term) {

            this->QtopConstCoef += (this->CTFout[Term] * this->TtopHistory[Term]) - (this->CTFcross[Term] * this->TbtmHistory[Term]) +
                                   (this->CTFflux[Term] * this->QtopHistory[Term]) + (this->CTFSourceOut[Term] * this->QsrcHistory[Term]);
        }

        // correct for extra top surface flux term
        this->QtopConstCoef -= (this->CTFSourceOut[0] * this->QsrcHistory[0]);
        // surface flux current coefficient
        this->QtopVarCoef = this->CTFSourceOut[0];
    }

    void SurfaceGroundHeatExchangerData::CalcSourceTempCoefficents(Real64 const Tbottom, // current bottom (lower) surface temperature
                                                                   Real64 const Ttop     // current top (upper) surface temperature
    )
    {

        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates current version of constant variable parts of QTF equations.

        // METHODOLOGY EMPLOYED:
        // For given current surface temperatures the terms of the QTF equations can be
        // grouped into constant terms, and those depending on the current source flux.
        // This routine calculates the current coefficient values for the source temperature
        // equation.

        // REFERENCES:
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Term;

        // add current surface temperatures to history data
        this->TbtmHistory[0] = Tbottom;
        this->TtopHistory[0] = Ttop;

        this->TsrcConstCoef = 0.0;
        for (Term = 0; Term <= this->NumCTFTerms - 1; ++Term) {

            this->TsrcConstCoef += (this->CTFTSourceIn[Term] * this->TbtmHistory[Term]) + (this->CTFTSourceOut[Term] * this->TtopHistory[Term]) +
                                   (this->CTFflux[Term] * this->TsrcHistory[Term]) + (this->CTFTSourceQ[Term] * this->QsrcHistory[Term]);
        }

        // correct for extra source flux term
        this->TsrcConstCoef -= this->CTFTSourceQ[0] * this->QsrcHistory[0];
        // source flux current coefficient
        this->TsrcVarCoef = this->CTFTSourceQ[0];
    }

    Real64 SurfaceGroundHeatExchangerData::CalcSourceFlux(EnergyPlusData &state) // component number
    {

        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This calculates the source flux given the inlet fluid temperature. A
        // heat exchanger analogy is used, with the surface as a 'Fixed' fluid.

        // METHODOLOGY EMPLOYED:

        // REFERENCES:
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.

        // Return value
        Real64 CalcSourceFlux;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 EpsMdotCp; // Epsilon (heat exchanger terminology) times water mass flow rate times water specific heat

        // Effectiveness * Modot * specific heat
        if (state.dataSurfaceGroundHeatExchangers->FlowRate > 0.0) {
            EpsMdotCp = CalcHXEffectTerm(state, this->InletTemp, state.dataSurfaceGroundHeatExchangers->FlowRate);
            // calc flux
            CalcSourceFlux = (this->InletTemp - this->TsrcConstCoef) / (this->SurfaceArea / EpsMdotCp + this->TsrcVarCoef);
        } else {
            CalcSourceFlux = 0.0;
        }

        return CalcSourceFlux;
    }

    void SurfaceGroundHeatExchangerData::UpdateHistories(Real64 const TopFlux,    // current top (top) surface flux
                                                         Real64 const BottomFlux, // current bottom (bottom) surface flux
                                                         Real64 const sourceFlux, // current source surface flux
                                                         Real64 const sourceTemp  // current source temperature
    )
    {

        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This is used to update the temperature and flux records for the QTF
        // calculations. This is called at the start of each zone timestep.

        // METHODOLOGY EMPLOYED:
        // Just shift along and replace zero index element with current value.

        // update top surface temps
        this->TtopHistory = eoshiftArray(this->TtopHistory, -1, 0.0);

        // update bottom surface temps
        this->TbtmHistory = eoshiftArray(this->TbtmHistory, -1, 0.0);

        // update bottom surface temps
        this->TsrcHistory = eoshiftArray(this->TsrcHistory, -1, 0.0);
        this->TsrcHistory[1] = sourceTemp;

        // update bottom surface fluxes
        this->QbtmHistory = eoshiftArray(this->QbtmHistory, -1, 0.0);
        this->QbtmHistory[1] = BottomFlux;

        // update bottom surface fluxes
        this->QtopHistory = eoshiftArray(this->QtopHistory, -1, 0.0);
        this->QtopHistory[1] = TopFlux;

        // update bottom surface fluxes
        this->QsrcHistory = eoshiftArray(this->QsrcHistory, -1, 0.0);
        this->QsrcHistory[1] = sourceFlux;
    }

    Real64 SurfaceGroundHeatExchangerData::CalcHXEffectTerm(EnergyPlusData &state,
                                                            Real64 const Temperature,  // Temperature of water entering the surface, in C
                                                            Real64 const WaterMassFlow // Mass flow rate, in kg/s
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   December 2000
        //       MODIFIED       Simon Rees, August 2002
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the "heat exchanger"
        // effectiveness term.  This is equal to the mass flow rate of water
        // times the specific heat of water times the effectiveness of
        // the surface heat exchanger. This routine is adapted from that in
        // the low temp radiant surface model.

        // METHODOLOGY EMPLOYED:
        // Assumes that the only REAL(r64) heat transfer term that we have to
        // deal with is the convection from the water to the tube.  The
        // other assumptions are that the tube bottom surface temperature
        // is equal to the "source location temperature" and that it is
        // a CONSTANT throughout the surface.

        // REFERENCES:
        // See RadiantSystemLowTemp module.
        // Property data for water shown below as parameters taken from
        //   Incropera and DeWitt, Introduction to Heat Transfer, Table A.6.
        // Heat exchanger information also from Incropera and DeWitt.
        // Code based loosely on code from IBLAST program (research version)

        // Return value
        Real64 CalcHXEffectTerm;

        Real64 constexpr MaxLaminarRe(2300.0); // Maximum Reynolds number for laminar flow
        int constexpr NumOfPropDivisions(13);  // intervals in property correlation
        static constexpr std::array<Real64, NumOfPropDivisions> Temps = {
            1.85, 6.85, 11.85, 16.85, 21.85, 26.85, 31.85, 36.85, 41.85, 46.85, 51.85, 56.85, 61.85}; // Temperature, in C
        static constexpr std::array<Real64, NumOfPropDivisions> Mu = {0.001652,
                                                                      0.001422,
                                                                      0.001225,
                                                                      0.00108,
                                                                      0.000959,
                                                                      0.000855,
                                                                      0.000769,
                                                                      0.000695,
                                                                      0.000631,
                                                                      0.000577,
                                                                      0.000528,
                                                                      0.000489,
                                                                      0.000453}; // Viscosity, in Ns/m2
        static constexpr std::array<Real64, NumOfPropDivisions> Conductivity = {
            0.574, 0.582, 0.590, 0.598, 0.606, 0.613, 0.620, 0.628, 0.634, 0.640, 0.645, 0.650, 0.656}; // Conductivity, in W/mK
        static constexpr std::array<Real64, NumOfPropDivisions> Pr = {
            12.22, 10.26, 8.81, 7.56, 6.62, 5.83, 5.20, 4.62, 4.16, 3.77, 3.42, 3.15, 2.88}; // Prandtl number (dimensionless)
        int constexpr WaterIndex(1);
        static constexpr std::string_view RoutineName("SurfaceGroundHeatExchanger:CalcHXEffectTerm");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Index;
        Real64 InterpFrac;
        Real64 NuD;
        Real64 ReD;
        Real64 NTU;
        Real64 CpWater;
        Real64 Kactual;
        Real64 MUactual;
        Real64 PRactual;
        Real64 PipeLength;

        // First find out where we are in the range of temperatures
        Index = 0;
        while (Index < NumOfPropDivisions) {
            if (Temperature < Temps[Index]) {
                break; // DO loop
            }
            ++Index;
        }

        // Initialize thermal properties of water
        if (Index == 0) {
            MUactual = Mu[Index];
            Kactual = Conductivity[Index];
            PRactual = Pr[Index];
        } else if (Index > NumOfPropDivisions - 1) {
            Index = NumOfPropDivisions - 1;
            MUactual = Mu[Index];
            Kactual = Conductivity[Index];
            PRactual = Pr[Index];
        } else {
            InterpFrac = (Temperature - Temps[Index - 1]) / (Temps[Index] - Temps[Index - 1]);
            MUactual = Mu[Index - 1] + InterpFrac * (Mu[Index] - Mu[Index - 1]);
            Kactual = Conductivity[Index - 1] + InterpFrac * (Conductivity[Index] - Conductivity[Index - 1]);
            PRactual = Pr[Index - 1] + InterpFrac * (Pr[Index] - Pr[Index - 1]);
        }
        // arguments are glycol name, temperature, and concentration
        if (Temperature < 0.0) { // check if fluid is water and would be freezing
            if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).FluidIndex == WaterIndex) {
                if (this->FrozenErrIndex1 == 0) {
                    ShowWarningMessage(
                        state,
                        format("GroundHeatExchanger:Surface=\"{}\", water is frozen; Model not valid. Calculated Water Temperature=[{:.2R}] C",
                               this->Name,
                               this->InletTemp));
                    ShowContinueErrorTimeStamp(state, "");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "GroundHeatExchanger:Surface=\"" + this->Name + "\", water is frozen",
                                               this->FrozenErrIndex1,
                                               this->InletTemp,
                                               this->InletTemp,
                                               _,
                                               "[C]",
                                               "[C]");
                this->InletTemp = max(this->InletTemp, 0.0);
            }
        }
        CpWater = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, Temperature, RoutineName);

        // Calculate the Reynold's number from RE=(4*Mdot)/(Pi*Mu*Diameter)
        ReD = 4.0 * WaterMassFlow / (Constant::Pi * MUactual * this->TubeDiameter * this->TubeCircuits);

        // Calculate the Nusselt number based on what flow regime one is in
        if (ReD >= MaxLaminarRe) { // Turbulent flow --> use Colburn equation
            NuD = 0.023 * std::pow(ReD, 0.8) * std::pow(PRactual, 1.0 / 3.0);
        } else { // Laminar flow --> use constant surface temperature relation
            NuD = 3.66;
        }
        // Calculate the NTU parameter
        // NTU = UA/[(Mdot*Cp)min]
        // where: U = h (convection coefficient) and h = (k)(Nu)/D
        //        A = Pi*D*TubeLength
        //  NTU = PI * Kactual * NuD * SurfaceGHE(SurfaceGHENum)%TubeLength / (WaterMassFlow * CpWater)

        PipeLength = this->SurfaceLength * this->SurfaceWidth / this->TubeSpacing;

        NTU = Constant::Pi * Kactual * NuD * PipeLength / (WaterMassFlow * CpWater);
        // Calculate Epsilon*MassFlowRate*Cp
        if (-NTU >= DataPrecisionGlobals::EXP_LowerLimit) {
            CalcHXEffectTerm = (1.0 - std::exp(-NTU)) * WaterMassFlow * CpWater;
        } else {
            CalcHXEffectTerm = 1.0 * WaterMassFlow * CpWater;
        }

        return CalcHXEffectTerm;
    }

    void SurfaceGroundHeatExchangerData::CalcTopSurfTemp(Real64 const FluxTop,             // top surface flux
                                                         Real64 &TempTop,                  // top surface temperature
                                                         Real64 const ThisDryBulb,         // dry bulb temperature
                                                         Real64 const ThisWetBulb,         // wet bulb temperature
                                                         Real64 const ThisSkyTemp,         // sky temperature
                                                         Real64 const ThisBeamSolarRad,    // beam solar radiation
                                                         Real64 const ThisDifSolarRad,     // diffuse solar radiation
                                                         Real64 const ThisSolarDirCosVert, // vertical component of solar normal
                                                         Real64 const ThisWindSpeed,       // wind speed
                                                         bool const ThisIsRain,            // rain flag
                                                         bool const ThisIsSnow             // snow flag
    )
    {

        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is used to calculate the top surface
        // temperature for the given surface flux.

        // METHODOLOGY EMPLOYED:
        // calc surface heat balance

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ConvCoef;     // convection coefficient
        Real64 RadCoef;      // radiation coefficient
        Real64 ExternalTemp; // external environmental temp - drybulb or wetbulb
        Real64 OldSurfTemp;  // previous surface temperature
        Real64 QSolAbsorbed; // absorbed solar flux
        Real64 SurfTempAbs;  // absolute value of surface temp
        Real64 SkyTempAbs;   // absolute value of sky temp

        // make a surface heat balance and solve for temperature

        // set appropriate external temp
        if (ThisIsSnow || ThisIsRain) {
            ExternalTemp = ThisWetBulb;
        } else { // normal dry conditions
            ExternalTemp = ThisDryBulb;
        }

        // set previous surface temp
        OldSurfTemp = this->TtopHistory[1];
        // absolute temperatures
        SurfTempAbs = OldSurfTemp + Constant::Kelvin;
        SkyTempAbs = ThisSkyTemp + Constant::Kelvin;

        // ASHRAE simple convection coefficient model for external surfaces.
        ConvCoef = Convect::CalcASHRAESimpExtConvCoeff(this->TopRoughness, ThisWindSpeed);
        // radiation coefficient using surf temp from past time step
        if (std::abs(SurfTempAbs - SkyTempAbs) > SmallNum) {
            RadCoef = StefBoltzmann * this->TopThermAbs * (pow_4(SurfTempAbs) - pow_4(SkyTempAbs)) / (SurfTempAbs - SkyTempAbs);
        } else {
            RadCoef = 0.0;
        }

        // total absorbed solar - no ground solar
        QSolAbsorbed = this->TopSolarAbs * (max(ThisSolarDirCosVert, 0.0) * ThisBeamSolarRad + ThisDifSolarRad);

        // solve for temperature
        TempTop = (FluxTop + ConvCoef * ExternalTemp + RadCoef * ThisSkyTemp + QSolAbsorbed) / (ConvCoef + RadCoef);
    }

    void SurfaceGroundHeatExchangerData::CalcBottomSurfTemp(Real64 const FluxBtm,       // bottom surface flux
                                                            Real64 &TempBtm,            // bottom surface temperature
                                                            Real64 const ThisDryBulb,   // dry bulb temperature
                                                            Real64 const ThisWindSpeed, // wind speed
                                                            Real64 const ThisGroundTemp // ground temperature
    )
    {

        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is used to calculate the bottom surface
        // temperature for the given surface flux.

        // METHODOLOGY EMPLOYED:
        // calc surface heat balances

        // Using/Aliasing

        Real64 ConvCoef;    // convection coefficient
        Real64 RadCoef;     // radiation coefficient
        Real64 OldSurfTemp; // previous surface temperature
        Real64 SurfTempAbs; // absolute value of surface temp
        Real64 ExtTempAbs;  // absolute value of sky temp

        if (this->LowerSurfCond == SurfCond_Exposed) {

            // make a surface heat balance and solve for temperature
            OldSurfTemp = this->TbtmHistory[1];
            // absolute temperatures
            SurfTempAbs = OldSurfTemp + Constant::Kelvin;
            ExtTempAbs = ThisDryBulb + Constant::Kelvin;

            // ASHRAE simple convection coefficient model for external surfaces.
            ConvCoef = Convect::CalcASHRAESimpExtConvCoeff(this->TopRoughness, ThisWindSpeed);

            // radiation coefficient using surf temp from past time step
            if (std::abs(SurfTempAbs - ExtTempAbs) > SmallNum) {
                RadCoef = StefBoltzmann * this->TopThermAbs * (pow_4(SurfTempAbs) - pow_4(ExtTempAbs)) / (SurfTempAbs - ExtTempAbs);
            } else {
                RadCoef = 0.0;
            }

            // total absorbed solar - no ground solar
            TempBtm = (FluxBtm + ConvCoef * ThisDryBulb + RadCoef * ThisDryBulb) / (ConvCoef + RadCoef);

        } else { // ground coupled
            // just use the supplied ground temperature
            TempBtm = ThisGroundTemp;
        }
    }

    void SurfaceGroundHeatExchangerData::UpdateSurfaceGroundHeatExchngr(EnergyPlusData &state) // Index for the surface
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does any updating that needs to be done for surface
        // ground heat exchangers.  One of the most important functions of
        // this routine is to update the average heat source/sink for a
        // particular system over the various system time steps that make up
        // the zone time step. This routine must also set the outlet water conditions.

        // METHODOLOGY EMPLOYED:
        // For the source/sink average update, if the system time step elapsed
        // is still what it used to be, then either we are still iterating or
        // we had to go back and shorten the time step.  As a result, we have
        // to subtract out the previous value that we added.  If the system
        // time step elapsed is different, then we just need to add the new
        // values to the running average.

        // Using/Aliasing
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        using PlantUtilities::SafeCopyPlantNode;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SurfaceGroundHeatExchanger:Update");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CpFluid; // Specific heat of working fluid

        // update flux
        this->QSrc = state.dataSurfaceGroundHeatExchangers->SourceFlux;

        if (this->LastSysTimeElapsed == SysTimeElapsed) { // only update in normal mode
            // Still iterating or reducing system time step, so subtract old values which were not valid
            this->QSrcAvg -= this->LastQSrc * this->LastTimeStepSys / state.dataGlobal->TimeStepZone;

            // Update the running average and the "last" values with the current values of the appropriate variables
            this->QSrcAvg += this->QSrc * TimeStepSys / state.dataGlobal->TimeStepZone;

            this->LastQSrc = state.dataSurfaceGroundHeatExchangers->SourceFlux;
            this->LastSysTimeElapsed = SysTimeElapsed;
            this->LastTimeStepSys = TimeStepSys;
        }

        // Calculate the water side outlet conditions and set the
        // appropriate conditions on the correct HVAC node.
        if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).FluidName == "WATER") {
            if (InletTemp < 0.0) {
                ShowRecurringWarningErrorAtEnd(state,
                                               "UpdateSurfaceGroundHeatExchngr: Water is frozen in Surf HX=" + this->Name,
                                               this->FrozenErrIndex2,
                                               this->InletTemp,
                                               this->InletTemp);
            }
            this->InletTemp = max(this->InletTemp, 0.0);
        }

        CpFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->InletTemp, RoutineName);

        SafeCopyPlantNode(state, this->InletNodeNum, this->OutletNodeNum);
        // check for flow
        if ((CpFluid > 0.0) && (state.dataSurfaceGroundHeatExchangers->FlowRate > 0.0)) {
            state.dataLoopNodes->Node(this->OutletNodeNum).Temp = this->InletTemp - this->SurfaceArea *
                                                                                        state.dataSurfaceGroundHeatExchangers->SourceFlux /
                                                                                        (state.dataSurfaceGroundHeatExchangers->FlowRate * CpFluid);
            state.dataLoopNodes->Node(this->OutletNodeNum).Enthalpy = state.dataLoopNodes->Node(this->OutletNodeNum).Temp * CpFluid;
        }
    }

    void SurfaceGroundHeatExchangerData::ReportSurfaceGroundHeatExchngr(EnergyPlusData &state) // Index for the surface under consideration
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Rees
        //       DATE WRITTEN   August 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simply produces output for Surface ground heat exchangers

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        // update flows and temps from node data
        this->InletTemp = state.dataLoopNodes->Node(this->InletNodeNum).Temp;
        this->OutletTemp = state.dataLoopNodes->Node(this->OutletNodeNum).Temp;
        this->MassFlowRate = state.dataLoopNodes->Node(this->InletNodeNum).MassFlowRate;

        // update other variables from module variables
        this->HeatTransferRate = state.dataSurfaceGroundHeatExchangers->SourceFlux * this->SurfaceArea;
        this->SurfHeatTransferRate =
            this->SurfaceArea * (state.dataSurfaceGroundHeatExchangers->TopSurfFlux + state.dataSurfaceGroundHeatExchangers->BtmSurfFlux);
        this->Energy = state.dataSurfaceGroundHeatExchangers->SourceFlux * this->SurfaceArea * TimeStepSysSec;
        this->TopSurfaceTemp = state.dataSurfaceGroundHeatExchangers->TopSurfTemp;
        this->BtmSurfaceTemp = state.dataSurfaceGroundHeatExchangers->BtmSurfTemp;
        this->TopSurfaceFlux = state.dataSurfaceGroundHeatExchangers->TopSurfFlux;
        this->BtmSurfaceFlux = state.dataSurfaceGroundHeatExchangers->BtmSurfFlux;
        this->SurfEnergy =
            SurfaceArea * (state.dataSurfaceGroundHeatExchangers->TopSurfFlux + state.dataSurfaceGroundHeatExchangers->BtmSurfFlux) * TimeStepSysSec;
    }
    void SurfaceGroundHeatExchangerData::oneTimeInit_new(EnergyPlusData &state)
    {

        using PlantUtilities::InitComponentNodes;
        using PlantUtilities::RegisterPlantCompDesignFlow;
        using PlantUtilities::ScanPlantLoopsForObject;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr DesignVelocity(0.5); // Hypothetical design max pipe velocity [m/s]
        Real64 rho;                           // local fluid density
        bool errFlag;
        static std::string const RoutineName("InitSurfaceGroundHeatExchanger");

        // Locate the hx on the plant loops for later usage
        errFlag = false;
        ScanPlantLoopsForObject(state, this->Name, DataPlant::PlantEquipmentType::GrndHtExchgSurface, this->plantLoc, errFlag, _, _, _, _, _);

        if (errFlag) {
            ShowFatalError(state, "InitSurfaceGroundHeatExchanger: Program terminated due to previous condition(s).");
        }
        rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, 0.0, RoutineName);
        this->DesignMassFlowRate = Constant::Pi / 4.0 * pow_2(this->TubeDiameter) * DesignVelocity * rho * this->TubeCircuits;
        InitComponentNodes(state, 0.0, this->DesignMassFlowRate, this->InletNodeNum, this->OutletNodeNum);
        RegisterPlantCompDesignFlow(state, this->InletNodeNum, this->DesignMassFlowRate / rho);
    }
    void SurfaceGroundHeatExchangerData::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
    {
    }

} // namespace SurfaceGroundHeatExchanger

} // namespace EnergyPlus
