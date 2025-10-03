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
#include <algorithm>
#include <cmath>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1S.hh>
#include <ObjexxFCL/ArrayS.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EconomicTariff.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACSizingSimulationManager.hh>
#include <EnergyPlus/HVACSystemRootFindingAlgorithm.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/MatrixDataManager.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/PhaseChangeModeling/HysteresisModel.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/StringUtilities.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/SurfaceOctree.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowComplexManager.hh>
#include <EnergyPlus/WindowEquivalentLayer.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace HeatBalanceManager {

    // Module containing the heat balance simulation routines
    // calculation (initialization) routines

    // MODULE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       November 1998, FW
    //       MODIFIED       April 1999, LKL
    //       MODIFIED       Dec 2006 DJS of PSU for ecoroof
    //       Added          Dec 2008 TH for thermochromic windows:
    //                       new subroutine CreateTCConstructions called by GetHeatBalanceInput

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the heat balance simulation on the building.

    // REFERENCES:
    // The heat balance method is outlined in the "Tarp Algorithms Manual"
    // The methods are also summarized in many BSO Theses and papers.

    // OTHER NOTES:
    // This module was created from IBLAST subroutines

    Array1D_string const PassFail(2, {"Fail", "Pass"});

    void ManageHeatBalance(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   January 1997
        //       MODIFIED       February 1998 Richard Liesen

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the heat balance method of calculating
        // building thermal loads.  It is called from the SimulationManager
        // at the time step level.  This driver manages the calls to all of
        // the other modules, drivers, and simulation algorithms.

        // METHODOLOGY EMPLOYED:
        // The order of this routine was taken from HeatBalanceModule with routine
        //  and Data Structuring

        // REFERENCES:
        // Legacy code from (I)BLAST, subroutine SIMZGD.

        // Using/Aliasing
        using namespace HeatBalanceSurfaceManager;
        using EMSManager::ManageEMS;
        using EMSManager::UpdateEMSTrendVariables;

        // Get the heat balance input at the beginning of the simulation only
        if (state.dataHeatBalMgr->ManageHeatBalanceGetInputFlag) {
            GetHeatBalanceInput(state); // Obtains heat balance related parameters from input file
            if (state.dataGlobal->DoingSizing) {
                state.dataHeatBal->doSpaceHeatBalance = state.dataHeatBal->doSpaceHeatBalanceSizing;
            }

            // Surface octree setup
            //  The surface octree holds live references to surfaces so it must be updated
            //   if in the future surfaces are altered after this point
            if (state.dataSurface->TotSurfaces >= Dayltg::octreeCrossover) {                                            // Octree can be active
                if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Daylighting:Controls") > 0) { // Daylighting is active
                    state.dataHeatBalMgr->surfaceOctree.init(state.dataSurface->Surface);                               // Set up surface octree
                }
            }

            for (auto &surface : state.dataSurface->Surface) {
                surface.set_computed_geometry(); // Set up extra surface geometry info for PierceSurface
            }

            state.dataHeatBalMgr->ManageHeatBalanceGetInputFlag = false;
        }

        bool anyRan;

        ManageEMS(state,
                  EMSManager::EMSCallFrom::BeginZoneTimestepBeforeInitHeatBalance,
                  anyRan,
                  ObjexxFCL::Optional_int_const()); // EMS calling point

        // These Inits will still have to be looked at as the routines are re-engineered further

        InitHeatBalance(state); // Initialize all heat balance related parameters
        ManageEMS(
            state, EMSManager::EMSCallFrom::BeginZoneTimestepAfterInitHeatBalance, anyRan, ObjexxFCL::Optional_int_const()); // EMS calling point

        // Solve the zone heat balance by first calling the Surface Heat Balance Manager
        // and then the Air Heat Balance Manager is called by the Surface Heat Balance
        // Manager.  The order of execution is still important and the zone cannot
        // go through any record keeping before the HVAC system has run because there
        // may be a radiant system in the building which will require iteration between
        // the HVAC system (called from the Air Heat Balance) and the zone (simulated
        // in the Surface Heat Balance Manager).  In the future, this may be improved.
        ManageSurfaceHeatBalance(state);
        ManageEMS(state, EMSManager::EMSCallFrom::EndZoneTimestepBeforeZoneReporting, anyRan, ObjexxFCL::Optional_int_const()); // EMS calling point
        RecKeepHeatBalance(state); // Do any heat balance related record keeping

        // This call has been moved to the FanSystemModule and does effect the output file
        //   You do get a shift in the Air Handling System Summary for the building electric loads
        // IF ((.NOT.WarmupFlag).AND.(DayOfSim.GT.0)) CALL RCKEEP  ! Do fan system accounting (to be moved later)

        ReportHeatBalance(state); // Manage heat balance reporting until the new reporting is in place

        ManageEMS(state, EMSManager::EMSCallFrom::EndZoneTimestepAfterZoneReporting, anyRan, ObjexxFCL::Optional_int_const()); // EMS calling point

        UpdateEMSTrendVariables(state);
        EnergyPlus::PluginManagement::PluginManager::updatePluginValues(state);

        if (state.dataGlobal->WarmupFlag && state.dataGlobal->EndDayFlag) {

            CheckWarmupConvergence(state);
            if (!state.dataGlobal->WarmupFlag) {
                state.dataGlobal->DayOfSim = 0; // Reset DayOfSim if Warmup converged
                state.dataGlobal->DayOfSimChr = "0";

                ManageEMS(state, EMSManager::EMSCallFrom::BeginNewEnvironmentAfterWarmUp, anyRan, ObjexxFCL::Optional_int_const()); // calling point
            }
        }

        if (!state.dataGlobal->WarmupFlag && state.dataGlobal->EndDayFlag && state.dataGlobal->DayOfSim == 1 && !state.dataGlobal->DoingSizing) {
            ReportWarmupConvergence(state);
        }
    }

    // Get Input Section of the Module
    //******************************************************************************

    void GetHeatBalanceInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   September 1997
        //       MODIFIED       February 1998 Richard Liesen
        //                      November 1998 FW

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main driver for initializations within the
        // heat balance.

        // Using/Aliasing
        using InternalHeatGains::ManageInternalHeatGains;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false); // If errors detected in input

        GetProjectControlData(state, ErrorsFound);

        GetSiteAtmosphereData(state, ErrorsFound);

        Material::GetWindowGlassSpectralData(state, ErrorsFound);

        Material::GetMaterialData(state, ErrorsFound); // Read materials from input file/transfer from legacy data structure

        Material::GetHysteresisData(state, ErrorsFound);

        GetFrameAndDividerData(state);

        GetConstructData(state, ErrorsFound); // Read constructs from input file/transfer from legacy data structure

        GetBuildingData(state, ErrorsFound); // Read building data from input file

        DataSurfaces::GetVariableAbsorptanceSurfaceList(state);

        GetIncidentSolarMultiplier(state, ErrorsFound);

        // Added SV 6/26/2013 to load scheduled surface gains
        GetScheduledSurfaceGains(state, ErrorsFound);

        if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
            print(state.files.eio, "{}\n", "! <Representative Surface Assignment>,Surface Name,Representative Surface Name");
            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                int RepSurfNum = state.dataSurface->Surface(SurfNum).RepresentativeCalcSurfNum;
                if (SurfNum != RepSurfNum) {
                    print(state.files.eio,
                          " Representative Surface Assignment,{},{}\n",
                          state.dataSurface->Surface(SurfNum).Name,
                          state.dataSurface->Surface(RepSurfNum).Name);
                }
            }
        }

        // Added TH 1/9/2009 to create thermochromic window constructions
        CreateTCConstructions(state, ErrorsFound);

        if (state.dataSurface->TotSurfaces > 0 && state.dataGlobal->NumOfZones == 0) {
            bool ValidSimulationWithNoZones = CheckValidSimulationObjects(state);
            if (!ValidSimulationWithNoZones) {
                ShowSevereError(state, "GetHeatBalanceInput: There are surfaces in input but no zones found.  Invalid simulation.");
                ErrorsFound = true;
            }
        }

        CheckUsedConstructions(state, ErrorsFound);

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in Building Input, Program Stopped");
        }

        // Set up enclosures before processing internal gains input
        HeatBalanceIntRadExchange::InitSolarViewFactors(state);

        // following is done to "get internal heat gains" input so that lights are gotten before
        // daylighting input
        ManageInternalHeatGains(state, true);

        // following is done so that people are gotten before for thermal comfort calculations
        // Setup Kiva instances
        if (state.dataHeatBal->AnyKiva) {
            state.dataSurfaceGeometry->kivaManager.setupKivaInstances(state);
        }
    }

    void CheckUsedConstructions(EnergyPlusData &state, [[maybe_unused]] bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2011

        // PURPOSE OF THIS SUBROUTINE:
        // Counts or details unused constructions.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr NumConstrObjects(6);
        Array1D_string const ConstrObjects(NumConstrObjects,
                                           {"Pipe:Indoor",
                                            "Pipe:Outdoor",
                                            "Pipe:Underground",
                                            "GroundHeatExchanger:Surface",
                                            "DaylightingDevice:Tubular",
                                            "EnergyManagementSystem:ConstructionIndexVariable"});

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;
        int NumNumbers;
        int Status;
        int CNum;

        // Needs to account for Pipe:HeatTransfer/indoor, etc constructions.
        for (int ONum = 1; ONum <= NumConstrObjects; ++ONum) {
            int NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ConstrObjects(ONum));
            for (int Loop = 1; Loop <= NumObjects; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         ConstrObjects(ONum),
                                                                         Loop,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         Status);
                if (ONum == 5) {
                    CNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(4), state.dataConstruction->Construct);
                } else {
                    CNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataConstruction->Construct);
                }
                if (CNum == 0) {
                    continue;
                }
                state.dataConstruction->Construct(CNum).IsUsed = true;
                if (ONum == 4 || ONum == 6) {
                    // GroundHeatExchanger:Surface or EnergyManagementSystem:ConstructionIndexVariable
                    // Include all EMS constructions since they can potentially be used by a CTF surface
                    if (!state.dataConstruction->Construct(CNum).TypeIsWindow) {
                        state.dataConstruction->Construct(CNum).IsUsedCTF = true;
                    }
                }
            }
        }
        int Unused = state.dataHeatBal->TotConstructs - std::count_if(state.dataConstruction->Construct.begin(),
                                                                      state.dataConstruction->Construct.end(),
                                                                      [](Construction::ConstructionProps const &e) { return e.IsUsed; });
        if (Unused > 0) {
            if (!state.dataGlobal->DisplayExtraWarnings) {
                ShowWarningError(state, format("CheckUsedConstructions: There are {} nominally unused constructions in input.", Unused));
                ShowContinueError(state, "For explicit details on each unused construction, use Output:Diagnostics,DisplayExtraWarnings;");
            } else {
                ShowWarningError(state, format("CheckUsedConstructions: There are {} nominally unused constructions in input.", Unused));
                ShowContinueError(state, "Each Unused construction is shown.");
                for (int Loop = 1; Loop <= state.dataHeatBal->TotConstructs; ++Loop) {
                    if (state.dataConstruction->Construct(Loop).IsUsed) {
                        continue;
                    }
                    ShowMessage(state, format("Construction={}", state.dataConstruction->Construct(Loop).Name));
                }
            }
        }
    }

    bool CheckValidSimulationObjects(EnergyPlusData &state)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   July 2008

        // PURPOSE OF THIS FUNCTION:
        // If an input file presents with surfaces but no zones, there are certain objects
        // that must be present for the simulation to be valid.  This check was necessitated by
        // an input file that was entirely detached shading surfaces but no zones (and nothing else).
        // Other objects include Solar Collectors, PV arrays.

        // METHODOLOGY EMPLOYED:
        // Check for specific objects that must be present for such a simulation to be valid.

        // Return value
        bool ValidSimulation; // True is other objects appear to make this a valid simulation.

        ValidSimulation = false;
        if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "SolarCollector:FlatPlate:Water") > 0) {
            ValidSimulation = true;
        } else if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:Photovoltaic") > 0) {
            ValidSimulation = true;
        } else if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:InternalCombustionEngine") > 0) {
            ValidSimulation = true;
        } else if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:CombustionTurbine") > 0) {
            ValidSimulation = true;
        } else if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:FuelCell") > 0) {
            ValidSimulation = true;
        } else if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:MicroCHP") > 0) {
            ValidSimulation = true;
        } else if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:MicroTurbine") > 0) {
            ValidSimulation = true;
        } else if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:WindTurbine") > 0) {
            ValidSimulation = true;
        }

        return ValidSimulation;
    }

    void SetPreConstructionInputParameters(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   October 2014

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets parameters that need to be established before any heat balance inputs are read

        int NumAlpha;
        int NumNumber;
        int IOStat;

        // Get all the construction objects to determine the max layers and use this as the value for DataHeatBalance::MaxSolidWinLayers
        // The variable MaxSolidWinLayers is initialized to zero to immediately catch any issues with timing of this routine

        // start by setting this to 5; it will satisfy the regular window constructions (Construction) and the Window5 files
        // (Construction:WindowDataFile)
        state.dataHeatBal->MaxSolidWinLayers = 7;

        // Construction:ComplexFenestrationState have a limit of 10 layers, so set it up to 10 if they are present
        if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:ComplexFenestrationState") > 0) {
            state.dataHeatBal->MaxSolidWinLayers = max(state.dataHeatBal->MaxSolidWinLayers, 10);
        }

        // then process the rest of the relevant constructions
        std::string constructName("Construction:WindowEquivalentLayer");
        int numConstructions(state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, constructName));
        for (int constructionNum = 1; constructionNum <= numConstructions; ++constructionNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     constructName,
                                                                     constructionNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlpha,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            int numLayersInThisConstruct(NumAlpha - 1);
            state.dataHeatBal->MaxSolidWinLayers = max(state.dataHeatBal->MaxSolidWinLayers, numLayersInThisConstruct);
        }

        // construction types being ignored as they are opaque: Construction:CfactorUndergroundWall, Construction:FfactorGroundFloor,
    }

    void GetProjectControlData(EnergyPlusData &state, bool &ErrorsFound) // Set to true if errors detected during getting data
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   October 2004

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the project control data before the rest of the building data (such as
        // materials) is obtained.

        // REFERENCES:
        // This routine gets the following objects:
        // BUILDING
        // INSIDE CONVECTION ALGORITHM
        // OUTSIDE CONVECTION ALGORITHM
        // SOLUTION ALGORITHM
        // ASHRAE Handbook of Fundamentals, Chap 16, for the setting of Site Atmospheric defaults based
        //   on terrain.
        // ZoneAirHeatBalanceAlgorithm, Added by L. Gu, 12/09
        // ZoneAirContaminantBalance, Added by L. Gu, 06/10

        // Using/Aliasing
        auto &HVACSystemRootFinding = state.dataRootFinder->HVACSystemRootFinding;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetProjectControlData: ");
        static constexpr std::string_view routineName = "GetProjectControlData";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string AlphaName(4);
        Array1D<Real64> BuildingNumbers(5);
        int NumAlpha;
        int NumNumber;
        int IOStat;
        std::string::size_type TMP;

        // Assign the values to the building data

        state.dataHeatBalMgr->CurrentModuleObject = "Building";
        int NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);

        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            // Building Name (remove certain characters)
            state.dataHeatBal->BuildingName = AlphaName(1);
            TMP = index(state.dataHeatBal->BuildingName, char(1));
            while (TMP != std::string::npos) {
                state.dataHeatBal->BuildingName[TMP] = ',';
                TMP = index(state.dataHeatBal->BuildingName, char(1));
            }
            TMP = index(state.dataHeatBal->BuildingName, char(2));
            while (TMP != std::string::npos) {
                state.dataHeatBal->BuildingName[TMP] = '!';
                TMP = index(state.dataHeatBal->BuildingName, char(2));
            }
            TMP = index(state.dataHeatBal->BuildingName, char(3));
            while (TMP != std::string::npos) {
                state.dataHeatBal->BuildingName[TMP] = '\\';
                TMP = index(state.dataHeatBal->BuildingName, char(3));
            }
            // Building Azimuth (no validation)
            state.dataHeatBal->BuildingAzimuth = mod(BuildingNumbers(1), 360.0);
            // Terrain
            if (AlphaName(2) == "COUNTRY" || AlphaName(2) == "1") {
                state.dataEnvrn->SiteWindExp = 0.14;
                state.dataEnvrn->SiteWindBLHeight = 270.0;
                AlphaName(2) = "Country";
            } else if (AlphaName(2) == "SUBURBS" || AlphaName(2) == "2" || AlphaName(2) == "SUBURB") {
                state.dataEnvrn->SiteWindExp = 0.22;
                state.dataEnvrn->SiteWindBLHeight = 370.0;
                AlphaName(2) = "Suburbs";
            } else if (AlphaName(2) == "CITY" || AlphaName(2) == "3") {
                state.dataEnvrn->SiteWindExp = 0.33;
                state.dataEnvrn->SiteWindBLHeight = 460.0;
                AlphaName(2) = "City";
            } else if (AlphaName(2) == "OCEAN") {
                state.dataEnvrn->SiteWindExp = 0.10;
                state.dataEnvrn->SiteWindBLHeight = 210.0;
                AlphaName(2) = "Ocean";
            } else if (AlphaName(2) == "URBAN") {
                state.dataEnvrn->SiteWindExp = 0.22;
                state.dataEnvrn->SiteWindBLHeight = 370.0;
                AlphaName(2) = "Urban";
            } else {
                ShowSevereError(state,
                                format("{}{}: {} invalid={}",
                                       RoutineName,
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       state.dataIPShortCut->cAlphaFieldNames(2),
                                       AlphaName(2)));
                state.dataEnvrn->SiteWindExp = 0.14;
                state.dataEnvrn->SiteWindBLHeight = 270.0;
                AlphaName(2) = AlphaName(2) + "-invalid";
                ErrorsFound = true;
            }
            // Loads Convergence Tolerance Value
            state.dataHeatBal->LoadsConvergTol = BuildingNumbers(2);
            if (state.dataHeatBal->LoadsConvergTol <= 0.0) {
                ShowSevereError(state,
                                format("{}{}: {} value invalid, [{:.3R}]",
                                       RoutineName,
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       state.dataIPShortCut->cNumericFieldNames(2),
                                       state.dataHeatBal->LoadsConvergTol));
                ErrorsFound = true;
            }
            // Temperature Convergence Tolerance Value
            state.dataHeatBal->TempConvergTol = BuildingNumbers(3);
            if (state.dataHeatBal->TempConvergTol <= 0.0) {
                ShowSevereError(state,
                                format("{}{}: {} value invalid, [{:.3R}]",
                                       RoutineName,
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       state.dataIPShortCut->cNumericFieldNames(3),
                                       state.dataHeatBal->TempConvergTol));
                ErrorsFound = true;
            }
            // Solar Distribution
            if (has_prefix(AlphaName(3), "MIN") || AlphaName(3) == "-1" || state.dataSysVars->lMinimalShadowing) {
                state.dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::Minimal;
                AlphaName(3) = "MinimalShadowing";
                state.dataSurface->CalcSolRefl = false;
            } else if (AlphaName(3) == "FULLEXTERIOR" || AlphaName(3) == "0") {
                state.dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;
                AlphaName(3) = "FullExterior";
                state.dataSurface->CalcSolRefl = false;
            } else if (AlphaName(3) == "FULLINTERIORANDEXTERIOR" || AlphaName(3) == "1") {
                state.dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullInteriorExterior;
                AlphaName(3) = "FullInteriorAndExterior";
                state.dataSurface->CalcSolRefl = false;
            } else if (AlphaName(3) == "FULLEXTERIORWITHREFLECTIONS") {
                state.dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;
                AlphaName(3) = "FullExteriorWithReflectionsFromExteriorSurfaces";
                state.dataSurface->CalcSolRefl = true;
            } else if (AlphaName(3) == "FULLINTERIORANDEXTERIORWITHREFLECTIONS") {
                state.dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullInteriorExterior;
                AlphaName(3) = "FullInteriorAndExteriorWithReflectionsFromExteriorSurfaces";
                state.dataSurface->CalcSolRefl = true;
            } else {
                ShowSevereError(state,
                                format("{}{}: {} invalid={}",
                                       RoutineName,
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       state.dataIPShortCut->cAlphaFieldNames(3),
                                       AlphaName(3)));
                ErrorsFound = true;
                AlphaName(3) = AlphaName(3) + "-invalid";
            }
            // Maximum Number of Warmup Days
            if (!state.dataIPShortCut->lNumericFieldBlanks(4)) {
                state.dataHeatBal->MaxNumberOfWarmupDays = BuildingNumbers(4);
                if (state.dataHeatBal->MaxNumberOfWarmupDays <= 0) {
                    ShowSevereError(state,
                                    format("{}{}: {} invalid, [{}], {} will be used",
                                           RoutineName,
                                           state.dataHeatBalMgr->CurrentModuleObject,
                                           state.dataIPShortCut->cNumericFieldNames(4),
                                           state.dataHeatBal->MaxNumberOfWarmupDays,
                                           DataHeatBalance::DefaultMaxNumberOfWarmupDays));
                    state.dataHeatBal->MaxNumberOfWarmupDays = DataHeatBalance::DefaultMaxNumberOfWarmupDays;
                }
            } else {
                state.dataHeatBal->MaxNumberOfWarmupDays = DataHeatBalance::DefaultMaxNumberOfWarmupDays;
            }
            // Minimum Number of Warmup Days
            if (!state.dataIPShortCut->lNumericFieldBlanks(5)) {
                state.dataHeatBal->MinNumberOfWarmupDays = BuildingNumbers(5);
                if (state.dataHeatBal->MinNumberOfWarmupDays <= 0) {
                    ShowWarningError(state,
                                     format("{}{}: {} invalid, [{}], {} will be used",
                                            RoutineName,
                                            state.dataHeatBalMgr->CurrentModuleObject,
                                            state.dataIPShortCut->cNumericFieldNames(5),
                                            state.dataHeatBal->MinNumberOfWarmupDays,
                                            DataHeatBalance::DefaultMinNumberOfWarmupDays));
                    state.dataHeatBal->MinNumberOfWarmupDays = DataHeatBalance::DefaultMinNumberOfWarmupDays;
                }
            } else {
                state.dataHeatBal->MinNumberOfWarmupDays = DataHeatBalance::DefaultMinNumberOfWarmupDays;
            }
            if (state.dataHeatBal->MinNumberOfWarmupDays > state.dataHeatBal->MaxNumberOfWarmupDays) {
                ShowWarningError(state,
                                 format("{}{}: {} [{}]  is greater than {} [{}], {} will be used.",
                                        RoutineName,
                                        state.dataHeatBalMgr->CurrentModuleObject,
                                        state.dataIPShortCut->cNumericFieldNames(5),
                                        state.dataHeatBal->MinNumberOfWarmupDays,
                                        state.dataIPShortCut->cNumericFieldNames(4),
                                        state.dataHeatBal->MaxNumberOfWarmupDays,
                                        state.dataHeatBal->MinNumberOfWarmupDays));
                state.dataHeatBal->MaxNumberOfWarmupDays = state.dataHeatBal->MinNumberOfWarmupDays;
            }

        } else {
            ShowSevereError(state, format("{} A {} Object must be entered.", RoutineName, state.dataHeatBalMgr->CurrentModuleObject));
            ErrorsFound = true;
            state.dataHeatBal->BuildingName = "NOT ENTERED";
            AlphaName(2) = "NOT ENTERED";
            AlphaName(3) = "NOT ENTERED";
            state.dataHeatBal->MaxNumberOfWarmupDays = DataHeatBalance::DefaultMaxNumberOfWarmupDays;
            state.dataHeatBal->MinNumberOfWarmupDays = DataHeatBalance::DefaultMinNumberOfWarmupDays;
        }

        constexpr const char *Format_720(" Building Information,{},{:.3R},{},{:.5R},{:.5R},{},{},{}\n");
        constexpr const char *Format_721("! <Building Information>, Building Name,North Axis {{deg}},Terrain,  Loads Convergence Tolerance "
                                         "Value,Temperature Convergence Tolerance Value,  Solar Distribution,Maximum Number of Warmup Days,Minimum "
                                         "Number of Warmup Days\n");
        // Write Building Information to the initialization output file
        print(state.files.eio, Format_721);
        print(state.files.eio,
              Format_720,
              state.dataHeatBal->BuildingName,
              state.dataHeatBal->BuildingAzimuth,
              AlphaName(2),
              state.dataHeatBal->LoadsConvergTol,
              state.dataHeatBal->TempConvergTol,
              AlphaName(3),
              state.dataHeatBal->MaxNumberOfWarmupDays,
              state.dataHeatBal->MinNumberOfWarmupDays);
        // Above should be validated...

        state.dataHeatBalMgr->CurrentModuleObject = "SurfaceConvectionAlgorithm:Inside";
        NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);
        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            Convect::HcInt hcIn = static_cast<Convect::HcInt>(getEnumValue(Convect::HcIntNamesUC, AlphaName(1)));

            if (hcIn == Convect::HcInt::TrombeWall) {
                ShowSevereError(state,
                                "GetInsideConvectionAlgorithm: TrombeWall has been used as a global definition. This is a zone oriented value.  "
                                "Will be illegal in the future.");
            } else if (hcIn != Convect::HcInt::ASHRAESimple && hcIn != Convect::HcInt::ASHRAETARP && hcIn != Convect::HcInt::CeilingDiffuser &&
                       hcIn != Convect::HcInt::AdaptiveConvectionAlgorithm && hcIn != Convect::HcInt::ASTMC1340) {
                ShowWarningError(state,
                                 format("GetInsideConvectionAlgorithm: Invalid value for {}, defaulting to TARP, invalid value={}",
                                        state.dataHeatBalMgr->CurrentModuleObject,
                                        AlphaName(1)));
                hcIn = Convect::HcInt::ASHRAETARP;
            }
            state.dataHeatBal->DefaultIntConvAlgo = hcIn;
        } else {
            // default value, if not specified
            state.dataHeatBal->DefaultIntConvAlgo = Convect::HcInt::ASHRAETARP;
        }
        constexpr const char *Format_722("! <Inside Convection Algorithm>, Algorithm {{Simple | TARP | CeilingDiffuser | "
                                         "AdaptiveConvectionAlgorithm}}\nInside Convection Algorithm,{}\n");
        print(state.files.eio, Format_722, Convect::HcIntNames[static_cast<int>(state.dataHeatBal->DefaultIntConvAlgo)]);

        // Get only the first (if more were input)
        state.dataHeatBalMgr->CurrentModuleObject = "SurfaceConvectionAlgorithm:Outside";
        NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);
        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     "SurfaceConvectionAlgorithm:Outside",
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            Convect::HcExt hcOut = static_cast<Convect::HcExt>(getEnumValue(Convect::HcExtNamesUC, AlphaName(1)));

            if (hcOut != Convect::HcExt::ASHRAESimple && hcOut != Convect::HcExt::ASHRAETARP && hcOut != Convect::HcExt::MoWiTTHcOutside &&
                hcOut != Convect::HcExt::DOE2HcOutside && hcOut != Convect::HcExt::AdaptiveConvectionAlgorithm) {
                ShowWarningError(state,
                                 format("GetOutsideConvectionAlgorithm: Invalid value for {}, defaulting to DOE-2, invalid value={}",
                                        state.dataHeatBalMgr->CurrentModuleObject,
                                        AlphaName(1)));
                hcOut = Convect::HcExt::DOE2HcOutside;
            }
            state.dataHeatBal->DefaultExtConvAlgo = hcOut;
        } else {
            // default value, if not specified
            state.dataHeatBal->DefaultExtConvAlgo = Convect::HcExt::DOE2HcOutside;
        }

        constexpr const char *Format_723("! <Outside Convection Algorithm>, Algorithm {{SimpleCombined | TARP | MoWitt | DOE-2 | "
                                         "AdaptiveConvectionAlgorithm}}\nOutside Convection Algorithm,{}\n");
        print(state.files.eio, Format_723, Convect::HcExtNames[static_cast<int>(state.dataHeatBal->DefaultExtConvAlgo)]);

        state.dataHeatBalMgr->CurrentModuleObject = "HeatBalanceAlgorithm";
        NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);
        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            {
                std::string const &SELECT_CASE_var = AlphaName(1);
                // The default is CTF
                if (SELECT_CASE_var == "CONDUCTIONTRANSFERFUNCTION") {
                    state.dataHeatBal->OverallHeatTransferSolutionAlgo = DataSurfaces::HeatTransferModel::CTF;
                    state.dataHeatBal->AnyCTF = true;

                } else if (SELECT_CASE_var == "MOISTUREPENETRATIONDEPTHCONDUCTIONTRANSFERFUNCTION") {
                    state.dataHeatBal->OverallHeatTransferSolutionAlgo = DataSurfaces::HeatTransferModel::EMPD;
                    state.dataHeatBal->AnyEMPD = true;
                    state.dataHeatBal->AllCTF = false;
                } else if (SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE") {
                    state.dataHeatBal->OverallHeatTransferSolutionAlgo = DataSurfaces::HeatTransferModel::CondFD;
                    state.dataHeatBal->AnyCondFD = true;
                    state.dataHeatBal->AllCTF = false;
                    if (state.dataGlobal->TimeStepsInHour < 20) {
                        ShowSevereError(
                            state,
                            format("GetSolutionAlgorithm: {} {} is Conduction Finite Difference but Number of TimeSteps in Hour < 20, Value is {}.",
                                   state.dataHeatBalMgr->CurrentModuleObject,
                                   state.dataIPShortCut->cAlphaFieldNames(1),
                                   state.dataGlobal->TimeStepsInHour));
                        ShowContinueError(state,
                                          "...Suggested minimum number of time steps in hour for Conduction Finite Difference solutions is 20. "
                                          "Errors or inaccurate calculations may occur.");
                    }

                } else if (SELECT_CASE_var == "COMBINEDHEATANDMOISTUREFINITEELEMENT") {
                    state.dataHeatBal->OverallHeatTransferSolutionAlgo = DataSurfaces::HeatTransferModel::HAMT;
                    state.dataHeatBal->AnyHAMT = true;
                    state.dataHeatBal->AllCTF = false;
                    if (state.dataGlobal->TimeStepsInHour < 20) {
                        ShowSevereError(state,
                                        format("GetSolutionAlgorithm: {} {} is Combined Heat and Moisture Finite Element but Number of TimeSteps in "
                                               "Hour < 20, Value is {}.",
                                               state.dataHeatBalMgr->CurrentModuleObject,
                                               state.dataIPShortCut->cAlphaFieldNames(1),
                                               state.dataGlobal->TimeStepsInHour));
                        ShowContinueError(state,
                                          "...Suggested minimum number of time steps in hour for Combined Heat and Moisture Finite Element solutions "
                                          "is 20. Errors or inaccurate calculations may occur.");
                        ShowContinueError(state,
                                          "...If the simulation crashes, look at material properties (esp porosity), use timestep=60, or less layers "
                                          "in your constructions.");
                    }

                } else {
                    state.dataHeatBal->OverallHeatTransferSolutionAlgo = DataSurfaces::HeatTransferModel::CTF;
                    state.dataHeatBal->AnyCTF = true;
                }
            }

            if (NumNumber > 0) {
                state.dataHeatBalSurf->MaxSurfaceTempLimit = BuildingNumbers(1);
                state.dataHeatBalSurf->MaxSurfaceTempLimitBeforeFatal = state.dataHeatBalSurf->MaxSurfaceTempLimit * 2.5;
                if (state.dataHeatBalSurf->MaxSurfaceTempLimit < DataHeatBalSurface::MinSurfaceTempLimit) {
                } else if (state.dataHeatBalSurf->MaxSurfaceTempLimit < 0.0) {
                    state.dataHeatBalSurf->MaxSurfaceTempLimit = DataHeatBalSurface::DefaultSurfaceTempLimit;
                    state.dataHeatBalSurf->MaxSurfaceTempLimitBeforeFatal = state.dataHeatBalSurf->MaxSurfaceTempLimit * 2.5;
                }
            }

            if (!state.dataIPShortCut->lNumericFieldBlanks(2)) {
                state.dataHeatBal->LowHConvLimit = BuildingNumbers(2);
            }
            if (!state.dataIPShortCut->lNumericFieldBlanks(3)) {
                state.dataHeatBal->HighHConvLimit = BuildingNumbers(3);
            }

        } else {
            state.dataHeatBal->OverallHeatTransferSolutionAlgo = DataSurfaces::HeatTransferModel::CTF;
            state.dataHeatBal->AnyCTF = true;
            state.dataHeatBalSurf->MaxSurfaceTempLimit = DataHeatBalSurface::DefaultSurfaceTempLimit;
            state.dataHeatBalSurf->MaxSurfaceTempLimitBeforeFatal = state.dataHeatBalSurf->MaxSurfaceTempLimit * 2.5;
        }

        // algorithm input checks now deferred until surface properties are read in,
        //  moved to SurfaceGeometry.cc routine GetSurfaceHeatTransferAlgorithmOverrides

        constexpr const char *Format_724("! <Sky Radiance Distribution>, Value {{Anisotropic}}\nSky Radiance Distribution,Anisotropic\n");
        print(state.files.eio, Format_724);

        state.dataHeatBalMgr->CurrentModuleObject = "Compliance:Building";
        NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);

        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            // Building Rotation for Appendix G
            state.dataHeatBal->BuildingRotationAppendixG = mod(BuildingNumbers(1), 360.0);
        }

        // A new object is added by L. Gu, 12/09
        state.dataHeatBalMgr->CurrentModuleObject = "ZoneAirHeatBalanceAlgorithm";
        NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);
        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            if (NumAlpha > 0) {
                {
                    std::string const &SELECT_CASE_var = AlphaName(1);
                    if (SELECT_CASE_var == "THIRDORDERBACKWARDDIFFERENCE") {
                        state.dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::ThirdOrder;
                        AlphaName(1) = "ThirdOrderBackwardDifference";
                    } else if (SELECT_CASE_var == "ANALYTICALSOLUTION") {
                        state.dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::AnalyticalSolution;
                        AlphaName(1) = "AnalyticalSolution";
                    } else if (SELECT_CASE_var == "EULERMETHOD") {
                        state.dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::EulerMethod;
                        AlphaName(1) = "EulerMethod";
                    } else {
                        state.dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::ThirdOrder;
                        AlphaName(1) = "ThirdOrderBackwardDifference";
                        ShowWarningError(state,
                                         format("{}: Invalid input of {}. The default choice is assigned = {}",
                                                state.dataHeatBalMgr->CurrentModuleObject,
                                                state.dataIPShortCut->cAlphaFieldNames(1),
                                                AlphaName(1)));
                        ShowContinueError(state, "Valid choices are: ThirdOrderBackwardDifference, AnalyticalSolution, or EulerMethod.");
                    }
                }
            }
            if (!state.dataIPShortCut->lAlphaFieldBlanks(2)) {
                state.dataHeatBal->doSpaceHeatBalanceSizing = static_cast<bool>(getYesNoValue(AlphaName(2)));
            }
            if (!state.dataIPShortCut->lAlphaFieldBlanks(3)) {
                state.dataHeatBal->doSpaceHeatBalanceSimulation = static_cast<bool>(getYesNoValue(AlphaName(3)));
            }
        } else {
            state.dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::ThirdOrder;
            AlphaName(1) = "ThirdOrderBackwardDifference";
        }
        if (state.dataHeatBal->OverrideZoneAirSolutionAlgo) {
            state.dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::EulerMethod;
            AlphaName(1) = "EulerMethod";
        }

        // Write Solution Algorithm to the initialization output file for User Verification
        constexpr const char *Format_726("! <Zone Air Solution Algorithm>, Algorithm {{ThirdOrderBackwardDifference | AnalyticalSolution | "
                                         "EulerMethod}}, Space Heat Balance Sizing, Space Heat Balance Simulation\n");
        print(state.files.eio, Format_726);
        constexpr const char *Format_727(" Zone Air Solution Algorithm, {}, {}, {}\n");
        print(state.files.eio,
              Format_727,
              AlphaName(1),
              state.dataHeatBal->doSpaceHeatBalanceSizing ? "Yes" : "No",
              state.dataHeatBal->doSpaceHeatBalanceSimulation ? "Yes" : "No");

        // A new object is added by L. Gu, 06/10
        state.dataHeatBalMgr->CurrentModuleObject = "ZoneAirContaminantBalance";
        NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);
        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, state.dataHeatBalMgr->CurrentModuleObject, state.dataHeatBalMgr->CurrentModuleObject};

            if (NumAlpha > 0) {
                { // Why an extra nested scope here?
                    std::string const &SELECT_CASE_var = AlphaName(1);
                    if (SELECT_CASE_var == "YES") {
                        state.dataContaminantBalance->Contaminant.CO2Simulation = true;
                        state.dataContaminantBalance->Contaminant.SimulateContaminants = true;
                    } else if (SELECT_CASE_var == "NO") {
                        state.dataContaminantBalance->Contaminant.CO2Simulation = false;
                    } else {
                        state.dataContaminantBalance->Contaminant.CO2Simulation = false;
                        AlphaName(1) = "NO";
                        ShowWarningError(state,
                                         format("{}: Invalid input of {}. The default choice is assigned = NO",
                                                state.dataHeatBalMgr->CurrentModuleObject,
                                                state.dataIPShortCut->cAlphaFieldNames(1)));
                    }
                }
            }

            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2));
                    ErrorsFound = true;
                } else if ((state.dataContaminantBalance->Contaminant.CO2OutdoorSched = Sched::GetSchedule(state, AlphaName(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), AlphaName(2));
                    ErrorsFound = true;
                }
            }

            if (NumAlpha > 2) {
                {
                    std::string const &SELECT_CASE_var = AlphaName(3);
                    if (SELECT_CASE_var == "YES") {
                        state.dataContaminantBalance->Contaminant.GenericContamSimulation = true;
                        if (!state.dataContaminantBalance->Contaminant.CO2Simulation) {
                            state.dataContaminantBalance->Contaminant.SimulateContaminants = true;
                        }
                    } else if (SELECT_CASE_var == "NO") {
                        state.dataContaminantBalance->Contaminant.GenericContamSimulation = false;
                    } else {
                        state.dataContaminantBalance->Contaminant.GenericContamSimulation = false;
                        AlphaName(3) = "NO";
                        ShowWarningError(state,
                                         format("{}: Invalid input of {}. The default choice is assigned = NO",
                                                state.dataHeatBalMgr->CurrentModuleObject,
                                                state.dataIPShortCut->cAlphaFieldNames(3)));
                    }
                }

                if (NumAlpha == 3 && state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    ShowSevereError(state,
                                    format("{}, {} is required and not given.",
                                           state.dataHeatBalMgr->CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaFieldNames(4)));
                    ErrorsFound = true;
                } else if (NumAlpha > 3 && state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    if ((state.dataContaminantBalance->Contaminant.genericOutdoorSched = Sched::GetSchedule(state, AlphaName(4))) == nullptr) {
                        ShowSevereError(state,
                                        format("{}, {} not found: {}",
                                               state.dataHeatBalMgr->CurrentModuleObject,
                                               state.dataIPShortCut->cAlphaFieldNames(4),
                                               AlphaName(4)));
                        ErrorsFound = true;
                    }
                }
            }
        } else {
            state.dataContaminantBalance->Contaminant.SimulateContaminants = false;
            state.dataContaminantBalance->Contaminant.CO2Simulation = false;
            state.dataContaminantBalance->Contaminant.GenericContamSimulation = false;
            AlphaName(1) = "NO";
            AlphaName(3) = "NO";
        }

        Window::initWindowModel(state);

        constexpr const char *Format_728("! <Zone Air Carbon Dioxide Balance Simulation>, Simulation {{Yes/No}}, Carbon Dioxide Concentration\n");
        print(state.files.eio, Format_728);
        constexpr const char *Format_730(" Zone Air Carbon Dioxide Balance Simulation, {},{}\n");
        if (state.dataContaminantBalance->Contaminant.SimulateContaminants && state.dataContaminantBalance->Contaminant.CO2Simulation) {
            print(state.files.eio, Format_730, "Yes", AlphaName(1));
        } else {
            print(state.files.eio, Format_730, "No", "N/A");
        }

        constexpr const char *Format_729(
            "! <Zone Air Generic Contaminant Balance Simulation>, Simulation {{Yes/No}}, Generic Contaminant Concentration\n");
        constexpr const char *Format_731(" Zone Air Generic Contaminant Balance Simulation, {},{}\n");
        print(state.files.eio, Format_729);
        if (state.dataContaminantBalance->Contaminant.SimulateContaminants && state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            print(state.files.eio, Format_731, "Yes", AlphaName(3));
        } else {
            print(state.files.eio, Format_731, "No", "N/A");
        }

        // A new object is added by B. Nigusse, 02/14
        state.dataHeatBalMgr->CurrentModuleObject = "ZoneAirMassFlowConservation";
        NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);
        state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = false;

        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            if (NumAlpha > 0) {
                {
                    int FlowTypeNum = getEnumValue(DataHeatBalance::AdjustmentTypeNamesUC, Util::makeUPPER(AlphaName(1)));
                    state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment = static_cast<DataHeatBalance::AdjustmentType>(FlowTypeNum);
                    AlphaName(1) = DataHeatBalance::AdjustmentTypeNamesCC[FlowTypeNum];
                    DataHeatBalance::AdjustmentType ZoneFlowAdjustment = state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment;
                    if ((ZoneFlowAdjustment == DataHeatBalance::AdjustmentType::AdjustMixingOnly) ||
                        (ZoneFlowAdjustment == DataHeatBalance::AdjustmentType::AdjustReturnOnly) ||
                        (ZoneFlowAdjustment == DataHeatBalance::AdjustmentType::AdjustMixingThenReturn) ||
                        (ZoneFlowAdjustment == DataHeatBalance::AdjustmentType::AdjustReturnThenMixing)) {
                        state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = true;
                    }
                    if (state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment == DataHeatBalance::AdjustmentType::Invalid) {
                        state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment = DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing;
                        AlphaName(1) = "None";
                        ShowWarningError(state,
                                         format("{}: Invalid input of {}. The default choice is assigned = None",
                                                state.dataHeatBalMgr->CurrentModuleObject,
                                                state.dataIPShortCut->cAlphaFieldNames(1)));
                    }
                }
                if (state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing) {
                    state.dataHeatBal->ZoneAirMassFlow.AdjustZoneMixingFlow = true;
                }
            }
            if (NumAlpha > 1) {
                {
                    int FlowTypeNum = getEnumValue(DataHeatBalance::InfiltrationFlowTypeNamesUC, Util::makeUPPER(AlphaName(2)));
                    state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment = static_cast<DataHeatBalance::InfiltrationFlow>(FlowTypeNum);
                    AlphaName(2) = DataHeatBalance::InfiltrationFlowTypeNamesCC[FlowTypeNum];
                    if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment == DataHeatBalance::InfiltrationFlow::Add ||
                        state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment == DataHeatBalance::InfiltrationFlow::Adjust) {
                        state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = true;
                    } else if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment == DataHeatBalance::InfiltrationFlow::Invalid) {
                        state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment = DataHeatBalance::InfiltrationFlow::Add;
                        state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = true;
                        AlphaName(2) = "AddInfiltrationFlow";
                        ShowWarningError(state,
                                         format("{}: Invalid input of {}. The default choice is assigned = AddInfiltrationFlow",
                                                state.dataHeatBalMgr->CurrentModuleObject,
                                                state.dataIPShortCut->cAlphaFieldNames(2)));
                    }
                }
            } else {
                state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment = DataHeatBalance::InfiltrationFlow::Add;
                state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = true;
                AlphaName(2) = "AddInfiltrationFlow";
            }
            if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment == DataHeatBalance::InfiltrationFlow::No) {
                AlphaName(3) = "N/A";
            } else {
                if (NumAlpha > 2) {
                    {
                        int FlowTypeNum = getEnumValue(DataHeatBalance::InfiltrationZoneTypeNamesUC, Util::makeUPPER(AlphaName(3)));
                        state.dataHeatBal->ZoneAirMassFlow.InfiltrationForZones = static_cast<DataHeatBalance::InfiltrationZoneType>(FlowTypeNum);
                        AlphaName(3) = DataHeatBalance::InfiltrationZoneTypeNamesCC[FlowTypeNum];
                        if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationForZones == DataHeatBalance::InfiltrationZoneType::Invalid) {
                            state.dataHeatBal->ZoneAirMassFlow.InfiltrationForZones = DataHeatBalance::InfiltrationZoneType::MixingSourceZonesOnly;
                            AlphaName(3) = "MixingSourceZonesOnly";
                            ShowWarningError(state,
                                             format("{}: Invalid input of {}. The default choice is assigned = MixingSourceZonesOnly",
                                                    state.dataHeatBalMgr->CurrentModuleObject,
                                                    state.dataIPShortCut->cAlphaFieldNames(3)));
                        }
                    }
                } else {
                    state.dataHeatBal->ZoneAirMassFlow.InfiltrationForZones = DataHeatBalance::InfiltrationZoneType::MixingSourceZonesOnly;
                    AlphaName(3) = "MixingSourceZonesOnly";
                }
            }
        } else {
            state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = false;
        }
        if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment != DataHeatBalance::InfiltrationFlow::No) {
            state.dataHeatBal->ZoneAirMassFlow.AdjustZoneInfiltrationFlow = true;
        }

        constexpr const char *Format_732(
            "! <Zone Air Mass Flow Balance Simulation>, Enforce Mass Balance, Adjust Zone Mixing and Return {{AdjustMixingOnly | AdjustReturnOnly | "
            "AdjustMixingThenReturn | AdjustReturnThenMixing | None}}, Adjust Zone Infiltration "
            "{{AddInfiltration | AdjustInfiltration | None}}, Infiltration Zones {{MixingSourceZonesOnly | AllZones}}\n");
        constexpr const char *Format_733(" Zone Air Mass Flow Balance Simulation, {},{},{},{}\n");

        print(state.files.eio, Format_732);
        if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
            print(state.files.eio, Format_733, "Yes", AlphaName(1), AlphaName(2), AlphaName(3));
        } else {
            print(state.files.eio, Format_733, "No", "N/A", "N/A", "N/A");
        }

        // A new object is added by L. Gu, 4/17
        state.dataHeatBalMgr->CurrentModuleObject = "HVACSystemRootFindingAlgorithm";
        NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);
        if (NumObjects > 0) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphaName,
                                                                     NumAlpha,
                                                                     BuildingNumbers,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, state.dataHeatBalMgr->CurrentModuleObject, AlphaName(1)};
            if (NumAlpha > 0) {
                HVACSystemRootFinding.Algorithm = AlphaName(1);
                HVACSystemRootFinding.HVACSystemRootSolverMethod =
                    static_cast<HVACSystemRootSolverAlgorithm>(getEnumValue(HVACSystemRootSolverAlgorithmUC, Util::makeUPPER(AlphaName(1))));
                if (HVACSystemRootFinding.HVACSystemRootSolverMethod == HVACSystemRootSolverAlgorithm::Invalid) {
                    HVACSystemRootFinding.HVACSystemRootSolverMethod = HVACSystemRootSolverAlgorithm::RegulaFalsi;
                    ShowWarningInvalidKey(
                        state, eoh, state.dataIPShortCut->cAlphaFieldNames(1), AlphaName(1), "Invalid input. The default choice is assigned.");
                    ShowContinueError(
                        state, "Valid choices are: RegulaFalsi, Bisection, BisectionThenRegulaFalsi, RegulaFalsiThenBisection, or Alternation.");
                }
            }
            if (NumNumber > 0) {
                HVACSystemRootFinding.NumOfIter = BuildingNumbers(1);
            }
        } else {
            HVACSystemRootFinding.Algorithm = "RegulaFalsi";
            HVACSystemRootFinding.HVACSystemRootSolverMethod = HVACSystemRootSolverAlgorithm::RegulaFalsi;
        }

        // Write Solution Algorithm to the initialization output file for User Verification
        constexpr const char *Format_734(
            "! <HVACSystemRootFindingAlgorithm>, Value {{RegulaFalsi | Bisection | BisectionThenRegulaFalsi | RegulaFalsiThenBisection}}\n");
        constexpr const char *Format_735(" HVACSystemRootFindingAlgorithm, {}\n");
        print(state.files.eio, Format_734);
        print(state.files.eio, Format_735, HVACSystemRootFinding.Algorithm);
    }

    void GetSiteAtmosphereData(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   January 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Reads the input data for the SITE ATMOSPHERIC VARIATION object.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string AlphArray(1); // Character string data
        Array1D<Real64> NumArray(3); // Numeric data

        // Formats
        constexpr const char *Format_720("Environment:Site Atmospheric Variation,{:.3R},{:.3R},{:.6R}\n");

        state.dataHeatBalMgr->CurrentModuleObject = "Site:HeightVariation";
        int NumObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);

        if (NumObjects == 1) {
            int NumAlphas; // Number of elements in the alpha array
            int NumNums;   // Number of elements in the numeric array
            int IOStat;    // IO Status when calling get input subroutine
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     1,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            if (NumNums > 0) {
                state.dataEnvrn->SiteWindExp = NumArray(1);
            }
            if (NumNums > 1) {
                state.dataEnvrn->SiteWindBLHeight = NumArray(2);
            }
            if (NumNums > 2) {
                state.dataEnvrn->SiteTempGradient = NumArray(3);
            }

        } else if (NumObjects > 1) {
            ShowSevereError(state, format("Too many {} objects, only 1 allowed.", state.dataHeatBalMgr->CurrentModuleObject));
            ErrorsFound = true;
        } else { //  None entered
            // IDD defaults would have this:
            // Building object defaults use Terrain to set SiteWindExp and SiteWindBLHeight but would
            // be overridden by a Site Atmospheric Variation Object.
            // SiteWindExp = 0.22
            // SiteWindBLHeight = 370.0
            state.dataEnvrn->SiteTempGradient = 0.0065;
        }

        // Write to the initialization output file
        print(state.files.eio,
              "! <Environment:Site Atmospheric Variation>,Wind Speed Profile Exponent {{}},Wind Speed Profile Boundary "
              "Layer Thickness {{m}},Air Temperature Gradient Coefficient {{K/m}}\n");

        print(state.files.eio, Format_720, state.dataEnvrn->SiteWindExp, state.dataEnvrn->SiteWindBLHeight, state.dataEnvrn->SiteTempGradient);
    }

    void GetConstructData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   September 1997
        //       MODIFIED       January 2003, FCW: accommodate between-glass shading device
        //                      July 2009, TH: added constructions defined with F and C factors

        // PURPOSE OF THIS SUBROUTINE:
        // This file reads the input through the input processor for Constructions.
        // Data read in this routine is stored in a derived type (Construct)
        // defined in the DataHeatBalance module.
        // This subroutine only sets those parameters which must be obtained
        // from the input file--all other portions of the Construct derived
        // type are set during the initializations.

        // Using/Aliasing
        using namespace DataStringGlobals;

        static constexpr std::string_view routineName = "GetConstructData";

        // If UniqueConstructionNames size, then input has already been gotten
        if (state.dataHeatBalMgr->UniqueConstructNames.size()) {
            return;
        }

        int ConstructNumAlpha;                                                   // Number of construction alpha names being passed
        int DummyNumProp;                                                        // dummy variable for properties being passed
        int IOStat;                                                              // IO Status when calling get input subroutine
        Array1D_string ConstructAlphas({0, Construction::MaxLayersInConstruct}); // Construction Alpha names defined
        Array1D<Real64> DummyProps(5);                                           // Temporary array to transfer construction properties
        int TotRegConstructs; // Number of "regular" constructions (no embedded sources or sinks and

        int TotFfactorConstructs; // Number of slabs-on-grade or underground floor constructions defined with F factors
        int TotCfactorConstructs; // Number of underground wall constructions defined with C factors

        // int TotSourceConstructs;  // Number of constructions with embedded sources or sinks
        int TotWindow5Constructs; // Number of constructions from Window5 data file
        bool ConstructionFound;   // True if input window construction name is found in the
        //  Window5 data file
        bool EOFonW5File; // True if EOF encountered reading Window5 data file

        Array1D_string WConstructNames;

        auto &s_ipsc = state.dataIPShortCut;
        auto &s_mat = state.dataMaterial;

        // Get the Total number of Constructions from the input
        TotRegConstructs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction");
        int totAirBoundaryConstructs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:AirBoundary");

        TotFfactorConstructs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:FfactorGroundFloor");
        TotCfactorConstructs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:CfactorUndergroundWall");

        if (TotFfactorConstructs > 0) {
            state.dataHeatBal->NoFfactorConstructionsUsed = false;
        }

        if (TotCfactorConstructs > 0) {
            state.dataHeatBal->NoCfactorConstructionsUsed = false;
        }

        state.dataBSDFWindow->TotComplexFenStates =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:ComplexFenestrationState");
        TotWindow5Constructs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:WindowDataFile");
        state.dataWindowEquivLayer->TotWinEquivLayerConstructs =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:WindowEquivalentLayer");

        WConstructNames.allocate(TotWindow5Constructs);

        state.dataHeatBal->TotConstructs = TotRegConstructs + TotFfactorConstructs + TotCfactorConstructs + totAirBoundaryConstructs +
                                           state.dataBSDFWindow->TotComplexFenStates + state.dataWindowEquivLayer->TotWinEquivLayerConstructs;

        state.dataHeatBal->NominalRforNominalUCalculation.dimension(state.dataHeatBal->TotConstructs, 0.0);
        state.dataHeatBal->NominalU.dimension(state.dataHeatBal->TotConstructs, 0.0);
        state.dataHeatBal->NominalUBeforeAdjusted.dimension(state.dataHeatBal->TotConstructs, 0.0);
        state.dataHeatBal->CoeffAdjRatio.dimension(state.dataHeatBal->TotConstructs, 1.0);

        // Allocate the array to the number of constructions/initialize selected variables
        state.dataConstruction->Construct.allocate(state.dataHeatBal->TotConstructs);
        state.dataHeatBalMgr->UniqueConstructNames.reserve(state.dataHeatBal->TotConstructs);
        // Note: If TotWindow5Constructs > 0, additional constructions are created in
        // subr. SearchWindow5DataFile corresponding to those found on the data file.
        for (auto &e : state.dataConstruction->Construct) {
            // Initialize CTF and History terms
            e.NumCTFTerms = 0;
            e.NumHistories = 0;

            // Initialize some heat source/sink variables
            e.SourceSinkPresent = false; // "default" is no source or sink present
            e.SolutionDimensions = 1;    // "default" is 1-D heat transfer
            e.SourceAfterLayer = 0;      // this has no meaning if a source/sink is not present
            e.TempAfterLayer = 0;        // this has no meaning if a source/sink is not present
            e.ThicknessPerpend = 0.0;    // this has no meaning if a source/sink is not present

            e.W5FrameDivider = 0;
            e.FromWindow5DataFile = false;

            // these Construct arrays dimensioned based on MaxSolidWinLayers
            e.setArraysBasedOnMaxSolidWinLayers(state);
        }

        int ConstrNum = 0;

        s_ipsc->cCurrentModuleObject = "Construction";
        for (int Loop = 1; Loop <= TotRegConstructs; ++Loop) { // Loop through all constructs in the input...

            // Get the object names for each construction from the input processor
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ConstructAlphas,
                                                                     ConstructNumAlpha,
                                                                     DummyProps,
                                                                     DummyNumProp,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, ConstructAlphas(1)};

            if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataHeatBalMgr->UniqueConstructNames,
                                                         ConstructAlphas(0),
                                                         state.dataHeatBalMgr->CurrentModuleObject,
                                                         state.dataIPShortCut->cAlphaFieldNames(1),
                                                         ErrorsFound)) {
                continue;
            }

            ++ConstrNum;
            auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            // Assign Construction name to the Derived Type using the zeroth position of the array
            thisConstruct.Name = ConstructAlphas(0);

            // Set the total number of layers for the construction
            thisConstruct.TotLayers = ConstructNumAlpha - 1;

            // Loop through all of the layers of the construct to match the material names.
            // The loop index is the number minus 1

            for (int GlassLayer = 0, Layer = 1; Layer <= ConstructNumAlpha - 1; ++Layer) {

                // Find the material in the list of materials
                thisConstruct.LayerPoint(Layer) = Material::GetMaterialNum(state, ConstructAlphas(Layer));

                if (thisConstruct.LayerPoint(Layer) == 0) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(Layer), ConstructAlphas(Layer));
                    ErrorsFound = true;
                    continue;
                }

                auto const *mat = s_mat->materials(thisConstruct.LayerPoint(Layer));
                if (mat->group == Material::Group::Glass) {
                    ++GlassLayer;
                } else if (mat->group == Material::Group::GlassEQL || mat->group == Material::Group::ShadeEQL ||
                           mat->group == Material::Group::DrapeEQL || mat->group == Material::Group::BlindEQL ||
                           mat->group == Material::Group::ScreenEQL || mat->group == Material::Group::WindowGapEQL) {
                    ShowSevereError(state, format("Invalid material layer type in window {} = {}", s_ipsc->cCurrentModuleObject, thisConstruct.Name));
                    ShowContinueError(
                        state,
                        format("Equivalent Layer material type = {} is allowed only in Construction:WindowEquivalentLayer window object.",
                               ConstructAlphas(Layer)));
                    ErrorsFound = true;
                    continue;
                } else if (mat->group == Material::Group::GlassTCParent) {
                    ++GlassLayer;
                    // reset layer pointer to the first glazing in the TC GlazingGroup
                    auto const *matGlassTC = dynamic_cast<Material::MaterialGlassTC const *>(mat);
                    assert(matGlassTC != nullptr);

                    thisConstruct.LayerPoint(Layer) = matGlassTC->matRefs(1).matNum;
                    thisConstruct.isTCWindow = true;
                    thisConstruct.isTCMaster = true;
                    thisConstruct.TCMasterConstrNum = ConstrNum;
                    thisConstruct.TCMasterMatNum = matGlassTC->Num;
                    thisConstruct.TCGlassNum = GlassLayer; // the TC glass layer ID
                    thisConstruct.TCLayerNum = Layer;
                    thisConstruct.TypeIsWindow = true;
                }

                state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum) += mat->NominalR;
                if (mat->group == Material::Group::Regular && !mat->ROnly) {
                    state.dataHeatBal->NoRegularMaterialsUsed = false;
                }

            } // ...end of the Layer DO loop

        } // ...end of Regular Construction DO loop

        TotRegConstructs = ConstrNum;

        // Added TH 7/2009 for underground walls and floors constructions
        if (TotFfactorConstructs + TotCfactorConstructs >= 1) {
            CreateFCfactorConstructions(state, ConstrNum, ErrorsFound);
            if (ErrorsFound) {
                ShowSevereError(state, "Errors found in creating the constructions defined with Ffactor or Cfactor method");
            }
            TotRegConstructs += TotFfactorConstructs + TotCfactorConstructs;
        }

        if (totAirBoundaryConstructs >= 1) {
            CreateAirBoundaryConstructions(state, ConstrNum, ErrorsFound);
            if (ErrorsFound) {
                ShowSevereError(state, "Errors found in creating the constructions defined with Construction:AirBoundary.");
            }
            TotRegConstructs += totAirBoundaryConstructs;
        }

        // Added BG 6/2010 for complex fenestration
        if (state.dataBSDFWindow->TotComplexFenStates > 0) {
            SetupComplexFenestrationStateInput(state, ConstrNum, ErrorsFound);
            if (ErrorsFound) {
                ShowSevereError(state, "Errors found in processing complex fenestration input");
            }
            TotRegConstructs += state.dataBSDFWindow->TotComplexFenStates;
        }

        state.dataHeatBalMgr->CurrentModuleObject = "ConstructionProperty:InternalHeatSource";

        auto instances = state.dataInputProcessing->inputProcessor->epJSON.find(state.dataHeatBalMgr->CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            state.dataHeatBal->AnyInternalHeatSourceInInput = true;
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                std::string const thisObjectName = Util::makeUPPER(instance.key());

                std::string construction_name = Util::makeUPPER(fields.at("construction_name").get<std::string>());
                int source_after_layer_number = fields.at("thermal_source_present_after_layer_number").get<int>();
                int calculation_after_layer_number = fields.at("temperature_calculation_requested_after_layer_number").get<int>();
                int ctf_dimensions = fields.at("dimensions_for_the_ctf_calculation").get<int>();
                if ((ctf_dimensions < 1) || (ctf_dimensions > 2)) {
                    ShowWarningError(state, "ConstructionProperty:InternalHeatSource must be either 1- or 2-D.  Reset to 1-D solution.");
                    ShowContinueError(state, format("Construction={} is affected.", construction_name));
                    ctf_dimensions = 1;
                }
                Real64 tube_spacing = fields.at("tube_spacing").get<Real64>();
                Real64 calculation_position = 0.0;
                auto const twoDimTempCalcPos = fields.find("two_dimensional_temperature_calculation_position");
                if (twoDimTempCalcPos != fields.end()) {
                    calculation_position = twoDimTempCalcPos.value().get<Real64>();
                }

                // Find the construction
                int construction_index = Util::FindItemInList(construction_name, state.dataConstruction->Construct);

                if (construction_index == 0) {
                    ShowSevereError(state,
                                    format("Did not find matching construction for {} {}, missing construction = {}",
                                           state.dataHeatBalMgr->CurrentModuleObject,
                                           thisObjectName,
                                           construction_name));
                    ErrorsFound = true;
                    continue;
                }

                state.dataInputProcessing->inputProcessor->markObjectAsUsed(state.dataHeatBalMgr->CurrentModuleObject, instance.key());

                auto &thisConstruct = state.dataConstruction->Construct(construction_index);

                // May need some additional validation of the construction here
                if (thisConstruct.SourceSinkPresent) {
                    // Emulate old behavior by disallowing two sources in a single material
                    ShowSevereError(
                        state,
                        format("Construction {} has more than one internal heat source referencing it, which is not allowed", construction_name));
                    ErrorsFound = true;
                    continue;
                }

                thisConstruct.SourceSinkPresent = true;
                thisConstruct.SourceAfterLayer = source_after_layer_number;
                thisConstruct.TempAfterLayer = calculation_after_layer_number;
                thisConstruct.SolutionDimensions = ctf_dimensions;
                thisConstruct.ThicknessPerpend = thisConstruct.setThicknessPerpendicular(state, tube_spacing);
                thisConstruct.userTemperatureLocationPerpendicular =
                    thisConstruct.setUserTemperatureLocationPerpendicular(state, calculation_position);

                // Set the total number of layers for the construction
                if ((thisConstruct.SourceAfterLayer >= thisConstruct.TotLayers) || (thisConstruct.SourceAfterLayer <= 0)) {
                    ShowWarningError(state, format("Construction {} must have a source that is between two layers", thisConstruct.Name));
                    ShowContinueError(state, "The source after layer parameter has been set to one less than the number of layers.");
                    thisConstruct.SourceAfterLayer = thisConstruct.TotLayers - 1;
                }
                if ((thisConstruct.TempAfterLayer >= thisConstruct.TotLayers) || (thisConstruct.TempAfterLayer <= 0)) {
                    ShowWarningError(state,
                                     format("Construction {} must have a temperature calculation that is between two layers", thisConstruct.Name));
                    ShowContinueError(state, "The temperature calculation after layer parameter has been set to one less than the number of layers.");
                    thisConstruct.TempAfterLayer = thisConstruct.TotLayers - 1;
                }
            }
        }

        state.dataHeatBal->TotConstructs = TotRegConstructs;

        if (state.dataHeatBal->TotConstructs > 0 && (state.dataHeatBal->NoRegularMaterialsUsed && state.dataHeatBal->NoCfactorConstructionsUsed &&
                                                     state.dataHeatBal->NoFfactorConstructionsUsed)) {
            ShowWarningError(state, "This building has no thermal mass which can cause an unstable solution.");
            ShowContinueError(state, "Use Material object for all opaque material definitions except very light insulation layers.");
        }

        ConstrNum = 0;
        state.dataHeatBalMgr->CurrentModuleObject = "Construction:WindowEquivalentLayer";
        for (int Loop = 1; Loop <= state.dataWindowEquivLayer->TotWinEquivLayerConstructs;
             ++Loop) { // Loop through all constructs with Window EquivalentLayer ...

            // Get the object names for each construction from the input processor
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     Loop,
                                                                     ConstructAlphas,
                                                                     ConstructNumAlpha,
                                                                     DummyProps,
                                                                     DummyNumProp,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataHeatBalMgr->UniqueConstructNames,
                                                         ConstructAlphas(0),
                                                         state.dataHeatBalMgr->CurrentModuleObject,
                                                         state.dataIPShortCut->cAlphaFieldNames(1),
                                                         ErrorsFound)) {
                continue;
            }

            ++ConstrNum;
            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            // Assign Construction name to the Derived Type using the zeroth position of the array
            state.dataConstruction->Construct(TotRegConstructs + ConstrNum).Name = ConstructAlphas(0);

            // Set the total number of layers for the construction
            state.dataConstruction->Construct(TotRegConstructs + ConstrNum).TotLayers = ConstructNumAlpha - 1;
            if (state.dataConstruction->Construct(TotRegConstructs + ConstrNum).TotLayers < 1) {
                ShowSevereError(state,
                                format("Construction {} must have at least a single layer",
                                       state.dataConstruction->Construct(TotRegConstructs + ConstrNum).Name));
                ErrorsFound = true;
            }

            // Loop through all of the layers of the construct to match the material names.
            // The loop index is the number minus 1
            for (int Layer = 1; Layer <= ConstructNumAlpha - 1; ++Layer) {

                // Find the material in the list of materials
                state.dataConstruction->Construct(TotRegConstructs + ConstrNum).LayerPoint(Layer) =
                    Material::GetMaterialNum(state, ConstructAlphas(Layer));

                if (state.dataConstruction->Construct(TotRegConstructs + ConstrNum).LayerPoint(Layer) == 0) {
                    ShowSevereError(state,
                                    format("Did not find matching material for {} {}, missing material = {}",
                                           state.dataHeatBalMgr->CurrentModuleObject,
                                           thisConstruct.Name,
                                           ConstructAlphas(Layer)));
                    ErrorsFound = true;
                } else {
                    auto const *mat = s_mat->materials(state.dataConstruction->Construct(TotRegConstructs + ConstrNum).LayerPoint(Layer));
                    if (!((mat->group == Material::Group::GlassEQL) || (mat->group == Material::Group::ShadeEQL) ||
                          (mat->group == Material::Group::DrapeEQL) || (mat->group == Material::Group::BlindEQL) ||
                          (mat->group == Material::Group::ScreenEQL) || (mat->group == Material::Group::WindowGapEQL))) {
                        ShowSevereError(state,
                                        format("Invalid material layer type in window {} = {}",
                                               state.dataHeatBalMgr->CurrentModuleObject,
                                               state.dataConstruction->Construct(TotRegConstructs + ConstrNum).Name));
                        ShowContinueError(state,
                                          format("...Window layer = {} is not allowed in Construction:WindowEquivalentLayer window object.",
                                                 ConstructAlphas(Layer)));
                        ShowContinueError(state, "Only materials of type Material:*:EquivalentLayer are allowed");
                        ErrorsFound = true;
                    }

                    if (ConstructNumAlpha <= 2) {

                    } else {
                        auto const *mat = s_mat->materials(state.dataConstruction->Construct(TotRegConstructs + ConstrNum).LayerPoint(Layer));
                        state.dataHeatBal->NominalRforNominalUCalculation(TotRegConstructs + ConstrNum) += mat->NominalR;
                    }
                }

            } // Layer loop
            state.dataConstruction->Construct(TotRegConstructs + ConstrNum).EQLConsPtr = ConstrNum;
            state.dataConstruction->Construct(TotRegConstructs + ConstrNum).WindowTypeEQL = true;
        } // TotWinEquivLayerConstructs loop

        state.dataWindowEquivLayer->TotWinEquivLayerConstructs = ConstrNum;
        TotRegConstructs += state.dataWindowEquivLayer->TotWinEquivLayerConstructs;
        state.dataHeatBal->TotConstructs = TotRegConstructs;
        //-------------------------------------------------------------------------------
        ConstrNum = 0;

        state.dataHeatBalMgr->CurrentModuleObject = "Construction:WindowDataFile";
        for (int Loop = 1; Loop <= TotWindow5Constructs; ++Loop) { // Loop through all Window5 constructions. These constructions come
                                                                   // from the Window5 data file and can be referenced only by windows

            // Get the object names for each construction from the input processor
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     Loop,
                                                                     ConstructAlphas,
                                                                     ConstructNumAlpha,
                                                                     DummyProps,
                                                                     DummyNumProp,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ++ConstrNum;
            WConstructNames(ConstrNum) = ConstructAlphas(0);

            // Obtain the data
            if (DummyNumProp != 0) {
                ShowSevereError(state, format("Construction From Window5 Data File: there should be no numerical inputs for {}", ConstructAlphas(0)));
                ErrorsFound = true;
                continue;
            }

            // See if this construction is in the W5DataFile produced by the WINDOW 5 program;
            // if so, ConstructionFound will be set to true and the Material objects
            // associated with the construction will be created in subr. SearchWindow5DataFile.
            // (If the matching construction on the Window5 data file has two glazing systems, a
            // second construction and its associated materials will be created in subr.
            // SearchWindow5DataFile and TotConstructs WILL BE INCREMENTED BY 1 in that routine.
            // A FrameAndDivider object will also be created if window on data file has a
            // frame or divider.)

            fs::path window5DataFilePath;
            if (ConstructAlphas(1) == "") {
                window5DataFilePath = state.dataStrGlobals->CurrentWorkingFolder / "Window5DataFile.dat";
            } else {
                window5DataFilePath = ConstructAlphas(1);
            }
            DisplayString(state, "Searching Window5 data file for Construction=" + ConstructAlphas(0));

            SearchWindow5DataFile(state, window5DataFilePath, ConstructAlphas(0), ConstructionFound, EOFonW5File, ErrorsFound);

            if (EOFonW5File || !ConstructionFound) {
                DisplayString(state, "--Construction not found");
                ErrorsFound = true;
                ShowSevereError(state, format("No match on WINDOW5 data file for Construction={}, or error in data file.", ConstructAlphas(0)));
                ShowContinueError(state, format("...Looking on file={}", window5DataFilePath)); // TODO: call getAbsolutePath maybe?
                continue;
            }

        } // ...end of Window5 Constructions DO loop

        WConstructNames.deallocate();

        // set some (default) properties of the Construction Derived Type
        for (int ConstrNum = 1; ConstrNum <= state.dataHeatBal->TotConstructs; ++ConstrNum) {

            auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            // For air boundaries, skip TypeIsAirBoundary
            if (thisConstruct.TypeIsAirBoundary) {
                continue;
            }
            if (state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum) != 0.0) {
                state.dataHeatBal->NominalU(ConstrNum) = 1.0 / state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum);
            } else {
                if (!thisConstruct.WindowTypeEQL) {
                    ShowSevereError(state, format("Nominal U is zero, for construction={}", thisConstruct.Name));
                    ErrorsFound = true;
                }
            }

            DataHeatBalance::CheckAndSetConstructionProperties(state, ConstrNum, ErrorsFound);

        } // End of ConstrNum DO loop
    }

    void GetBuildingData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   November 1997
        //       MODIFIED       October 1998, FW; May 1999 FW; Oct 2004 LKL

        // PURPOSE OF THIS SUBROUTINE:
        // This routine calls other routines to get the Zone, and Surface data
        //  from the input file.

        // METHODOLOGY EMPLOYED:
        // The GetObjectItem routines are employed to retrieve the data.

        SolarShading::GetShadowingInput(state);

        GetZoneData(state, ErrorsFound); // Read Zone data from input file

        SurfaceGeometry::SetupZoneGeometry(state, ErrorsFound);
    }

    void GetZoneData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   November 1997
        //       MODIFIED       PGE: Added ZONE LIST and ZONE GROUP objects, Nov 2003
        //                      RJH: Added init of DElight member of ZoneDaylight object, Jan 2004
        //                      JG: Added Part of Total Floor Area field March 2006

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the zone data for each zone in the input file.

        // METHODOLOGY EMPLOYED:
        // The GetObjectItem routines are employed to retrieve the data.

        // REFERENCES:
        // IDD Definition for Zone object

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr const char *RoutineName("GetZoneData: ");
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;
        int NumNumbers;
        int IOStatus;
        std::string::size_type TMP;
        int ZoneNum;
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "Zone";
        state.dataGlobal->NumOfZones = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        state.dataHeatBal->Zone.allocate(state.dataGlobal->NumOfZones);
        state.dataDayltg->ZoneDaylight.allocate(state.dataGlobal->NumOfZones);
        // always allocate as the data structure is needed in output variable Zone Heat Index, Zone Humidity Index
        state.dataHeatBal->Resilience.allocate(state.dataGlobal->NumOfZones);

        int ZoneLoop = 0;

        for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {

            state.dataIPShortCut->rNumericArgs = 0.0; // Zero out just in case
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Loop,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            TMP = index(state.dataIPShortCut->cAlphaArgs(1), char(1));
            while (TMP != std::string::npos) {
                state.dataIPShortCut->cAlphaArgs(1)[TMP] = ',';
                TMP = index(state.dataIPShortCut->cAlphaArgs(1), char(1));
            }
            TMP = index(state.dataIPShortCut->cAlphaArgs(1), char(2));
            while (TMP != std::string::npos) {
                state.dataIPShortCut->cAlphaArgs(1)[TMP] = '!';
                TMP = index(state.dataIPShortCut->cAlphaArgs(1), char(2));
            }

            if (Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), state.dataHeatBalMgr->CurrentModuleObject, ErrorsFound)) {
                continue;
            }

            ++ZoneLoop;
            ProcessZoneData(state,
                            cCurrentModuleObject,
                            ZoneLoop,
                            state.dataIPShortCut->cAlphaArgs,
                            NumAlphas,
                            state.dataIPShortCut->rNumericArgs,
                            NumNumbers,
                            state.dataIPShortCut->lNumericFieldBlanks,
                            state.dataIPShortCut->lAlphaFieldBlanks,
                            state.dataIPShortCut->cAlphaFieldNames,
                            state.dataIPShortCut->cNumericFieldNames,
                            ErrorsFound);

        } // Loop

        for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
            // Check to see if "nominally" controlled -- Zone Name appears in Zone Equip Configuration
            // relies on zone name being the "name" of the Zone Controlled Equip Configuration
            if (state.dataInputProcessing->inputProcessor->getObjectItemNum(
                    state, "ZoneHVAC:EquipmentConnections", "zone_name", state.dataHeatBal->Zone(Loop).Name) > 0) {
                state.dataHeatBal->Zone(Loop).isNominalControlled = true;
            } else {
                state.dataHeatBal->Zone(Loop).isNominalControlled = false;
            }
        }

        // Get ZONE LIST objects
        cCurrentModuleObject = "ZoneList";
        state.dataHeatBal->NumOfZoneLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataHeatBal->NumOfZoneLists > 0) {

            state.dataHeatBal->ZoneList.allocate(state.dataHeatBal->NumOfZoneLists);

            for (int ListNum = 1; ListNum <= state.dataHeatBal->NumOfZoneLists; ++ListNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         ListNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         state.dataIPShortCut->lAlphaFieldBlanks,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                state.dataHeatBal->ZoneList(ListNum).Name = state.dataIPShortCut->cAlphaArgs(1);
                if (Util::FindItemInList(state.dataHeatBal->ZoneList(ListNum).Name, state.dataHeatBal->Zone) > 0) {
                    ShowWarningError(
                        state,
                        format(
                            "{}{}=\"{}\":  is a duplicate of a zone name.", RoutineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(state, "This could be a problem in places where either a Zone Name or a Zone List can be used.");
                }

                // List of zones
                state.dataHeatBal->ZoneList(ListNum).NumOfZones = NumAlphas - 1;

                if (state.dataHeatBal->ZoneList(ListNum).NumOfZones < 1) {
                    ShowSevereError(
                        state, format("{}{}=\"{}\":  No zones specified.", RoutineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ErrorsFound = true;
                } else {
                    state.dataHeatBal->ZoneList(ListNum).Zone.allocate(state.dataHeatBal->ZoneList(ListNum).NumOfZones);
                    state.dataHeatBal->ZoneList(ListNum).Zone = 0;

                    for (ZoneNum = 1; ZoneNum <= state.dataHeatBal->ZoneList(ListNum).NumOfZones; ++ZoneNum) {
                        std::string ZoneName = state.dataIPShortCut->cAlphaArgs(ZoneNum + 1);
                        state.dataHeatBal->ZoneList(ListNum).MaxZoneNameLength =
                            max(state.dataHeatBal->ZoneList(ListNum).MaxZoneNameLength, len(ZoneName));
                        state.dataHeatBal->ZoneList(ListNum).Zone(ZoneNum) = Util::FindItemInList(ZoneName, state.dataHeatBal->Zone);
                        if (state.dataHeatBal->ZoneList(ListNum).Zone(ZoneNum) == 0) {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\":  {} {} not found.",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   state.dataIPShortCut->cAlphaFieldNames(ZoneNum + 1),
                                                   ZoneName));
                            ErrorsFound = true;
                        }

                        // Check for duplicate zones
                        for (int Loop = 1; Loop <= ZoneNum - 1; ++Loop) {
                            if (state.dataHeatBal->ZoneList(ListNum).Zone(ZoneNum) == state.dataHeatBal->ZoneList(ListNum).Zone(Loop)) {
                                ShowSevereError(state,
                                                format("{}{}=\"{}\":  {} {} appears more than once in list.",
                                                       RoutineName,
                                                       cCurrentModuleObject,
                                                       state.dataIPShortCut->cAlphaArgs(1),
                                                       state.dataIPShortCut->cAlphaFieldNames(ZoneNum + 1),
                                                       ZoneName));
                                ErrorsFound = true;
                            }
                        } // Loop
                    } // ZoneNum
                }

            } // ListNum
        }

        // Get ZONE GROUP objects
        cCurrentModuleObject = "ZoneGroup";
        state.dataHeatBal->NumOfZoneGroups = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataHeatBal->NumOfZoneGroups > 0) {
            state.dataHeatBal->ZoneGroup.allocate(state.dataHeatBal->NumOfZoneGroups);

            for (int GroupNum = 1; GroupNum <= state.dataHeatBal->NumOfZoneGroups; ++GroupNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         GroupNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         state.dataIPShortCut->lAlphaFieldBlanks,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                state.dataHeatBal->ZoneGroup(GroupNum).Name = state.dataIPShortCut->cAlphaArgs(1);

                // Multiplier - checked already by IDD rules
                state.dataHeatBal->ZoneGroup(GroupNum).Multiplier = state.dataIPShortCut->rNumericArgs(1);

                // Zone list
                int ListNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataHeatBal->ZoneList);
                state.dataHeatBal->ZoneGroup(GroupNum).ZoneList = ListNum;

                if (ListNum == 0) {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\":  {} named {} not found.",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaFieldNames(2),
                                           state.dataIPShortCut->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    // Check to make sure list is not in use by another ZONE GROUP
                    for (int Loop = 1; Loop <= GroupNum - 1; ++Loop) {
                        if (state.dataHeatBal->ZoneGroup(GroupNum).ZoneList == state.dataHeatBal->ZoneGroup(Loop).ZoneList) {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\":  {} already used by {} named {}.",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                                   cCurrentModuleObject,
                                                   state.dataHeatBal->ZoneGroup(Loop).Name));
                            ErrorsFound = true;
                        }
                    } // Loop

                    // Set group multiplier for each zone in the list
                    for (int Loop = 1; Loop <= state.dataHeatBal->ZoneList(ListNum).NumOfZones; ++Loop) {
                        ZoneNum = state.dataHeatBal->ZoneList(ListNum).Zone(Loop);

                        if (ZoneNum > 0) {
                            // Check to make sure group multiplier was not already set by another ZONE GROUP
                            if (state.dataHeatBal->Zone(ZoneNum).ListGroup == 0) {
                                state.dataHeatBal->Zone(ZoneNum).ListMultiplier = state.dataHeatBal->ZoneGroup(GroupNum).Multiplier;
                                state.dataHeatBal->Zone(ZoneNum).ListGroup = ListNum;
                            } else {
                                ShowSevereError(state,
                                                format("{}{}=\"{}\":  Zone {} in ZoneList already exists in ZoneList of another ZoneGroup.",
                                                       RoutineName,
                                                       cCurrentModuleObject,
                                                       state.dataIPShortCut->cAlphaArgs(1),
                                                       state.dataHeatBal->Zone(ZoneNum).Name));
                                ShowContinueError(
                                    state,
                                    format("Previous ZoneList={}", state.dataHeatBal->ZoneList(state.dataHeatBal->Zone(ZoneNum).ListGroup).Name));
                                ErrorsFound = true;
                            }
                        }
                    } // Loop
                }

            } // GroupNum
        }

        GetZoneLocalEnvData(state, ErrorsFound);

        // allocate the array the holds the predefined report data
        state.dataHeatBal->ZonePreDefRep.allocate(state.dataGlobal->NumOfZones);

        // Now get Space data after Zones are set up, because Space is optional, Zones are not
        GetSpaceData(state, ErrorsFound);
    }

    void GetIncidentSolarMultiplier(EnergyPlusData &state, bool &ErrorsFound)
    {
        static constexpr std::string_view RoutineName("GetIncidentSolarMultiplier: ");
        static constexpr std::string_view routineName = "GetIncidentSolarMultiplier";

        auto &s_mat = state.dataMaterial;
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "SurfaceProperty:IncidentSolarMultiplier";

        state.dataSurface->TotSurfIncSolMultiplier = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataSurface->TotSurfIncSolMultiplier <= 0) {
            return;
        }

        if (!allocated(state.dataSurface->SurfIncSolMultiplier)) {
            // could be extended to interior surfaces later
            state.dataSurface->SurfIncSolMultiplier.allocate(state.dataSurface->TotSurfaces);
        }

        int NumAlpha;
        int NumNumeric;
        int IOStat;
        for (int Loop = 1; Loop <= state.dataSurface->TotSurfIncSolMultiplier; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Loop,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlpha,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumeric,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

            // Assign surface number
            int SurfNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(1), state.dataSurface->Surface);
            if (SurfNum == 0) {
                ShowSevereError(state,
                                format("{}{}=\"{}, object. Illegal value for {} has been found.",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       state.dataIPShortCut->cAlphaArgs(1),
                                       state.dataIPShortCut->cAlphaFieldNames(1)));
                ShowContinueError(
                    state,
                    format("{} entered value = \"{}\" no corresponding surface (ref BuildingSurface:Detailed) has been found in the input file.",
                           state.dataIPShortCut->cAlphaFieldNames(1),
                           state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
                continue;
            }
            auto &Surf = state.dataSurface->Surface(SurfNum);
            if (Surf.Class != DataSurfaces::SurfaceClass::Window) {
                ShowSevereError(state, "IncidentSolarMultiplier defined for non-window surfaces");
                ErrorsFound = true;
                continue;
            }
            if (Surf.ExtBoundCond != DataSurfaces::ExternalEnvironment) {
                ShowSevereError(state, "IncidentSolarMultiplier defined for interior surfaces");
                ErrorsFound = true;
                continue;
            }
            int ConstrNum = Surf.Construction;
            auto const &Constr = state.dataConstruction->Construct(ConstrNum);
            auto const *mat = s_mat->materials(Constr.LayerPoint(Constr.TotLayers));
            bool withNoncompatibleShades =
                (mat->group == Material::Group::Shade || mat->group == Material::Group::Blind || mat->group == Material::Group::Screen ||
                 mat->group == Material::Group::GlassEQL || mat->group == Material::Group::WindowGapEQL || mat->group == Material::Group::ShadeEQL ||
                 mat->group == Material::Group::DrapeEQL || mat->group == Material::Group::ScreenEQL || mat->group == Material::Group::BlindEQL ||
                 Surf.HasShadeControl);
            if (withNoncompatibleShades) {
                ShowSevereError(state, "Non-compatible shades defined alongside SurfaceProperty:IncidentSolarMultiplier for the same window");
                ErrorsFound = true;
                continue;
            }

            Surf.hasIncSolMultiplier = true;
            auto &SurfIncSolMult = state.dataSurface->SurfIncSolMultiplier(SurfNum);
            SurfIncSolMult.Name = state.dataIPShortCut->cAlphaArgs(1);
            SurfIncSolMult.SurfaceIdx = SurfNum;
            SurfIncSolMult.Scaler = state.dataIPShortCut->rNumericArgs(1);

            if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            } else if ((SurfIncSolMult.sched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            }
        } // for (Loop)
    } // GetIncidentSolarMultiplier()

    void GetZoneLocalEnvData(EnergyPlusData &state, bool &ErrorsFound) // Error flag indicator (true if errors found)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         X LUO
        //       DATE WRITTEN   July 2017

        // PURPOSE OF THIS SUBROUTINE:
        // load input data for Outdoor Air Node for zones

        // Using/Aliasing
        using DataLoopNode::ObjectIsParent;
        using NodeInputManager::GetOnlySingleNode;
        using OutAirNodeManager::CheckOutAirNodeNumber;

        //-----------------------------------------------------------------------
        //               ZoneProperty:LocalEnvironment
        //-----------------------------------------------------------------------
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "ZoneProperty:LocalEnvironment";
        int TotZoneEnv = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (TotZoneEnv > 0) {
            int NumAlpha;
            int NumNumeric;
            int IOStat;
            constexpr const char *RoutineName("GetZoneLocalEnvData: ");
            // Check if IDD definition is correct
            state.dataGlobal->AnyLocalEnvironmentsInModel = true;

            if (!allocated(state.dataHeatBal->ZoneLocalEnvironment)) {
                state.dataHeatBal->ZoneLocalEnvironment.allocate(TotZoneEnv);
            }

            for (int Loop = 1; Loop <= TotZoneEnv; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         Loop,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlpha,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumeric,
                                                                         IOStat,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         state.dataIPShortCut->lAlphaFieldBlanks,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                state.dataHeatBal->ZoneLocalEnvironment(Loop).Name = state.dataIPShortCut->cAlphaArgs(1);

                // Assign zone number
                int ZoneNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataHeatBal->Zone);
                if (ZoneNum == 0) {
                    ShowSevereError(state,
                                    format("{}{}=\"{}, object. Illegal value for {} has been found.",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaFieldNames(2)));
                    ShowContinueError(state,
                                      format("{} entered value = \"{}\" no corresponding zone has been found in the input file.",
                                             state.dataIPShortCut->cAlphaFieldNames(2),
                                             state.dataIPShortCut->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    state.dataHeatBal->ZoneLocalEnvironment(Loop).ZonePtr = ZoneNum;
                }

                // Assign outdoor air node number;
                int NodeNum = GetOnlySingleNode(state,
                                                state.dataIPShortCut->cAlphaArgs(3),
                                                ErrorsFound,
                                                DataLoopNode::ConnectionObjectType::ZonePropertyLocalEnvironment,
                                                state.dataIPShortCut->cAlphaArgs(1),
                                                DataLoopNode::NodeFluidType::Air,
                                                DataLoopNode::ConnectionType::Inlet,
                                                NodeInputManager::CompFluidStream::Primary,
                                                ObjectIsParent);
                if (NodeNum == 0 && CheckOutAirNodeNumber(state, NodeNum)) {
                    ShowSevereError(state,
                                    format("{}{}=\"{}, object. Illegal value for {} has been found.",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaFieldNames(3)));
                    ShowContinueError(state,
                                      format("{} entered value = \"{}\" no corresponding schedule has been found in the input file.",
                                             state.dataIPShortCut->cAlphaFieldNames(3),
                                             state.dataIPShortCut->cAlphaArgs(3)));
                    ErrorsFound = true;
                } else {
                    state.dataHeatBal->ZoneLocalEnvironment(Loop).OutdoorAirNodePtr = NodeNum;
                }
            }
        }
        // Link zone properties to zone object
        for (int ZoneLoop = 1; ZoneLoop <= state.dataGlobal->NumOfZones; ++ZoneLoop) {
            for (int Loop = 1; Loop <= TotZoneEnv; ++Loop) {
                if (state.dataHeatBal->ZoneLocalEnvironment(Loop).ZonePtr == ZoneLoop) {
                    if (state.dataHeatBal->ZoneLocalEnvironment(Loop).OutdoorAirNodePtr != 0) {
                        state.dataHeatBal->Zone(ZoneLoop).LinkedOutAirNode = state.dataHeatBal->ZoneLocalEnvironment(Loop).OutdoorAirNodePtr;
                    }
                }
            }
        }
    }

    void ProcessZoneData(EnergyPlusData &state,
                         std::string const &cCurrentModuleObject,
                         int const ZoneLoop,
                         Array1D_string const &cAlphaArgs,
                         int const NumAlphas,
                         Array1D<Real64> const &rNumericArgs,
                         int const NumNumbers,
                         [[maybe_unused]] Array1D_bool const &lNumericFieldBlanks, // Unused
                         Array1D_bool const &lAlphaFieldBlanks,
                         Array1D_string const &cAlphaFieldNames,
                         [[maybe_unused]] Array1D_string const &cNumericFieldNames, // Unused
                         bool &ErrorsFound                                          // If errors found in input
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   November 1997
        //       MODIFIED       PGE: Added ZONE LIST and ZONE GROUP objects, Nov 2003
        //                      RJH: Added init of DElight member of ZoneDaylight object, Jan 2004
        //                      JG: Added Part of Total Floor Area field March 2006
        //       RE-ENGINEERED  MJW: Split out processing zone input to facilitate unit testing, Nov 2014

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the zone data for each zone in the input file.

        constexpr const char *RoutineName("ProcessZoneData: ");

        state.dataHeatBal->Zone(ZoneLoop).Name = cAlphaArgs(1);
        if (NumNumbers >= 1) {
            state.dataHeatBal->Zone(ZoneLoop).RelNorth = rNumericArgs(1);
        }
        if (NumNumbers >= 2) {
            state.dataHeatBal->Zone(ZoneLoop).OriginX = rNumericArgs(2);
        }
        if (NumNumbers >= 3) {
            state.dataHeatBal->Zone(ZoneLoop).OriginY = rNumericArgs(3);
        }
        if (NumNumbers >= 4) {
            state.dataHeatBal->Zone(ZoneLoop).OriginZ = rNumericArgs(4);
        }
        if (NumNumbers >= 5) {
            state.dataHeatBal->Zone(ZoneLoop).OfType = rNumericArgs(5);
        }
        state.dataHeatBal->Zone(ZoneLoop).OfType = DataHeatBalance::StandardZone;
        if (NumNumbers >= 6) {
            state.dataHeatBal->Zone(ZoneLoop).Multiplier = rNumericArgs(6);
        }
        if (NumNumbers >= 7) {
            state.dataHeatBal->Zone(ZoneLoop).CeilingHeight = rNumericArgs(7);
        }
        if (NumNumbers >= 8) {
            state.dataHeatBal->Zone(ZoneLoop).Volume = rNumericArgs(8);
        }
        if (NumNumbers >= 9) {
            state.dataHeatBal->Zone(ZoneLoop).UserEnteredFloorArea = rNumericArgs(9);
        }

        if (NumAlphas > 1 && !state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            Convect::HcInt hcIn = static_cast<Convect::HcInt>(getEnumValue(Convect::HcIntNamesUC, cAlphaArgs(2)));

            if (hcIn != Convect::HcInt::ASHRAESimple && hcIn != Convect::HcInt::ASHRAETARP && hcIn != Convect::HcInt::CeilingDiffuser &&
                hcIn != Convect::HcInt::TrombeWall && hcIn != Convect::HcInt::AdaptiveConvectionAlgorithm && hcIn != Convect::HcInt::ASTMC1340) {

                ShowSevereError(state, format("{}{}=\"{}\".", RoutineName, cCurrentModuleObject, state.dataHeatBal->Zone(ZoneLoop).Name));
                ShowContinueError(state, format("Invalid value for {}=\"{}\".", cAlphaFieldNames(2), cAlphaArgs(2)));
                ErrorsFound = true;
            }
            state.dataHeatBal->Zone(ZoneLoop).IntConvAlgo = hcIn;
        } else {
            // No zone specific algorithm specified, use default Inside Convection Algorithm
            state.dataHeatBal->Zone(ZoneLoop).IntConvAlgo = state.dataHeatBal->DefaultIntConvAlgo;
        }

        if (NumAlphas > 2 && !state.dataIPShortCut->lAlphaFieldBlanks(3)) {

            Convect::HcExt hcOut = static_cast<Convect::HcExt>(getEnumValue(Convect::HcExtNamesUC, cAlphaArgs(3)));

            if (hcOut != Convect::HcExt::ASHRAESimple && hcOut != Convect::HcExt::ASHRAETARP && hcOut != Convect::HcExt::MoWiTTHcOutside &&
                hcOut != Convect::HcExt::DOE2HcOutside && hcOut != Convect::HcExt::AdaptiveConvectionAlgorithm) {

                ShowSevereError(state, format("{}{}=\"{}\".", RoutineName, cCurrentModuleObject, state.dataHeatBal->Zone(ZoneLoop).Name));
                ShowContinueError(state, format("Invalid value for {}=\"{}\".", cAlphaFieldNames(3), cAlphaArgs(3)));
                ErrorsFound = true;
            }
            state.dataHeatBal->Zone(ZoneLoop).ExtConvAlgo = hcOut;

        } else {
            // No zone specific algorithm specified, use default Outside Convection Algorithm
            state.dataHeatBal->Zone(ZoneLoop).ExtConvAlgo = state.dataHeatBal->DefaultExtConvAlgo;
        }

        // Process the input field:    Part of Total Floor Area
        //   The default value is YES and so only NO needs to be handled
        if (NumAlphas > 3) {
            if (Util::SameString("No", cAlphaArgs(4))) {
                state.dataHeatBal->Zone(ZoneLoop).isPartOfTotalArea = false;
            } else if (Util::SameString("Yes", cAlphaArgs(4)) || lAlphaFieldBlanks(4)) {
                state.dataHeatBal->Zone(ZoneLoop).isPartOfTotalArea = true;
            } else {
                ShowSevereError(state, format("{}{}=\"{}\".", RoutineName, cCurrentModuleObject, state.dataHeatBal->Zone(ZoneLoop).Name));
                ShowContinueError(state, format("Invalid value for {}=\"{}\".", cAlphaFieldNames(4), cAlphaArgs(4)));
                ErrorsFound = true;
            }
        }

        // Zone outdoor environmental variables, used for zone infiltration/ventilation
        SetupOutputVariable(state,
                            "Zone Outdoor Air Drybulb Temperature",
                            Constant::Units::C,
                            state.dataHeatBal->Zone(ZoneLoop).OutDryBulbTemp,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            state.dataHeatBal->Zone(ZoneLoop).Name);
        SetupOutputVariable(state,
                            "Zone Outdoor Air Wetbulb Temperature",
                            Constant::Units::C,
                            state.dataHeatBal->Zone(ZoneLoop).OutWetBulbTemp,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            state.dataHeatBal->Zone(ZoneLoop).Name);
        SetupOutputVariable(state,
                            "Zone Outdoor Air Wind Speed",
                            Constant::Units::m_s,
                            state.dataHeatBal->Zone(ZoneLoop).WindSpeed,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            state.dataHeatBal->Zone(ZoneLoop).Name);
        SetupOutputVariable(state,
                            "Zone Outdoor Air Wind Direction",
                            Constant::Units::deg,
                            state.dataHeatBal->Zone(ZoneLoop).WindDir,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            state.dataHeatBal->Zone(ZoneLoop).Name);
    }

    void GetSpaceData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {
        constexpr const char *RoutineName("GetSpaceData: ");
        std::string cCurrentModuleObject = "Space";
        auto &ip = state.dataInputProcessing->inputProcessor;
        auto const instances = ip->epJSON.find(cCurrentModuleObject);
        if (instances != ip->epJSON.end()) {
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
            auto &instancesValue = instances.value();
            int numSpaces = instancesValue.size();
            int spaceNum = 0;
            // Allow for one additional Space per zone if some surfaces do not have a Space assigned in input
            state.dataHeatBal->space.allocate(size_t(numSpaces + state.dataGlobal->NumOfZones));
            // Allow for one additional "General" space type for auto-generated spaces
            state.dataHeatBal->spaceTypes.allocate(size_t(numSpaces + 1));

            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                ++spaceNum;
                auto const &objectFields = instance.value();
                auto &thisSpace = state.dataHeatBal->space(spaceNum);
                thisSpace.Name = Util::makeUPPER(instance.key());
                ip->markObjectAsUsed(cCurrentModuleObject, instance.key());
                std::string zoneName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "zone_name");
                thisSpace.CeilingHeight = ip->getRealFieldValue(objectFields, objectSchemaProps, "ceiling_height");
                thisSpace.Volume = ip->getRealFieldValue(objectFields, objectSchemaProps, "volume");
                thisSpace.userEnteredFloorArea = ip->getRealFieldValue(objectFields, objectSchemaProps, "floor_area");
                int zoneNum = Util::FindItemInList(zoneName, state.dataHeatBal->Zone);
                if (zoneNum > 0) {
                    thisSpace.zoneNum = zoneNum;
                    state.dataHeatBal->Zone(zoneNum).spaceIndexes.emplace_back(spaceNum);
                    ++state.dataHeatBal->Zone(zoneNum).numSpaces;
                } else {
                    ShowSevereError(state, format("{}{}={}", RoutineName, cCurrentModuleObject, thisSpace.Name));
                    ShowContinueError(state, format("Zone Name ={}not found.", zoneName));
                    ErrorsFound = true;
                }
                thisSpace.spaceType = ip->getAlphaFieldValue(objectFields, objectSchemaProps, "space_type");
                bool spaceTypeFound = false;
                for (int spaceTypePtr = 1; spaceTypePtr <= state.dataGlobal->numSpaceTypes; ++spaceTypePtr) {
                    if (Util::SameString(thisSpace.spaceType, state.dataHeatBal->spaceTypes(spaceTypePtr))) {
                        thisSpace.spaceTypeNum = spaceTypePtr;
                        spaceTypeFound = true;
                        break;
                    }
                }
                if (!spaceTypeFound) {
                    ++state.dataGlobal->numSpaceTypes;
                    state.dataHeatBal->spaceTypes(state.dataGlobal->numSpaceTypes) = thisSpace.spaceType;
                    thisSpace.spaceTypeNum = state.dataGlobal->numSpaceTypes;
                }

                auto extensibles = objectFields.find("tags");
                auto const &extensionSchemaProps = objectSchemaProps["tags"]["items"]["properties"];
                if (extensibles != objectFields.end()) {
                    auto &extensiblesArray = extensibles.value();
                    for (auto &extensibleInstance : extensiblesArray) {
                        thisSpace.tags.emplace_back(ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "tag"));
                    }
                }
            }
            state.dataGlobal->numSpaces = spaceNum;
        } else {
            // If no Spaces are defined, then allow for one Space per zone, and one spaceType
            state.dataHeatBal->space.allocate(state.dataGlobal->NumOfZones);
            state.dataHeatBal->spaceTypes.allocate(1);
        }

        cCurrentModuleObject = "SpaceList";
        auto const instances2 = ip->epJSON.find(cCurrentModuleObject);
        if (instances2 != ip->epJSON.end()) {
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
            auto &instancesValue = instances2.value();
            int numSpaceLists = instancesValue.size();
            int spaceListNum = 0;
            state.dataHeatBal->spaceList.allocate(numSpaceLists);
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                ++spaceListNum;
                auto const &objectFields = instance.value();
                auto &thisSpaceList = state.dataHeatBal->spaceList(spaceListNum);
                thisSpaceList.Name = Util::makeUPPER(instance.key());
                ip->markObjectAsUsed(cCurrentModuleObject, instance.key());

                if (Util::FindItemInList(thisSpaceList.Name, state.dataHeatBal->Zone) > 0) {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\":  is a duplicate of a zone name.", RoutineName, cCurrentModuleObject, thisSpaceList.Name));
                    ErrorsFound = true;
                }
                if (Util::FindItemInList(thisSpaceList.Name, state.dataHeatBal->space) > 0) {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\":  is a duplicate of a space name.", RoutineName, cCurrentModuleObject, thisSpaceList.Name));
                    ErrorsFound = true;
                }

                // List of spaces
                thisSpaceList.numListSpaces = 0;
                auto extensibles = objectFields.find("spaces");
                auto const &extensionSchemaProps = objectSchemaProps["spaces"]["items"]["properties"];
                if (extensibles != objectFields.end()) {
                    auto &extensiblesArray = extensibles.value();
                    for (auto &extensibleInstance : extensiblesArray) {
                        std::string thisSpaceName = ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "space_name");
                        int thisSpaceNum = Util::FindItemInList(thisSpaceName, state.dataHeatBal->space);
                        if (thisSpaceNum > 0) {
                            thisSpaceList.spaces.emplace_back(thisSpaceNum);
                            ++thisSpaceList.numListSpaces;
                        } else {
                            ShowSevereError(state, format("{}{}={}", RoutineName, cCurrentModuleObject, thisSpaceList.Name));
                            ShowContinueError(state, format("Space Name={} not found.", thisSpaceName));
                            ErrorsFound = true;
                        }
                        thisSpaceList.maxSpaceNameLength = max(thisSpaceList.maxSpaceNameLength, len(thisSpaceName));
                        // Check for duplicate spaces
                        for (int loop = 1; loop <= int(thisSpaceList.spaces.size()) - 1; ++loop) {
                            if (thisSpaceNum == thisSpaceList.spaces(loop)) {
                                ShowSevereError(state,
                                                format("{}{}=\"{}\":  Space Name {} appears more than once in list.",
                                                       RoutineName,
                                                       cCurrentModuleObject,
                                                       thisSpaceList.Name,
                                                       thisSpaceName));
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            }
        }

        // Make sure every zone has at least one space
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            auto &thisZone = state.dataHeatBal->Zone(zoneNum);
            if (thisZone.spaceIndexes.empty()) {
                ++state.dataGlobal->numSpaces;
                state.dataHeatBal->space(state.dataGlobal->numSpaces).zoneNum = zoneNum;
                state.dataHeatBal->space(state.dataGlobal->numSpaces).Name = thisZone.Name;
                state.dataHeatBal->space(state.dataGlobal->numSpaces).spaceType = "GENERAL";
                state.dataHeatBal->space(state.dataGlobal->numSpaces).spaceTypeNum = GetGeneralSpaceTypeNum(state);
                // Add to zone's list of spaces
                thisZone.spaceIndexes.emplace_back(state.dataGlobal->numSpaces);
                ++state.dataHeatBal->Zone(zoneNum).numSpaces;
            }
        }
    }

    int GetGeneralSpaceTypeNum(EnergyPlusData &state)
    {
        // If "General" exists as a space type return the index
        bool generalSpaceTypeExists = false;
        int generalSpaceTypeNum = 0;
        for (int spaceTypePtr = 1; spaceTypePtr <= state.dataGlobal->numSpaceTypes; ++spaceTypePtr) {
            if (Util::SameString(state.dataHeatBal->spaceTypes(spaceTypePtr), "GENERAL")) {
                generalSpaceTypeNum = spaceTypePtr;
                generalSpaceTypeExists = true;
                break;
            }
        }
        // Add General space type if it doesn't exist yet
        if (!generalSpaceTypeExists) {
            ++state.dataGlobal->numSpaceTypes;
            state.dataHeatBal->spaceTypes(state.dataGlobal->numSpaceTypes) = "GENERAL";
            generalSpaceTypeNum = state.dataGlobal->numSpaceTypes;
        }
        return generalSpaceTypeNum;
    }
    // End of Get Input subroutines for the HB Module
    //******************************************************************************

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitHeatBalance(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   April 1997

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main driver for initializations within the
        // heat balance.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initialization events.  Some of the files
        //  have been moved to other heat balance managers.  More of these initializations
        //  will have to continue to be re-structured.

        // Using/Aliasing
        using namespace Window;
        using namespace SolarShading;
        using Dayltg::InitDaylightingDevices;
        using OutAirNodeManager::SetOutAirNodes;
        using WindowEquivalentLayer::InitEquivalentLayerWindowCalculations;

        if (state.dataGlobal->BeginSimFlag) {
            AllocateHeatBalArrays(state); // Allocate the Module Arrays
            if (state.dataHeatBal->AnyCTF || state.dataHeatBal->AnyEMPD) {
                DisplayString(state, "Initializing Response Factors");
                InitConductionTransferFunctions(state); // Initialize the response factors
            }
            HeatBalanceSurfaceManager::InitSurfacePropertyViewFactors(state);
            DisplayString(state, "Initializing Window Optical Properties");
            InitEquivalentLayerWindowCalculations(state); // Initialize the EQL window optical properties
            // InitGlassOpticalCalculations(); // Initialize the window optical properties
            Window::InitWindowOpticalCalculations(state);
            InitDaylightingDevices(state); // Initialize any daylighting devices
            DisplayString(state, "Initializing Solar Calculations");
            InitSolarCalculations(state); // Initialize the shadowing calculations
        }

        if (state.dataGlobal->BeginEnvrnFlag) {
            state.dataHeatBalMgr->MaxHeatLoadPrevDay = 0.0;
            state.dataHeatBalMgr->MaxCoolLoadPrevDay = 0.0;
            state.dataHeatBalMgr->MaxTempPrevDay = 0.0;
            state.dataHeatBalMgr->MinTempPrevDay = 0.0;
            state.dataHeatBalMgr->MaxHeatLoadZone = -9999.0;
            state.dataHeatBalMgr->MaxCoolLoadZone = -9999.0;
            state.dataHeatBalMgr->MaxTempZone = -9999.0;
            state.dataHeatBalMgr->MinTempZone = 1000.0;
            state.dataHeatBalMgr->TempZone = -9999.0;
            state.dataHeatBalMgr->LoadZone = -9999.0;
            state.dataHeatBalMgr->TempZonePrevDay = 1000.0;
            state.dataHeatBalMgr->LoadZonePrevDay = -9999.0;
            state.dataHeatBalMgr->TempZoneSecPrevDay = -9999.0;
            state.dataHeatBalMgr->WarmupTempDiff = 0.0;
            state.dataHeatBalMgr->WarmupLoadDiff = 0.0;
            state.dataHeatBalMgr->TempZoneRpt = 0.0;
            state.dataHeatBalMgr->LoadZoneRpt = 0.0;
            state.dataHeatBalMgr->MaxLoadZoneRpt = 0.0;
            state.dataHeatBalMgr->CountWarmupDayPoints = 0;

            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
                std::fill(
                    state.dataSurface->SurfaceWindow(SurfNum).thetaFace.begin(), state.dataSurface->SurfaceWindow(SurfNum).thetaFace.end(), 296.15);
                state.dataSurface->SurfWinEffInsSurfTemp(SurfNum) = 23.0;
            }
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            HeatBalanceSurfaceManager::InitEMSControlledConstructions(state);
            HeatBalanceSurfaceManager::InitEMSControlledSurfaceProperties(state);
        }

        // Init storm window pointers
        if (state.dataSurface->TotStormWin > 0) {
            if (state.dataGlobal->BeginDayFlag) {
                SetStormWindowControl(state);
                state.dataHeatBalMgr->ChangeSet = false;
            } else if (!state.dataHeatBalMgr->ChangeSet) {
                state.dataHeatBal->StormWinChangeThisDay = false;
                for (int StormWinNum = 1; StormWinNum <= state.dataSurface->TotStormWin; ++StormWinNum) {
                    int SurfNum = state.dataSurface->StormWindow(StormWinNum).BaseWindowNum;
                    state.dataSurface->SurfWinStormWinFlagPrevDay(SurfNum) = state.dataSurface->SurfWinStormWinFlag(SurfNum);
                }
                state.dataHeatBalMgr->ChangeSet = true;
            }
            for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
                for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    int const firstSurfWin = thisSpace.WindowSurfaceFirst;
                    int const lastSurfWin = thisSpace.WindowSurfaceLast;
                    for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                        if (state.dataSurface->SurfWinStormWinFlag(SurfNum) == 1 &&
                            state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::Detailed) {
                            state.dataSurface->SurfActiveConstruction(SurfNum) = state.dataSurface->SurfWinStormWinConstr(SurfNum);
                        } else {
                            state.dataSurface->SurfActiveConstruction(SurfNum) = state.dataSurface->Surface(SurfNum).Construction;
                        }
                    }
                }
            }
        }

        if (state.dataGlobal->BeginSimFlag && state.dataGlobal->DoWeathSim && state.dataSysVars->ReportExtShadingSunlitFrac) {
            OpenShadingFile(state);
        }

        if (state.dataGlobal->BeginDayFlag) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataGlobal->DayOfSim == 1) {
                    state.dataHeatBalMgr->MaxHeatLoadZone = -9999.0;
                    state.dataHeatBalMgr->MaxCoolLoadZone = -9999.0;
                    state.dataHeatBalMgr->MaxTempZone = -9999.0;
                    state.dataHeatBalMgr->MinTempZone = 1000.0;
                }
            }
            if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
                PerformSolarCalculations(state);
            }
        }

        if (state.dataSysVars->DetailedSolarTimestepIntegration) { // always redo solar calcs
            PerformSolarCalculations(state);
        }

        if (state.dataGlobal->BeginDayFlag && !state.dataGlobal->WarmupFlag && state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodWeather &&
            state.dataSysVars->ReportExtShadingSunlitFrac) {
            for (int iHour = 1; iHour <= Constant::iHoursInDay; ++iHour) { // Do for all hours.
                for (int TS = 1; TS <= state.dataGlobal->TimeStepsInHour; ++TS) {
                    constexpr const char *ShdFracFmt1(" {:02}/{:02} {:02}:{:02},");
                    if (TS == state.dataGlobal->TimeStepsInHour) {
                        print(state.files.shade, ShdFracFmt1, state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, iHour, 0);
                    } else {
                        print(state.files.shade,
                              ShdFracFmt1,
                              state.dataEnvrn->Month,
                              state.dataEnvrn->DayOfMonth,
                              iHour - 1,
                              (60 / state.dataGlobal->TimeStepsInHour) * TS);
                    }
                    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        constexpr const char *ShdFracFmt2("{:10.8F},");
                        print(state.files.shade, ShdFracFmt2, state.dataHeatBal->SurfSunlitFrac(iHour, TS, SurfNum));
                    }
                    print(state.files.shade, "\n");
                }
            }
        }

        // Initialize zone outdoor environmental variables
        // Bulk Initialization for Temperatures & WindSpeed
        // using the zone, modify the zone  Dry/Wet BulbTemps
        DataHeatBalance::SetZoneOutBulbTempAt(state);
        DataHeatBalance::CheckZoneOutBulbTempAt(state);

        // set zone level wind dir to global value
        DataHeatBalance::SetZoneWindSpeedAt(state);
        DataHeatBalance::SetZoneWindDirAt(state);

        // Set zone data to linked air node value if defined.
        if (state.dataGlobal->AnyLocalEnvironmentsInModel) {
            SetOutAirNodes(state);
            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                if (state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode > 0) {
                    if (state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).outAirDryBulbSched != nullptr) {
                        state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp =
                            state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).outAirDryBulbSched->getCurrentVal();
                    } else {
                        state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp =
                            state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).OutAirDryBulb;
                    }
                    if (state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).outAirWetBulbSched != nullptr) {
                        state.dataHeatBal->Zone(ZoneNum).OutWetBulbTemp =
                            state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).outAirWetBulbSched->getCurrentVal();
                    } else {
                        state.dataHeatBal->Zone(ZoneNum).OutWetBulbTemp =
                            state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).OutAirWetBulb;
                    }
                    if (state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).outAirWindSpeedSched != nullptr) {
                        state.dataHeatBal->Zone(ZoneNum).WindSpeed =
                            state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).outAirWindSpeedSched->getCurrentVal();
                    } else {
                        state.dataHeatBal->Zone(ZoneNum).WindSpeed =
                            state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).OutAirWindSpeed;
                    }
                    if (state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).outAirWindDirSched != nullptr) {
                        state.dataHeatBal->Zone(ZoneNum).WindDir =
                            state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).outAirWindDirSched->getCurrentVal();
                    } else {
                        state.dataHeatBal->Zone(ZoneNum).WindDir =
                            state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).LinkedOutAirNode).OutAirWindDir;
                    }
                }
            }
        }

        // Overwriting surface and zone level environmental data with EMS override value
        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                if (state.dataHeatBal->Zone(ZoneNum).OutDryBulbTempEMSOverrideOn) {
                    state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp = state.dataHeatBal->Zone(ZoneNum).OutDryBulbTempEMSOverrideValue;
                }
                if (state.dataHeatBal->Zone(ZoneNum).OutWetBulbTempEMSOverrideOn) {
                    state.dataHeatBal->Zone(ZoneNum).OutWetBulbTemp = state.dataHeatBal->Zone(ZoneNum).OutWetBulbTempEMSOverrideValue;
                }
                if (state.dataHeatBal->Zone(ZoneNum).WindSpeedEMSOverrideOn) {
                    state.dataHeatBal->Zone(ZoneNum).WindSpeed = state.dataHeatBal->Zone(ZoneNum).WindSpeedEMSOverrideValue;
                }
                if (state.dataHeatBal->Zone(ZoneNum).WindDirEMSOverrideOn) {
                    state.dataHeatBal->Zone(ZoneNum).WindDir = state.dataHeatBal->Zone(ZoneNum).WindDirEMSOverrideValue;
                }
            }
        }

        if (state.dataGlobal->BeginSimFlag) {
            for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
                for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    int const firstSurfWin = thisSpace.WindowSurfaceFirst;
                    int const lastSurfWin = thisSpace.WindowSurfaceLast;
                    for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                        if (state.dataSurface->SurfWinWindowModelType(SurfNum) != DataSurfaces::WindowModel::BSDF &&
                            state.dataSurface->SurfWinWindowModelType(SurfNum) != DataSurfaces::WindowModel::EQL) {
                            state.dataSurface->SurfWinWindowModelType(SurfNum) = DataSurfaces::WindowModel::Detailed;
                        }
                    }
                }
            }
        }
    }

    void AllocateZoneHeatBalArrays(EnergyPlusData &state)
    {
        // Allocate zone / encl hb arrays

        // TODO MJW: Punt for now, sometimes unit test will get here and need these to be allocated, but simulations need them sooner
        if (!state.dataHeatBal->ZoneIntGain.allocated()) {
            DataHeatBalance::AllocateIntGains(state);
        }
        state.dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state.dataGlobal->NumOfZones);
        // Always allocate spaceHeatBalance, even if doSpaceHeatBalance is false, because it's used to gather some of the zone totals
        state.dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(state.dataGlobal->numSpaces);

        state.dataHeatBal->EnclSolQSDifSol.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        state.dataHeatBal->EnclSolQD.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        state.dataHeatBal->EnclSolQDforDaylight.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        state.dataHeatBal->EnclSolDB.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        state.dataHeatBal->EnclSolDBSSG.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        state.dataHeatBal->EnclSolDBIntWin.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        state.dataHeatBal->EnclSolQSWRad.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        state.dataHeatBal->EnclSolQSWRadLights.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
            state.dataHeatBal->EnclSolQSDifSol(enclosureNum) = 0.0;
            state.dataHeatBal->EnclSolQD(enclosureNum) = 0.0;
            state.dataHeatBal->EnclSolQDforDaylight(enclosureNum) = 0.0;
            state.dataHeatBal->EnclSolQSWRad(enclosureNum) = 0.0;
            state.dataHeatBal->EnclSolQSWRadLights(enclosureNum) = 0.0;
            state.dataHeatBal->EnclSolDB(enclosureNum) = 0.0;
            state.dataHeatBal->EnclSolDBSSG(enclosureNum) = 0.0;
            state.dataHeatBal->EnclSolDBIntWin(enclosureNum) = 0.0;
        }
    }
    void AllocateHeatBalArrays(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   February 1998

        // Use the total number of zones or surfaces to allocate variables to avoid a limit
        AllocateZoneHeatBalArrays(state);
        state.dataHeatBalFanSys->SumConvHTRadSys.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->SumLatentHTRadSys.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->SumConvPool.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->SumLatentPool.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->ZoneQdotRadHVACToPerson.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->ZoneQHTRadSysToPerson.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->ZoneQHWBaseboardToPerson.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->ZoneQSteamBaseboardToPerson.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->ZoneQElecBaseboardToPerson.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->ZoneQCoolingPanelToPerson.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalFanSys->ZoneReOrder.allocate(state.dataGlobal->NumOfZones);
        state.dataHeatBalFanSys->ZoneMassBalanceFlag.dimension(state.dataGlobal->NumOfZones, false);
        state.dataHeatBalFanSys->ZoneInfiltrationFlag.dimension(state.dataGlobal->NumOfZones, false);
        state.dataHeatBalFanSys->ZoneReOrder = 0;
        state.dataHeatBalFanSys->TempTstatAir.dimension(state.dataGlobal->NumOfZones, DataHeatBalance::ZoneInitialTemp);
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataContaminantBalance->OutdoorCO2 = state.dataContaminantBalance->Contaminant.CO2OutdoorSched->getCurrentVal();
            state.dataContaminantBalance->ZoneAirCO2.dimension(state.dataGlobal->NumOfZones, state.dataContaminantBalance->OutdoorCO2);
            state.dataContaminantBalance->ZoneAirCO2Temp.dimension(state.dataGlobal->NumOfZones, state.dataContaminantBalance->OutdoorCO2);
            state.dataContaminantBalance->ZoneAirCO2Avg.dimension(state.dataGlobal->NumOfZones, state.dataContaminantBalance->OutdoorCO2);
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataContaminantBalance->OutdoorGC = state.dataContaminantBalance->Contaminant.genericOutdoorSched->getCurrentVal();
            state.dataContaminantBalance->ZoneAirGC.dimension(state.dataGlobal->NumOfZones, state.dataContaminantBalance->OutdoorGC);
            state.dataContaminantBalance->ZoneAirGCTemp.dimension(state.dataGlobal->NumOfZones, state.dataContaminantBalance->OutdoorGC);
            state.dataContaminantBalance->ZoneAirGCAvg.dimension(state.dataGlobal->NumOfZones, state.dataContaminantBalance->OutdoorGC);
        }
        state.dataHeatBalMgr->MaxTempPrevDay.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->MinTempPrevDay.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->MaxHeatLoadPrevDay.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->MaxCoolLoadPrevDay.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->MaxHeatLoadZone.dimension(state.dataGlobal->NumOfZones, -9999.0);
        state.dataHeatBalMgr->MaxCoolLoadZone.dimension(state.dataGlobal->NumOfZones, -9999.0);
        state.dataHeatBalMgr->MaxTempZone.dimension(state.dataGlobal->NumOfZones, -9999.0);
        state.dataHeatBalMgr->MinTempZone.dimension(state.dataGlobal->NumOfZones, 1000.0);
        state.dataHeatBalMgr->TempZonePrevDay.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->LoadZonePrevDay.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->TempZoneSecPrevDay.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->LoadZoneSecPrevDay.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->WarmupTempDiff.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->WarmupLoadDiff.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->TempZone.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->LoadZone.dimension(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalMgr->TempZoneRpt.dimension(state.dataGlobal->NumOfZones, state.dataGlobal->TimeStepsInHour * Constant::iHoursInDay, 0.0);
        state.dataHeatBalMgr->LoadZoneRpt.dimension(state.dataGlobal->NumOfZones, state.dataGlobal->TimeStepsInHour * Constant::iHoursInDay, 0.0);
        state.dataHeatBalMgr->MaxLoadZoneRpt.dimension(state.dataGlobal->NumOfZones, state.dataGlobal->TimeStepsInHour * Constant::iHoursInDay, 0.0);
        state.dataHeatBalMgr->WarmupConvergenceValues.allocate(state.dataGlobal->NumOfZones);
        state.dataHeatBalMgr->TempZoneRptStdDev.allocate(state.dataGlobal->TimeStepsInHour * Constant::iHoursInDay);
        state.dataHeatBalMgr->LoadZoneRptStdDev.allocate(state.dataGlobal->TimeStepsInHour * Constant::iHoursInDay);
        // MassConservation.allocate( NumOfZones );

        state.dataHeatBalFanSys->CrossedColdThreshRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
        state.dataHeatBalFanSys->CrossedHeatThreshRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
        state.dataHeatBalFanSys->CrossedColdThreshRepPeriod = false;
        state.dataHeatBalFanSys->CrossedHeatThreshRepPeriod = false;
        if (state.dataWeather->TotThermalReportPers > 0) {
            state.dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                     state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                 state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                   state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                          state.dataWeather->TotThermalReportPers);
            state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                              state.dataWeather->TotThermalReportPers);
        }

        if (state.dataWeather->TotCO2ReportPers > 0) {
            state.dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotCO2ReportPers);
            state.dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotCO2ReportPers);
            state.dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                    state.dataWeather->TotCO2ReportPers);
        }
        if (state.dataWeather->TotVisualReportPers > 0) {
            state.dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                 state.dataWeather->TotVisualReportPers);
            state.dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                     state.dataWeather->TotVisualReportPers);
            state.dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod.allocate(state.dataGlobal->NumOfZones,
                                                                                         state.dataWeather->TotVisualReportPers);
        }

        state.dataHeatBalFanSys->ZoneLowSETHoursRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
        state.dataHeatBalFanSys->ZoneHighSETHoursRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
        state.dataHeatBalFanSys->lowSETLongestHoursRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
        state.dataHeatBalFanSys->highSETLongestHoursRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
        state.dataHeatBalFanSys->lowSETLongestStartRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);
        state.dataHeatBalFanSys->highSETLongestStartRepPeriod.allocate(state.dataGlobal->NumOfZones, state.dataWeather->TotThermalReportPers);

        state.dataHeatBalMgr->CountWarmupDayPoints = 0;
    }

    // End Initialization Section of the Module
    //******************************************************************************

    // Beginning of Record Keeping subroutines for the HB Module
    // *****************************************************************************

    void RecKeepHeatBalance(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   April 1997
        //       MODIFIED       June 2011, Daeho Kang for individual zone maximums & convergence outputs
        //                       July 2016, Rick Strand for movable insulation bug fix

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main driver for record keeping within the
        // heat balance.

        // Record Maxs & Mins for individual zone
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
            auto const &thisZoneSysEnergyDemand = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum);
            if (thisZoneHB.ZTAV > state.dataHeatBalMgr->MaxTempZone(ZoneNum)) {
                state.dataHeatBalMgr->MaxTempZone(ZoneNum) = thisZoneHB.ZTAV;
            }
            if (thisZoneHB.ZTAV < state.dataHeatBalMgr->MinTempZone(ZoneNum)) {
                state.dataHeatBalMgr->MinTempZone(ZoneNum) = thisZoneHB.ZTAV;
            }
            if (thisZoneSysEnergyDemand.airSysHeatRate > state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum)) {
                state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum) = thisZoneSysEnergyDemand.airSysHeatRate;
            }
            if (thisZoneSysEnergyDemand.airSysCoolRate > state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum)) {
                state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum) = thisZoneSysEnergyDemand.airSysCoolRate;
            }

            // Record temperature and load for individual zone
            state.dataHeatBalMgr->TempZoneSecPrevDay(ZoneNum) = state.dataHeatBalMgr->TempZonePrevDay(ZoneNum);
            state.dataHeatBalMgr->LoadZoneSecPrevDay(ZoneNum) = state.dataHeatBalMgr->LoadZonePrevDay(ZoneNum);
            state.dataHeatBalMgr->TempZonePrevDay(ZoneNum) = state.dataHeatBalMgr->TempZone(ZoneNum);
            state.dataHeatBalMgr->LoadZonePrevDay(ZoneNum) = state.dataHeatBalMgr->LoadZone(ZoneNum);
            state.dataHeatBalMgr->TempZone(ZoneNum) = thisZoneHB.ZTAV;
            state.dataHeatBalMgr->LoadZone(ZoneNum) = max(thisZoneSysEnergyDemand.airSysHeatRate, std::abs(thisZoneSysEnergyDemand.airSysCoolRate));

            // Calculate differences in temperature and load for the last two warmup days
            if (!state.dataGlobal->WarmupFlag && state.dataGlobal->DayOfSim == 1 &&
                (!state.dataGlobal->DoingSizing || state.dataGlobal->DoPureLoadCalc)) {
                state.dataHeatBalMgr->WarmupTempDiff(ZoneNum) =
                    std::abs(state.dataHeatBalMgr->TempZoneSecPrevDay(ZoneNum) - state.dataHeatBalMgr->TempZonePrevDay(ZoneNum));
                state.dataHeatBalMgr->WarmupLoadDiff(ZoneNum) =
                    std::abs(state.dataHeatBalMgr->LoadZoneSecPrevDay(ZoneNum) - state.dataHeatBalMgr->LoadZonePrevDay(ZoneNum));
                if (ZoneNum == 1) {
                    ++state.dataHeatBalMgr->CountWarmupDayPoints;
                }
                state.dataHeatBalMgr->TempZoneRpt(ZoneNum, state.dataHeatBalMgr->CountWarmupDayPoints) =
                    state.dataHeatBalMgr->WarmupTempDiff(ZoneNum);
                state.dataHeatBalMgr->LoadZoneRpt(ZoneNum, state.dataHeatBalMgr->CountWarmupDayPoints) =
                    state.dataHeatBalMgr->WarmupLoadDiff(ZoneNum);
                state.dataHeatBalMgr->MaxLoadZoneRpt(ZoneNum, state.dataHeatBalMgr->CountWarmupDayPoints) = state.dataHeatBalMgr->LoadZone(ZoneNum);

                if (state.dataSysVars->ReportDetailedWarmupConvergence) { // only do this detailed thing when requested by user is on
                    // Write Warmup Convergence Information to the initialization output file
                    if (state.dataHeatBalMgr->FirstWarmupWrite) {
                        constexpr const char *Format_732{
                            "! <Warmup Convergence Information - Detailed>,Zone Name,Time Step,Hour of Day,Warmup Temperature "
                            "Difference {{deltaC}},Warmup Load Difference {{W}}\n"};
                        print(state.files.eio, Format_732);
                        state.dataHeatBalMgr->FirstWarmupWrite = false;
                    }
                    constexpr const char *Format_731{" Warmup Convergence Information - Detailed, {},{},{},{:.10R},{:.10R}\n"};
                    print(state.files.eio,
                          Format_731,
                          state.dataHeatBal->Zone(ZoneNum).Name,
                          state.dataGlobal->TimeStep,
                          state.dataGlobal->HourOfDay,
                          state.dataHeatBalMgr->WarmupTempDiff(ZoneNum),
                          state.dataHeatBalMgr->WarmupLoadDiff(ZoneNum));
                }
            }
        }

        // Update interior movable insulation flag--needed at the end of a zone time step so that the interior radiant
        // exchange algorithm knows whether there has been a change in interior movable insulation or not.
        if (state.dataSurface->AnyMovableInsulation) {
            for (int surfNum : state.dataSurface->intMovInsulSurfNums) {
                auto &movInsul = state.dataSurface->intMovInsuls(surfNum);
                movInsul.presentPrevTS = movInsul.present;
            }
        }

        // For non-complex windows, update a report variable so this shows up in the output as something other than zero
        UpdateWindowFaceTempsNonBSDFWin(state);
    }

    void CheckWarmupConvergence(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   April 1997
        //       MODIFIED       June 2011, Daeho Kang for individual zone comparison

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine checks warmup convergence values.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr MinLoad(100.0); // Minimum loads for convergence check
        // To avoid big percentage difference in low load situations

        // Convergence criteria for warmup days:
        // Perform another warmup day unless both the % change in loads and
        // absolute change in zone temp min & max are less than their criteria.

        if (state.dataGlobal->NumOfZones <= 0) { // if there are no zones, immediate convergence
            state.dataGlobal->WarmupFlag = false;
        } else {
            bool ConvergenceChecksFailed = false;

            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {

                state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxTempValue =
                    std::abs(state.dataHeatBalMgr->MaxTempPrevDay(ZoneNum) - state.dataHeatBalMgr->MaxTempZone(ZoneNum));
                state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMinTempValue =
                    std::abs(state.dataHeatBalMgr->MinTempPrevDay(ZoneNum) - state.dataHeatBalMgr->MinTempZone(ZoneNum));
                if (state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxTempValue <= state.dataHeatBal->TempConvergTol) {
                    state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(1) = 2;
                } else {
                    ConvergenceChecksFailed = true;
                    state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(1) = 1;
                }

                if (state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMinTempValue <= state.dataHeatBal->TempConvergTol) {
                    state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(2) = 2;
                } else {
                    ConvergenceChecksFailed = true;
                    state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(2) = 1;
                }

                if (state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum) > 1.0e-4) { // make sure load big enough to divide
                    state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum) = std::abs(max(state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum), MinLoad));
                    state.dataHeatBalMgr->MaxHeatLoadPrevDay(ZoneNum) = std::abs(max(state.dataHeatBalMgr->MaxHeatLoadPrevDay(ZoneNum), MinLoad));
                    state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxHeatLoadValue =
                        std::abs((state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum) - state.dataHeatBalMgr->MaxHeatLoadPrevDay(ZoneNum)) /
                                 state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum));
                    if (state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxHeatLoadValue <= state.dataHeatBal->LoadsConvergTol) {
                        state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(3) = 2;
                    } else {
                        ConvergenceChecksFailed = true;
                        state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(3) = 1;
                    }
                } else {
                    state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(3) = 2;
                }

                if (state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum) > 1.0e-4) {
                    state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum) = std::abs(max(state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum), MinLoad));
                    state.dataHeatBalMgr->MaxCoolLoadPrevDay(ZoneNum) = std::abs(max(state.dataHeatBalMgr->MaxCoolLoadPrevDay(ZoneNum), MinLoad));
                    state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxCoolLoadValue =
                        std::abs((state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum) - state.dataHeatBalMgr->MaxCoolLoadPrevDay(ZoneNum)) /
                                 state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum));
                    if (state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxCoolLoadValue <= state.dataHeatBal->LoadsConvergTol) {
                        state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(4) = 2;
                    } else {
                        ConvergenceChecksFailed = true;
                        state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(4) = 1;
                    }
                } else {
                    state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(4) = 2;
                }

                if (state.dataGlobal->DayOfSim >= state.dataHeatBal->MaxNumberOfWarmupDays && state.dataGlobal->WarmupFlag) {
                    // Check convergence for individual zone
                    if (sum(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag) != 8) { // pass=2 * 4 values for convergence
                        ShowSevereError(state,
                                        format("CheckWarmupConvergence: Loads Initialization, Zone=\"{}\" did not converge after {} warmup days.",
                                               state.dataHeatBal->Zone(ZoneNum).Name,
                                               state.dataHeatBal->MaxNumberOfWarmupDays));
                        if (!state.dataHeatBalMgr->WarmupConvergenceWarning && !state.dataGlobal->DoingSizing) {
                            ShowContinueError(state, "See Warmup Convergence Information in .eio file for details.");
                            state.dataHeatBalMgr->WarmupConvergenceWarning = true;
                        } else if (!state.dataHeatBalMgr->SizingWarmupConvergenceWarning && state.dataGlobal->DoingSizing) {
                            ShowContinueError(state, "Warmup Convergence failing during sizing.");
                            state.dataHeatBalMgr->SizingWarmupConvergenceWarning = true;
                        }
                        if (state.dataEnvrn->RunPeriodEnvironment) {
                            ShowContinueError(state, format("...Environment(RunPeriod)=\"{}\"", state.dataEnvrn->EnvironmentName));
                        } else {
                            ShowContinueError(state, format("...Environment(SizingPeriod)=\"{}\"", state.dataEnvrn->EnvironmentName));
                        }

                        ShowContinueError(state,
                                          format("..Max Temp Comparison = {:.2R} vs Temperature Convergence Tolerance={:.2R} - {} Convergence",
                                                 state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxTempValue,
                                                 state.dataHeatBal->TempConvergTol,
                                                 PassFail(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(1))));
                        ShowContinueError(state,
                                          format("..Min Temp Comparison = {:.2R} vs Temperature Convergence Tolerance={:.2R} - {} Convergence",
                                                 state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMinTempValue,
                                                 state.dataHeatBal->TempConvergTol,
                                                 PassFail(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(2))));
                        ShowContinueError(state,
                                          format("..Max Heat Load Comparison = {:.4R} vs Loads Convergence Tolerance={:.2R} - {} Convergence",
                                                 state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxHeatLoadValue,
                                                 state.dataHeatBal->LoadsConvergTol,
                                                 PassFail(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(3))));
                        ShowContinueError(state,
                                          format("..Max Cool Load Comparison = {:.4R} vs Loads Convergence Tolerance={:.2R} - {} Convergence",
                                                 state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).TestMaxCoolLoadValue,
                                                 state.dataHeatBal->LoadsConvergTol,
                                                 PassFail(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(4))));
                    }
                }

                // Transfer current daily max and min loads and temperatures to the
                // variables containing the last day's values
                state.dataHeatBalMgr->MaxHeatLoadPrevDay(ZoneNum) = state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum);
                state.dataHeatBalMgr->MaxCoolLoadPrevDay(ZoneNum) = state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum);
                state.dataHeatBalMgr->MaxTempPrevDay(ZoneNum) = state.dataHeatBalMgr->MaxTempZone(ZoneNum);
                state.dataHeatBalMgr->MinTempPrevDay(ZoneNum) = state.dataHeatBalMgr->MinTempZone(ZoneNum);

                state.dataHeatBalMgr->MaxHeatLoadZone(ZoneNum) = -9999.0;
                state.dataHeatBalMgr->MaxCoolLoadZone(ZoneNum) = -9999.0;
                state.dataHeatBalMgr->MaxTempZone(ZoneNum) = -9999.0;
                state.dataHeatBalMgr->MinTempZone(ZoneNum) = 1000.0;
            }

            // Limit the number of warmup days, regardless of the number of zones
            // in the building, to some arbitrary value based on common sense and
            // experience with the (I)BLAST program.  If too many warmup days were
            // required, notify the program user.

            if ((state.dataGlobal->DayOfSim >= state.dataHeatBal->MaxNumberOfWarmupDays) && state.dataGlobal->WarmupFlag && ConvergenceChecksFailed) {
                if (state.dataHeatBal->MaxNumberOfWarmupDays < DataHeatBalance::DefaultMaxNumberOfWarmupDays) {
                    ShowSevereError(state,
                                    format("CheckWarmupConvergence: User supplied maximum warmup days={} is insufficient.",
                                           state.dataHeatBal->MaxNumberOfWarmupDays));
                    ShowContinueError(
                        state,
                        format("Suggest setting maximum number of warmup days to at least {}.", DataHeatBalance::DefaultMaxNumberOfWarmupDays));
                }
            }

            // Set warmup flag to true depending on value of ConvergenceChecksFailed (true=fail)
            // and minimum number of warmup days
            if (!ConvergenceChecksFailed && state.dataGlobal->DayOfSim >= state.dataHeatBal->MinNumberOfWarmupDays) {
                state.dataGlobal->WarmupFlag = false;
            } else if (!ConvergenceChecksFailed && state.dataGlobal->DayOfSim < state.dataHeatBal->MinNumberOfWarmupDays) {
                state.dataGlobal->WarmupFlag = true;
            }

            // If max warmup days reached and still WarmupFlag, then go to non-warmup state.
            // prior messages will have been displayed
            if ((state.dataGlobal->DayOfSim >= state.dataHeatBal->MaxNumberOfWarmupDays) && state.dataGlobal->WarmupFlag) {
                state.dataGlobal->WarmupFlag = false;
            }
        }
    }

    void ReportWarmupConvergence(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   October 2011

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AverageZoneTemp;
        Real64 AverageZoneLoad;
        Real64 StdDevZoneTemp;
        Real64 StdDevZoneLoad;

        if (!state.dataGlobal->WarmupFlag) { // Report out average/std dev
            // Write Warmup Convervence Information to the initialization output file
            if (state.dataHeatBalMgr->ReportWarmupConvergenceFirstWarmupWrite && state.dataGlobal->NumOfZones > 0) {
                constexpr const char *Format_730(
                    "! <Warmup Convergence Information>,Zone Name,Environment Type/Name,Average Warmup Temperature Difference "
                    "{{deltaC}},Std Dev Warmup Temperature Difference {{deltaC}},Max Temperature Pass/Fail Convergence,Min "
                    "Temperature Pass/Fail Convergence,Average Warmup Load Difference {{W}},Std Dev Warmup Load Difference "
                    "{{W}},Heating Load Pass/Fail Convergence,Cooling Load Pass/Fail Convergence\n");
                print(state.files.eio, Format_730);
                state.dataHeatBalMgr->ReportWarmupConvergenceFirstWarmupWrite = false;
            }

            state.dataHeatBalMgr->TempZoneRptStdDev = 0.0;
            state.dataHeatBalMgr->LoadZoneRptStdDev = 0.0;

            std::string EnvHeader;
            if (state.dataEnvrn->RunPeriodEnvironment) {
                EnvHeader = "RunPeriod:";
            } else {
                EnvHeader = "SizingPeriod:";
            }

            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                AverageZoneTemp = sum(state.dataHeatBalMgr->TempZoneRpt(ZoneNum, {1, state.dataHeatBalMgr->CountWarmupDayPoints})) /
                                  double(state.dataHeatBalMgr->CountWarmupDayPoints);
                for (int Num = 1; Num <= state.dataHeatBalMgr->CountWarmupDayPoints; ++Num) {
                    if (state.dataHeatBalMgr->MaxLoadZoneRpt(ZoneNum, Num) > 1.e-4) {
                        state.dataHeatBalMgr->LoadZoneRpt(ZoneNum, Num) /= state.dataHeatBalMgr->MaxLoadZoneRpt(ZoneNum, Num);
                    } else {
                        state.dataHeatBalMgr->LoadZoneRpt(ZoneNum, Num) = 0.0;
                    }
                }
                AverageZoneLoad = sum(state.dataHeatBalMgr->LoadZoneRpt(ZoneNum, {1, state.dataHeatBalMgr->CountWarmupDayPoints})) /
                                  double(state.dataHeatBalMgr->CountWarmupDayPoints);
                StdDevZoneTemp = 0.0;
                StdDevZoneLoad = 0.0;
                for (int Num = 1; Num <= state.dataHeatBalMgr->CountWarmupDayPoints; ++Num) {
                    state.dataHeatBalMgr->TempZoneRptStdDev(Num) = pow_2(state.dataHeatBalMgr->TempZoneRpt(ZoneNum, Num) - AverageZoneTemp);
                    state.dataHeatBalMgr->LoadZoneRptStdDev(Num) = pow_2(state.dataHeatBalMgr->LoadZoneRpt(ZoneNum, Num) - AverageZoneLoad);
                }
                StdDevZoneTemp = std::sqrt(sum(state.dataHeatBalMgr->TempZoneRptStdDev({1, state.dataHeatBalMgr->CountWarmupDayPoints})) /
                                           double(state.dataHeatBalMgr->CountWarmupDayPoints));
                StdDevZoneLoad = std::sqrt(sum(state.dataHeatBalMgr->LoadZoneRptStdDev({1, state.dataHeatBalMgr->CountWarmupDayPoints})) /
                                           double(state.dataHeatBalMgr->CountWarmupDayPoints));

                constexpr const char *Format_731(" Warmup Convergence Information,{},{},{:.10R},{:.10R},{},{},{:.10R},{:.10R},{},{}\n");
                print(state.files.eio,
                      Format_731,
                      state.dataHeatBal->Zone(ZoneNum).Name,
                      EnvHeader + ' ' + state.dataEnvrn->EnvironmentName,
                      AverageZoneTemp,
                      StdDevZoneTemp,
                      PassFail(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(1)),
                      PassFail(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(2)),
                      AverageZoneLoad,
                      StdDevZoneLoad,
                      PassFail(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(3)),
                      PassFail(state.dataHeatBalMgr->WarmupConvergenceValues(ZoneNum).PassFlag(4)));
            }
        }
    }

    void UpdateWindowFaceTempsNonBSDFWin(EnergyPlusData &state)
    {
        for (int SurfNum : state.dataSurface->AllHTWindowSurfaceList) {
            auto &thisConstruction = state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction);
            if (thisConstruction.WindowTypeBSDF) {
                continue;
            }
            state.dataHeatBal->SurfWinFenLaySurfTempFront(SurfNum, 1) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
            state.dataHeatBal->SurfWinFenLaySurfTempBack(SurfNum, thisConstruction.TotLayers) = state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum);
        }
    }

    //        End of Record Keeping subroutines for the HB Module
    // *****************************************************************************

    // Beginning of Reporting subroutines for the HB Module
    // *****************************************************************************

    void ReportHeatBalance(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   July 1997

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main driver for reporting within the heat
        // balance.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger record keeping events.

        // Using/Aliasing
        using EconomicTariff::UpdateUtilityBills; // added for computing annual utility costs
        using NodeInputManager::CalcMoreNodeInfo;
        using OutputReportTabular::UpdateTabularReports;

        Sched::ReportScheduleVals(state);

        if (!state.dataGlobal->WarmupFlag && state.dataGlobal->DoOutputReporting) {
            if (!state.dataGlobal->DoingSizing) {
                CalcMoreNodeInfo(state);
            }
            UpdateDataandReport(state, OutputProcessor::TimeStepType::Zone);
            if (state.dataGlobal->KindOfSim == Constant::KindOfSim::HVACSizeDesignDay ||
                state.dataGlobal->KindOfSim == Constant::KindOfSim::HVACSizeRunPeriodDesign) {
                if (state.dataHVACSizingSimMgr->hvacSizingSimulationManager) {
                    state.dataHVACSizingSimMgr->hvacSizingSimulationManager->UpdateSizingLogsZoneStep(state);
                }
            }

            UpdateTabularReports(state, OutputProcessor::TimeStepType::Zone);
            UpdateUtilityBills(state);
        } else if (!state.dataGlobal->KickOffSimulation && state.dataGlobal->DoOutputReporting && state.dataSysVars->ReportDuringWarmup) {
            if (state.dataGlobal->BeginDayFlag && !state.dataEnvrn->PrintEnvrnStampWarmupPrinted) {
                state.dataEnvrn->PrintEnvrnStampWarmup = true;
                state.dataEnvrn->PrintEnvrnStampWarmupPrinted = true;
            }
            if (!state.dataGlobal->BeginDayFlag) {
                state.dataEnvrn->PrintEnvrnStampWarmupPrinted = false;
            }
            if (state.dataEnvrn->PrintEnvrnStampWarmup) {
                if (state.dataReportFlag->PrintEndDataDictionary && state.dataGlobal->DoOutputReporting) {
                    constexpr const char *EndOfHeaderString("End of Data Dictionary"); // End of data dictionary marker
                    print(state.files.eso, "{}\n", EndOfHeaderString);
                    print(state.files.mtr, "{}\n", EndOfHeaderString);
                    state.dataReportFlag->PrintEndDataDictionary = false;
                }
                if (state.dataGlobal->DoOutputReporting) {
                    constexpr const char *EnvironmentStampFormatStr("{},{},{:7.2F},{:7.2F},{:7.2F},{:7.2F}\n"); // Format descriptor for environ stamp
                    print(state.files.eso,
                          EnvironmentStampFormatStr,
                          "1",
                          "Warmup {" + state.dataReportFlag->cWarmupDay + "} " + state.dataEnvrn->EnvironmentName,
                          state.dataEnvrn->Latitude,
                          state.dataEnvrn->Longitude,
                          state.dataEnvrn->TimeZoneNumber,
                          state.dataEnvrn->Elevation);

                    print(state.files.mtr,
                          EnvironmentStampFormatStr,
                          "1",
                          "Warmup {" + state.dataReportFlag->cWarmupDay + "} " + state.dataEnvrn->EnvironmentName,
                          state.dataEnvrn->Latitude,
                          state.dataEnvrn->Longitude,
                          state.dataEnvrn->TimeZoneNumber,
                          state.dataEnvrn->Elevation);
                    state.dataEnvrn->PrintEnvrnStampWarmup = false;
                }
            }
            if (!state.dataGlobal->DoingSizing) {
                CalcMoreNodeInfo(state);
            }
            UpdateDataandReport(state, OutputProcessor::TimeStepType::Zone);
            if (state.dataGlobal->KindOfSim == Constant::KindOfSim::HVACSizeDesignDay ||
                state.dataGlobal->KindOfSim == Constant::KindOfSim::HVACSizeRunPeriodDesign) {
                if (state.dataHVACSizingSimMgr->hvacSizingSimulationManager) {
                    state.dataHVACSizingSimMgr->hvacSizingSimulationManager->UpdateSizingLogsZoneStep(state);
                }
            }

        } else if (state.dataSysVars->UpdateDataDuringWarmupExternalInterface) { // added for FMI
            UpdateDataandReport(state, OutputProcessor::TimeStepType::Zone);
            if (state.dataGlobal->KindOfSim == Constant::KindOfSim::HVACSizeDesignDay ||
                state.dataGlobal->KindOfSim == Constant::KindOfSim::HVACSizeRunPeriodDesign) {
                if (state.dataHVACSizingSimMgr->hvacSizingSimulationManager) {
                    state.dataHVACSizingSimMgr->hvacSizingSimulationManager->UpdateSizingLogsZoneStep(state);
                }
            }
        }
        // There is no hourly reporting in the heat balance.

        // There is no daily reporting in the heat balance.

        // There is no simulation level record keeping in the heat balance.
    }

    //        End of Reporting subroutines for the HB Module

    void OpenShadingFile(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         X Luo
        //       DATE WRITTEN   August 2017

        // PURPOSE OF THIS SUBROUTINE:
        // Open and set up headers for a external shading fraction export file.

        state.files.shade.ensure_open(state, "OpenOutputFiles", state.files.outputControl.extshd);
        print(state.files.shade, "Surface Name,");
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            print(state.files.shade, "{},", state.dataSurface->Surface(SurfNum).Name);
        }
        print(state.files.shade, "\n");
    }
    void GetFrameAndDividerData(EnergyPlusData &state) // set to true if errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   May 2000
        //       MODIFIED       April 2002 (FCW): get window reveal data

        // PURPOSE OF THIS SUBROUTINE:
        // Gets input data for window frame and/or divider and/or window
        // inside/outside reveal.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;                            // IO Status when calling get input subroutine
        Array1D_string FrameDividerAlphas(3);  // Frame/Divider Alpha names
        int FrameDividerNum;                   // Counter to keep track of the frame/divider number
        int FrameDividerNumAlpha;              // Number of frame/divider alpha names being passed
        int FrameDividerNumProp;               // Number of frame/divider properties being passed
        Array1D<Real64> FrameDividerProps(23); // Temporary array to transfer frame/divider properties
        int Loop;

        constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::NfrcProductOptions::Num)> NfrcProductNamesUC = {
            "CASEMENTDOUBLE", "CASEMENTSINGLE",   "DUALACTION",
            "FIXED",          "GARAGE",           "GREENHOUSE",
            "HINGEDESCAPE",   "HORIZONTALSLIDER", "JAL",
            "PIVOTED",        "PROJECTINGSINGLE", "PROJECTINGDUAL",
            "DOORSIDELITE",   "SKYLIGHT",         "SLIDINGPATIODOOR",
            "CURTAINWALL",    "SPANDRELPANEL",    "SIDEHINGEDDOOR",
            "DOORTRANSOM",    "TROPICALAWNING",   "TUBULARDAYLIGHTINGDEVICE",
            "VERTICALSLIDER"};

        constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::FrameDividerType::Num)> FrameDividerTypeNamesUC = {
            "DIVIDEDLITE", // 0
            "SUSPENDED"    // 1
        };

        state.dataHeatBalMgr->CurrentModuleObject = "WindowProperty:FrameAndDivider";
        state.dataHeatBal->TotFrameDivider =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHeatBalMgr->CurrentModuleObject);
        state.dataSurface->FrameDivider.allocate(state.dataHeatBal->TotFrameDivider);
        if (state.dataHeatBal->TotFrameDivider == 0) {
            return;
        }

        FrameDividerNum = 0;

        for (Loop = 1; Loop <= state.dataHeatBal->TotFrameDivider; ++Loop) {

            // Call Input Get routine to retrieve frame/divider data
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     Loop,
                                                                     FrameDividerAlphas,
                                                                     FrameDividerNumAlpha,
                                                                     FrameDividerProps,
                                                                     FrameDividerNumProp,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            // Load the frame/divider derived type from the input data.
            ++FrameDividerNum;
            auto &frameDivider = state.dataSurface->FrameDivider(FrameDividerNum);
            frameDivider.Name = FrameDividerAlphas(1);
            frameDivider.FrameWidth = FrameDividerProps(1);
            frameDivider.FrameProjectionOut = FrameDividerProps(2);
            frameDivider.FrameProjectionIn = FrameDividerProps(3);
            if (frameDivider.FrameWidth == 0.0) {
                frameDivider.FrameProjectionOut = 0.0;
                frameDivider.FrameProjectionIn = 0.0;
            }
            frameDivider.FrameConductance = FrameDividerProps(4);
            frameDivider.FrEdgeToCenterGlCondRatio = FrameDividerProps(5);
            frameDivider.FrameSolAbsorp = FrameDividerProps(6);
            frameDivider.FrameVisAbsorp = FrameDividerProps(7);
            frameDivider.FrameEmis = FrameDividerProps(8);

            DataSurfaces::FrameDividerType currentDividerType =
                DataSurfaces::FrameDividerType(getEnumValue(FrameDividerTypeNamesUC, FrameDividerAlphas(2)));
            if (currentDividerType == DataSurfaces::FrameDividerType::Invalid) {
                ShowWarningError(state,
                                 fmt::format("{}={}, Invalid {}",
                                             state.dataHeatBalMgr->CurrentModuleObject,
                                             std::quoted(FrameDividerAlphas(1)),
                                             state.dataIPShortCut->cAlphaFieldNames(2)));
                ShowContinueError(
                    state,
                    fmt::format("Entered={}, must be DividedLite or Suspended.  Will be set to DividedLite.", std::quoted(FrameDividerAlphas(2))));
                frameDivider.DividerType = DataSurfaces::FrameDividerType::DividedLite;
            } else {
                frameDivider.DividerType = currentDividerType;
            }

            frameDivider.DividerWidth = FrameDividerProps(9);
            frameDivider.HorDividers = FrameDividerProps(10);
            frameDivider.VertDividers = FrameDividerProps(11);
            frameDivider.DividerProjectionOut = FrameDividerProps(12);
            frameDivider.DividerProjectionIn = FrameDividerProps(13);
            if (frameDivider.DividerWidth == 0.0 || frameDivider.DividerType == DataSurfaces::FrameDividerType::Suspended) {
                frameDivider.DividerProjectionOut = 0.0;
                frameDivider.DividerProjectionIn = 0.0;
            }
            frameDivider.DividerConductance = FrameDividerProps(14);
            frameDivider.DivEdgeToCenterGlCondRatio = FrameDividerProps(15);
            frameDivider.DividerSolAbsorp = FrameDividerProps(16);
            frameDivider.DividerVisAbsorp = FrameDividerProps(17);
            frameDivider.DividerEmis = FrameDividerProps(18);

            // look up the NFRC Product Type for Assembly Calculations using the DataSurfaces::NfrcProductName
            frameDivider.NfrcProductType = DataSurfaces::NfrcProductOptions(getEnumValue(NfrcProductNamesUC, FrameDividerAlphas(3)));
            if (frameDivider.NfrcProductType == DataSurfaces::NfrcProductOptions::Invalid) {
                frameDivider.NfrcProductType = DataSurfaces::NfrcProductOptions::CurtainWall;
            }

            frameDivider.OutsideRevealSolAbs = FrameDividerProps(19);
            frameDivider.InsideSillDepth = FrameDividerProps(20);
            frameDivider.InsideSillSolAbs = FrameDividerProps(21);
            frameDivider.InsideReveal = FrameDividerProps(22);
            frameDivider.InsideRevealSolAbs = FrameDividerProps(23);

            if (frameDivider.DividerWidth > 0.0 && (frameDivider.HorDividers == 0 && frameDivider.VertDividers == 0)) {
                ShowWarningError(state,
                                 format("{}: In FrameAndDivider {} {} > 0 ",
                                        state.dataHeatBalMgr->CurrentModuleObject,
                                        frameDivider.Name,
                                        state.dataIPShortCut->cNumericFieldNames(9)));
                ShowContinueError(
                    state,
                    format("...but {} = 0 and {} = 0.", state.dataIPShortCut->cNumericFieldNames(10), state.dataIPShortCut->cNumericFieldNames(11)));
                ShowContinueError(state, format("...{} set to 0.", state.dataIPShortCut->cNumericFieldNames(9)));
                frameDivider.DividerWidth = 0.0;
            }
            // Prevent InsideSillDepth < InsideReveal
            if (frameDivider.InsideSillDepth < state.dataSurface->FrameDivider(FrameDividerNum).InsideReveal) {
                ShowWarningError(state,
                                 format("{}: In FrameAndDivider {} {} is less than {}; it will be set to {}.",
                                        state.dataHeatBalMgr->CurrentModuleObject,
                                        frameDivider.Name,
                                        state.dataIPShortCut->cNumericFieldNames(20),
                                        state.dataIPShortCut->cNumericFieldNames(22),
                                        state.dataIPShortCut->cNumericFieldNames(22)));
                frameDivider.InsideSillDepth = state.dataSurface->FrameDivider(FrameDividerNum).InsideReveal;
            }

            //    ! Warn if InsideSillDepth OR InsideReveal > 0.2meters to warn of inaccuracies
            //    IF(FrameDivider(FrameDividerNum)%InsideSillDepth > 0.2d0) THEN
            //      CALL ShowWarningError(state, TRIM(state.dataHeatBalMgr->CurrentModuleObject)//': In FrameAndDivider
            //      '//TRIM(FrameDivider(FrameDividerNum)%Name)// &
            //        ' '//TRIM(cNumericFieldNames(20))//' is greater than 0.2 meters, which could cause inaccuracies in zone cooling energy.')
            //    END IF
            //    IF(FrameDivider(FrameDividerNum)%InsideReveal > 0.2d0) THEN
            //      CALL ShowWarningError(state, TRIM(state.dataHeatBalMgr->CurrentModuleObject)//': In FrameAndDivider
            //      '//TRIM(FrameDivider(FrameDividerNum)%Name)// &
            //        ' '//TRIM(cNumericFieldNames(22))//' is greater than 0.2 meters, which could cause inaccuracies in zone cooling energy.')
            //    END IF
        }
    }

    void SearchWindow5DataFile(EnergyPlusData &state,
                               fs::path const &DesiredFilePath,            // File path that contains the Window5 constructions.
                               std::string const &DesiredConstructionName, // Name that will be searched for in the Window5 data file
                               bool &ConstructionFound,                    // True if DesiredConstructionName is in the Window5 data file
                               bool &EOFonFile,                            // True if EOF during file read
                               bool &ErrorsFound                           // True if there is a problem with the entry requested from the data file
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   August 2001
        //       MODIFIED       June 2002, FW: do all reallocation here for constructions found on
        //                        data file; 1 new construction of entry has one glazing system;
        //                        2 new constructions if entry has two glazing systems.
        //                      Nov 2002, FW: skip read of mullion data line if one glazing system;
        //                        add error messages for bad data; increase length of input line
        //                        from 132 to 200 to handle case where Window5 puts in extra blanks
        //                        in gas data line.
        //                      Feb 2007, LKL: Add more checks on Window5DataFile
        //                      Jan 2008, LKL: Change Edge/Cond ratio check.

        // PURPOSE OF THIS SUBROUTINE:
        // Searches the WINDOW5 data file for a window with the name "DesiredConstructionName,"
        // which is the name of an idf Construction input using CONSTRUCTION FROM WINDOW5 DATA FILE.
        // (The WINDOW5 data file contains data for one or more complete windows --
        // glazing, frame, mullion, and divider.
        // WINDOW5 writes the data file for export to EnergyPlus so that an annual energy
        // analysis can be done on exactly the same window without having to re-input into
        // EnergyPlus.)

        // If a match is found, a Construction is created and the Material objects associated with
        // the Construction are created. If there is an associated frame or
        // divider in the Window5 data file for this Construction, a FrameAndDivider object will
        // also be created.

        // If the window on the data file has two glazing systems, a second Construction (and its
        // associated materials) corresponding to the second glazing system is created.

        // Using/Aliasing
        using namespace DataStringGlobals;
        using DataSystemVariables::CheckForActualFilePath;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Array1D_string const NumName(5, {"1", "2", "3", "4", "5"});

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int FileLineCount;            // counter for number of lines read (used in some error messages)
        Array1D_string DataLine(100); // Array of data lines
        std::string WindowNameInW5DataFile;
        std::string W5Name;
        Array1D_string GasName(3);      // Gas name from data file
        std::string LayerName;          // Layer name from data file
        std::string MullionOrientation; // Horizontal, vertical or none
        int LineNum;
        Array1D_int NGlass(2);              // Number of glass layers in glazing system
        Array2D_int NumGases(4, 2);         // Number of gases in each gap of a glazing system
        Array2D_int MaterNumSysGlass(5, 2); // Material numbers for glazing system / glass combinations
        Array2D_int MaterNumSysGap(4, 2);   // Material numbers for glazing system / gap combinations
        int TotFrameDividerPrev;            // Number of FrameAndDivider objects before adding ones from W5DataFile
        Array1D_int NGaps(2);               // Number of gaps in window construction
        int NGlSys;                         // Number of glazing systems (normally 1, but 2 for mullioned window
        //  with two different glazing systems
        int loop;      // DO loop counter
        int ILine;     // Line counter
        int ConstrNum; // Construction number
        int IGlass;    // Glass layer counter
        int IGap;      // Gap counter
        //  INTEGER            :: ICoeff              ! Gas property coefficient counter
        int IGlSys;                   // Glazing system counter
        int FrDivNum;                 // FrameDivider number
        Array1D<Real64> WinHeight(2); // Height, width for glazing system (m)
        Array1D<Real64> WinWidth(2);
        Array1D<Real64> UValCenter(2);                 // Center of glass U-value (W/m2-K) for glazing system
        Array1D<Real64> SCCenter(2);                   // Center of glass shading coefficient for glazing system
        Array1D<Real64> SHGCCenter(2);                 // Center of glass solar heat gain coefficient for glazing system
        Array1D<Real64> TVisCenter(2);                 // Center of glass visible transmittance for glazing system
        Array1D<Real64> TsolTemp(Window::numPhis + 1); // Solar transmittance vs incidence angle; diffuse trans.
        std::array<Real64, Window::numPhis> Tsol;
        Array2D<Real64> AbsSolTemp(Window::maxGlassLayers, Window::numPhis + 1);     // Solar absorptance vs inc. angle in each glass layer
        Array1D<std::array<Real64, Window::numPhis>> AbsSol(Window::maxGlassLayers); // Solar absorptance vs inc. angle in each glass layer
        Array1D<Real64> RfsolTemp(Window::numPhis + 1);                              // Front solar reflectance vs inc. angle
        std::array<Real64, Window::numPhis> Rfsol;
        Array1D<Real64> RbsolTemp(Window::numPhis + 1); // Back solar reflectance vs inc. angle
        std::array<Real64, Window::numPhis> Rbsol;
        Array1D<Real64> TvisTemp(Window::numPhis + 1); // Visible transmittance vs inc. angle
        std::array<Real64, Window::numPhis> Tvis;
        Array1D<Real64> RfvisTemp(Window::numPhis + 1); // Front visible reflectance vs inc. angle
        std::array<Real64, Window::numPhis> Rfvis;
        Array1D<Real64> RbvisTemp(Window::numPhis + 1); // Back visible reflectance vs inc. angle
        std::array<Real64, Window::numPhis> Rbvis;

        std::array<Real64, Window::numPhis> tsolFit;  // Fitted solar transmittance vs incidence angle
        std::array<Real64, Window::numPhis> tvisFit;  // Fitted visible transmittance vs incidence angle
        std::array<Real64, Window::numPhis> rfsolFit; // Fitted solar front reflectance vs incidence angle
        Array1D<std::array<Real64, Window::numPhis>> solabsFit(
            Window::maxGlassLayers);   // Fitted solar absorptance vs incidence angle for each glass layer
        Array1D_string DividerType(2); // Divider type: DividedLite or Suspended
        Real64 FrameWidth;
        Real64 MullionWidth;
        Real64 FrameProjectionOut;
        Real64 FrameProjectionIn;
        Real64 FrameConductance;
        Real64 FrEdgeToCenterGlCondRatio;
        Real64 FrameSolAbsorp;
        Real64 FrameVisAbsorp;
        Real64 FrameEmis;
        Array1D_int HorDividers(2);  // For divider: number horizontal for each glazing system
        Array1D_int VertDividers(2); // For divider: number vertical for each glazing system
        Array1D<Real64> DividerWidth(2);
        Array1D<Real64> DividerProjectionOut(2);
        Array1D<Real64> DividerProjectionIn(2);
        Array1D<Real64> DividerConductance(2);
        Array1D<Real64> DivEdgeToCenterGlCondRatio(2);
        Array1D<Real64> DividerSolAbsorp(2);
        Array1D<Real64> DividerVisAbsorp(2);
        Array1D<Real64> DividerEmis(2);
        std::string::size_type endcol;

        auto &s_mat = state.dataMaterial;
        // Object Data

        // In the following four gas-related data sets, the first
        //  index is gas type (1=air, 2=Argon, 3=Krypton, 4=Xenon)
        //  and the second index gives a,b,c in the expression
        //  property value = a + bT(K) + cT(K)**2, where T is mean
        //  gap temperature in deg K.

        ConstructionFound = false;
        // ErrorsFound = .FALSE.
        EOFonFile = false;
        std::string contextString = "HeatBalanceManager::SearchWindow5DataFile: ";

        state.files.TempFullFilePath.filePath = CheckForActualFilePath(state, DesiredFilePath, contextString);

        // INQUIRE(FILE=TRIM(DesiredFileName), EXIST=exists)
        if (state.files.TempFullFilePath.filePath.empty()) {
            ShowFatalError(state, "Program terminates due to these conditions.");
        }

        auto W5DataFile = state.files.TempFullFilePath.open(state, "SearchWindow5DataFile");
        auto NextLine = W5DataFile.readLine();
        endcol = len(NextLine.data);
        if (endcol > 0) {
            if (int(NextLine.data[endcol - 1]) == DataSystemVariables::iUnicode_end) {
                ShowSevereError(state,
                                format("SearchWindow5DataFile: For \"{}\" in {} file, appears to be a Unicode or binary file.",
                                       DesiredConstructionName,
                                       DesiredFilePath));
                ShowContinueError(state, "...This file cannot be read by this program. Please save as PC or Unix file and try again");
                ShowFatalError(state, "Program terminates due to previous condition.");
            }
        }
        W5DataFile.rewind();
        FileLineCount = 0;

        NextLine = W5DataFile.readLine();
        if (NextLine.eof) {
            goto Label1000;
        }
        ++FileLineCount;
        if (!has_prefixi(NextLine.data, "WINDOW5")) {
            ShowSevereError(state, format("HeatBalanceManager: SearchWindow5DataFile: Error in Data File={}", DesiredFilePath));
            ShowFatalError(
                state,
                format("Error reading Window5 Data File: first word of window entry is \"{}\", should be Window5.", NextLine.data.substr(0, 7)));
        }

    Label10:;
        NextLine = W5DataFile.readLine();
        if (NextLine.eof) {
            goto Label1000;
        }
        ++FileLineCount;
        if (!has_prefixi(NextLine.data, "WINDOW NAME")) {
            // Berkeley Lab WINDOW 7 has been found to include extra blank lines after the "WINDOW5" line
            // so find the line that begins with "WINDOW NAME"
            goto Label10;
        } else {
            // Get window name
            W5Name = std::string{NextLine.data.substr(19)};
            WindowNameInW5DataFile = Util::makeUPPER(W5Name);

            // Get description (currently not used)
            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000;
            }
            ++FileLineCount;
        }
        // Check for window name match
        if (DesiredConstructionName != WindowNameInW5DataFile) {
            // Doesn't match; read through file until next window entry is found
        Label20:;
            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000;
            }
            ++FileLineCount;
            if (!has_prefixi(NextLine.data, "WINDOW5")) {
                goto Label20;
            }
            // Beginning of next window entry found
            goto Label10;
        } else {
            // Match found
            ConstructionFound = true;

            // Create Material:WindowGlass, Material:WindowGas, Construction
            // and WindowFrameAndDividerObjects for this window

            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000;
            }
            ++FileLineCount;
            bool error = false;
            NGlSys = static_cast<int>(Util::ProcessNumber(NextLine.data.substr(19), error));
            if (NGlSys <= 0 || NGlSys > 2 || error) {
                ShowFatalError(
                    state,
                    format("Construction={} from the Window5 data file cannot be used: it has {} glazing systems; only 1 or 2 are allowed.",
                           DesiredConstructionName,
                           NGlSys));
            }
            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000;
            }
            ++FileLineCount;
            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                NextLine = W5DataFile.readLine();
                if (NextLine.eof) {
                    goto Label1000;
                }
                ++FileLineCount;

                const bool succeeded = readList(NextLine.data.substr(19),
                                                WinHeight(IGlSys),
                                                WinWidth(IGlSys),
                                                NGlass(IGlSys),
                                                UValCenter(IGlSys),
                                                SCCenter(IGlSys),
                                                SHGCCenter(IGlSys),
                                                TVisCenter(IGlSys));
                if (!succeeded) {
                    ShowSevereError(
                        state,
                        format("HeatBalanceManager: SearchWindow5DataFile: Error in Read of glazing system values. For glazing system={}", IGlSys));
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount, NextLine.data.substr(0, 100)));
                    ErrorsFound = true;
                }
                if (WinHeight(IGlSys) == 0.0 || WinWidth(IGlSys) == 0.0) {
                    ShowSevereError(state,
                                    format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used: it "
                                           "has window height or width = 0 for glazing system {}",
                                           DesiredConstructionName,
                                           IGlSys));
                    ErrorsFound = true;
                }
                if (NGlass(IGlSys) <= 0 || NGlass(IGlSys) > 4) {
                    ShowSevereError(state,
                                    format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used: it "
                                           "has 0 or more than 4 glass layers in glazing system {}",
                                           DesiredConstructionName,
                                           IGlSys));
                    ErrorsFound = true;
                }
                if (UValCenter(IGlSys) <= 0.0) {
                    ShowSevereError(state,
                                    format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used: it "
                                           "has Center-of-Glass U-value <= 0 in glazing system {}",
                                           DesiredConstructionName,
                                           IGlSys));
                    ErrorsFound = true;
                }
                if (SCCenter(IGlSys) <= 0.0) {
                    ShowWarningError(
                        state,
                        format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file has flawed data: it "
                               "has a Shading Coefficient <= 0 in glazing system {}",
                               DesiredConstructionName,
                               IGlSys));
                }
                if (SHGCCenter(IGlSys) <= 0.0) {
                    ShowWarningError(
                        state,
                        format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file has flawed data: it "
                               "has a SHGC <= 0 in glazing system {}",
                               DesiredConstructionName,
                               IGlSys));
                }
                WinHeight(IGlSys) *= 0.001;
                WinWidth(IGlSys) *= 0.001;
            }
            for (LineNum = 1; LineNum <= 11; ++LineNum) {
                NextLine = W5DataFile.readLine();
                if (NextLine.eof) {
                    goto Label1000;
                }
                DataLine(LineNum) = NextLine.data;
            }

            // Mullion width and orientation
            MullionWidth = 0.0;
            MullionOrientation = "Vertical";
            if (NGlSys == 2) {
                error = false;
                MullionWidth = Util::ProcessNumber(DataLine(10).substr(19), error);
                if (error) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of Mullion Width.");
                    ShowContinueError(state,
                                      format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 10, DataLine(10).substr(0, 100)));
                    ErrorsFound = true;
                }
                MullionWidth *= 0.001;
                MullionOrientation = Util::ProcessNumber(DataLine(10).substr(88), error);
                if (error) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of Mullion Orientation.");
                    ShowContinueError(state,
                                      format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 10, DataLine(10).substr(0, 100)));
                    ErrorsFound = true;
                }
            }

            // Frame data; if there are two glazing systems, the frame is assumed to be
            // the same for both.
            FrameWidth = 0.0;
            FrameProjectionOut = 0.0;
            FrameProjectionIn = 0.0;
            FrameConductance = 0.0;
            FrEdgeToCenterGlCondRatio = 0.0;
            FrameSolAbsorp = 0.0;
            FrameVisAbsorp = 0.0;
            FrameEmis = 0.0;
            const bool succeeded = readList(DataLine(11).substr(19),
                                            FrameWidth,
                                            FrameProjectionOut,
                                            FrameProjectionIn,
                                            FrameConductance,
                                            FrEdgeToCenterGlCondRatio,
                                            FrameSolAbsorp,
                                            FrameVisAbsorp,
                                            FrameEmis);
            if (!succeeded) {
                ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of frame data values.");
                ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 11, DataLine(11).substr(0, 100)));
                ErrorsFound = true;
            }
            if (FrameWidth > 0.0) {
                if (FrameConductance <= 0.0) {
                    ShowSevereError(state,
                                    format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used: it "
                                           "has Frame Conductance <= 0.0",
                                           DesiredConstructionName));
                    ErrorsFound = true;
                }
                // Relax this check for Window5 data: 1/28/2008.
                //        IF(FrEdgeToCenterGlCondRatio < 1.0) THEN
                //            CALL ShowSevereError(state, 'HeatBalanceManager: SearchWindow5DataFile: Construction='//TRIM(DesiredConstructionName)//
                //            & ' from the Window5 data file cannot be used: it has Frame Edge-of-Glass Conduction Ratio < 1.0')
                //          ErrorsFound = .TRUE.
                //        END IF
                if (FrameSolAbsorp < 0.0 || FrameSolAbsorp > 1.0) {
                    ShowSevereError(state,
                                    "HeatBalanceManager: SearchWindow5DataFile: Construction=" + DesiredConstructionName +
                                        " from the Window5 data file cannot be used: it has Frame Solar Absorptance < 0.0 or > 1.0");
                    ErrorsFound = true;
                }
                if (FrameEmis <= 0.0 || FrameEmis >= 1.0) {
                    ShowSevereError(
                        state,
                        format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used: it has "
                               "Frame Emissivity <= 0.0 or >= 1.0",
                               DesiredConstructionName));
                    ErrorsFound = true;
                }
            }
            FrameWidth *= 0.001;
            FrameProjectionOut *= 0.001;
            FrameProjectionIn *= 0.001;
            FileLineCount += 11;

            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000;
            }
            ++FileLineCount;

            // Divider data for each glazing system
            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                NextLine = W5DataFile.readLine();
                if (NextLine.eof) {
                    goto Label1000;
                }
                ++FileLineCount;

                const bool dividerReadSucceeded = readList(NextLine.data.substr(19),
                                                           DividerWidth(IGlSys),
                                                           DividerProjectionOut(IGlSys),
                                                           DividerProjectionIn(IGlSys),
                                                           DividerConductance(IGlSys),
                                                           DivEdgeToCenterGlCondRatio(IGlSys),
                                                           DividerSolAbsorp(IGlSys),
                                                           DividerVisAbsorp(IGlSys),
                                                           DividerEmis(IGlSys),
                                                           DividerType(IGlSys),
                                                           HorDividers(IGlSys),
                                                           VertDividers(IGlSys));
                if (!dividerReadSucceeded) {
                    ShowSevereError(
                        state,
                        format("HeatBalanceManager: SearchWindow5DataFile: Error in Read of divider data values. For Glazing System={}", IGlSys));
                    ShowContinueError(state,
                                      format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 11, NextLine.data.substr(0, 100)));
                    ErrorsFound = true;
                }
                uppercase(DividerType(IGlSys));
                if (DividerWidth(IGlSys) > 0.0) {
                    if (HorDividers(IGlSys) == 0 && VertDividers(IGlSys) == 0) {
                        ShowSevereError(
                            state,
                            format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used:",
                                   DesiredConstructionName));
                        ShowContinueError(
                            state, format("glazing system {} has a divider but number of horizontal and vertical divider elements = 0", IGlSys));
                        ErrorsFound = true;
                    }
                    if (DividerConductance(IGlSys) <= 0.0) {
                        ShowSevereError(
                            state,
                            format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used:",
                                   DesiredConstructionName));
                        ShowContinueError(state, format("glazing system {} has Divider Conductance <= 0.0", IGlSys));
                        ErrorsFound = true;
                    }
                    if (DivEdgeToCenterGlCondRatio(IGlSys) < 1.0) {
                        ShowSevereError(
                            state,
                            format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used:",
                                   DesiredConstructionName));
                        ShowContinueError(state, format("glazing system {} has Divider Edge-Of-Glass Conduction Ratio < 1.0", IGlSys));
                        ErrorsFound = true;
                    }
                    if (DividerSolAbsorp(IGlSys) < 0.0 || DividerSolAbsorp(IGlSys) > 1.0) {
                        ShowSevereError(
                            state,
                            format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used:",
                                   DesiredConstructionName));
                        ShowContinueError(state, format("glazing system {} has Divider Solar Absorptance < 0.0 or > 1.0", IGlSys));
                        ErrorsFound = true;
                    }
                    if (DividerEmis(IGlSys) <= 0.0 || DividerEmis(IGlSys) >= 1.0) {
                        ShowSevereError(
                            state,
                            format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used:",
                                   DesiredConstructionName));
                        ShowContinueError(state, format("glazing system {} has Divider Emissivity <= 0.0 or >= 1.0", IGlSys));
                        ErrorsFound = true;
                    }
                    if (DividerType(IGlSys) != "DIVIDEDLITE" && DividerType(IGlSys) != "SUSPENDED") {
                        ShowSevereError(
                            state,
                            format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used:",
                                   DesiredConstructionName));
                        ShowContinueError(
                            state,
                            format("glazing system {} has Divider Type = {}; it should be DIVIDEDLITE or SUSPENDED.", IGlSys, DividerType(IGlSys)));
                        ErrorsFound = true;
                    }
                }
                DividerWidth(IGlSys) *= 0.001;
                if (DividerType(IGlSys) == "DIVIDEDLITE") {
                    DividerProjectionOut(IGlSys) *= 0.001;
                    DividerProjectionIn(IGlSys) *= 0.001;
                } else {
                    DividerProjectionOut(IGlSys) = 0.0;
                    DividerProjectionIn(IGlSys) = 0.0;
                }
            }

            if (ErrorsFound) {
                ShowFatalError(state,
                               format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used because "
                                      "of above errors",
                                      DesiredConstructionName));
            }

            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                NGaps(IGlSys) = NGlass(IGlSys) - 1;
            }

            // Glass objects
            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000;
            }
            ++FileLineCount;
            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                for (IGlass = 1; IGlass <= NGlass(IGlSys); ++IGlass) {
                    auto *mat = new Material::MaterialGlass;
                    mat->group = Material::Group::Glass;
                    mat->Name = (NGlSys == 1) ? format("W5:{}:GLASS{}", DesiredConstructionName, NumName(IGlass))
                                              : format("W5:{}:{}:GLASS{}", DesiredConstructionName, NumName(IGlSys), NumName(IGlass));

                    s_mat->materials.push_back(mat);
                    mat->Num = s_mat->materials.isize();
                    s_mat->materialMap.insert_or_assign(mat->Name, mat->Num);

                    MaterNumSysGlass(IGlass, IGlSys) = mat->Num;

                    NextLine = W5DataFile.readLine();
                    ++FileLineCount;

                    readList(NextLine.data.substr(25),
                             mat->Thickness,
                             mat->Conductivity,
                             mat->Trans,
                             mat->ReflectSolBeamFront,
                             mat->ReflectSolBeamBack,
                             mat->TransVis,
                             mat->ReflectVisBeamFront,
                             mat->ReflectVisBeamBack,
                             mat->TransThermal,
                             mat->AbsorpThermalFront,
                             mat->AbsorpThermalBack,
                             LayerName);

                    mat->Thickness *= 0.001;
                    if (mat->Thickness <= 0.0) {
                    }
                    mat->Roughness = Material::SurfaceRoughness::VerySmooth;
                    mat->AbsorpThermal = mat->AbsorpThermalBack;
                    if (mat->Thickness <= 0.0) {
                        ShowSevereError(state,
                                        format("SearchWindow5DataFile: Material=\"{}\" has thickness of 0.0.  Will be set to thickness = .001 but "
                                               "inaccuracies may result.",
                                               mat->Name));
                        ShowContinueError(state, format("Line being read={}", NextLine.data));
                        ShowContinueError(state, format("Thickness field starts at column 26={}", NextLine.data.substr(25)));
                        mat->Thickness = 0.001;
                    }
                }
            }

            // Gap objects
            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000;
            }
            ++FileLineCount;
            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                for (IGap = 1; IGap <= NGaps(IGlSys); ++IGap) {
                    auto *matGas = new Material::MaterialGasMix;
                    matGas->Name = (NGlSys == 1) ? format("W5:{}:GAP{}", DesiredConstructionName, NumName(IGap))
                                                 : format("W5:{}:{}:GAP{}", DesiredConstructionName, NumName(IGlSys), NumName(IGap));
                    s_mat->materials.push_back(matGas);
                    matGas->Num = s_mat->materials.isize();
                    s_mat->materialMap.insert_or_assign(matGas->Name, matGas->Num);

                    MaterNumSysGap(IGap, IGlSys) = matGas->Num;
                    NextLine = W5DataFile.readLine();
                    ++FileLineCount;
                    readList(NextLine.data.substr(23), matGas->Thickness, NumGases(IGap, IGlSys));
                    matGas->Thickness *= 0.001;
                    matGas->Roughness = Material::SurfaceRoughness::MediumRough; // Unused
                }
            }

            // Gap/gas materials are read in multiple passes
            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000; // Exsqueeze me?
            }
            ++FileLineCount;
            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                for (IGap = 1; IGap <= NGaps(IGlSys); ++IGap) {
                    int matNum = MaterNumSysGap(IGap, IGlSys);
                    auto *matGas = dynamic_cast<Material::MaterialGasMix *>(s_mat->materials(matNum));
                    assert(matGas != nullptr);
                    matGas->numGases = NumGases(IGap, IGlSys);
                    for (int IGas = 0; IGas < matGas->numGases; ++IGas) {
                        auto &gas = matGas->gases[IGas];
                        NextLine = W5DataFile.readLine();
                        ++FileLineCount;
                        readList(NextLine.data.substr(19),
                                 GasName(IGas + 1),
                                 matGas->gasFracts[IGas],
                                 gas.wght,
                                 gas.con.c0,
                                 gas.con.c1,
                                 gas.con.c2,
                                 gas.vis.c0,
                                 gas.vis.c1,
                                 gas.vis.c2,
                                 gas.cp.c0,
                                 gas.cp.c1,
                                 gas.cp.c2);
                    }
                    // Nominal resistance of gap at room temperature (based on first gas in mixture)
                    auto const &gas0 = matGas->gases[0];
                    matGas->NominalR = matGas->Thickness / (gas0.con.c0 + gas0.con.c1 * 300.0 + gas0.con.c2 * 90000.0);
                }
            }

            // Construction objects

            // reallocate Construct types
            state.dataHeatBal->TotConstructs += NGlSys;
            state.dataConstruction->Construct.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalRforNominalUCalculation.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalU.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalUBeforeAdjusted.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->CoeffAdjRatio.redimension(state.dataHeatBal->TotConstructs) = 1.0;

            // these Construct arrays dimensioned based on MaxSolidWinLayers
            for (int i = (state.dataHeatBal->TotConstructs - NGlSys + 1); i <= state.dataHeatBal->TotConstructs; ++i) {
                auto &e = state.dataConstruction->Construct(i);
                e.setArraysBasedOnMaxSolidWinLayers(state);
            }

            NextLine = W5DataFile.readLine();
            if (NextLine.eof) {
                goto Label1000;
            }
            ++FileLineCount;

            // When pulling in develop, the following two blocks appear to have been modified in develop,
            //  but removed entirely in this branch.  I'm going to leave them commented.
            // Pre-calculate constants
            // for (IPhi = 1; IPhi <= 10; ++IPhi) {
            //     CosPhiIndepVar(IPhi) = std::cos((IPhi - 1) * 10.0 * Constant::DegToRad);
            //}

            // Pre-calculate constants
            // for (IPhi = 1; IPhi <= 10; ++IPhi) {
            //    Phi = double(IPhi - 1) * 10.0;
            //    CosPhi(IPhi) = std::cos(Phi * Constant::DegToRad);
            //    if (std::abs(CosPhi(IPhi)) < 0.0001) CosPhi(IPhi) = 0.0;
            //}

            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                ConstrNum = state.dataHeatBal->TotConstructs - NGlSys + IGlSys;
                auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                if (IGlSys == 1) {
                    thisConstruct.Name = DesiredConstructionName;
                } else {
                    thisConstruct.Name = DesiredConstructionName + ":2";
                }
                for (loop = 1; loop <= Construction::MaxLayersInConstruct; ++loop) {
                    thisConstruct.LayerPoint(loop) = 0;
                }
                thisConstruct.InsideAbsorpSolar = 0.0;
                thisConstruct.OutsideAbsorpSolar = 0.0;
                thisConstruct.DayltPropPtr = 0;
                thisConstruct.CTFCross.fill(0.0);
                thisConstruct.CTFFlux.fill(0.0);
                thisConstruct.CTFInside.fill(0.0);
                thisConstruct.CTFOutside.fill(0.0);
                thisConstruct.CTFSourceIn.fill(0.0);
                thisConstruct.CTFSourceOut.fill(0.0);
                thisConstruct.CTFTimeStep = 0.0;
                thisConstruct.CTFTSourceOut.fill(0.0);
                thisConstruct.CTFTSourceIn.fill(0.0);
                thisConstruct.CTFTSourceQ.fill(0.0);
                thisConstruct.CTFTUserOut.fill(0.0);
                thisConstruct.CTFTUserIn.fill(0.0);
                thisConstruct.CTFTUserSource.fill(0.0);
                thisConstruct.NumHistories = 0;
                thisConstruct.NumCTFTerms = 0;
                thisConstruct.UValue = 0.0;
                thisConstruct.SourceSinkPresent = false;
                thisConstruct.SolutionDimensions = 0;
                thisConstruct.SourceAfterLayer = 0;
                thisConstruct.TempAfterLayer = 0;
                thisConstruct.ThicknessPerpend = 0.0;
                thisConstruct.AbsDiff = 0.0;
                thisConstruct.AbsDiffBack = 0.0;
                thisConstruct.AbsDiffShade = 0.0;
                thisConstruct.AbsDiffBackShade = 0.0;
                thisConstruct.ShadeAbsorpThermal = 0.0;
                std::fill(thisConstruct.AbsBeamShadeCoef.begin(), thisConstruct.AbsBeamShadeCoef.end(), 0.0);
                thisConstruct.AbsDiffIn = 0.0;
                thisConstruct.AbsDiffOut = 0.0;
                thisConstruct.TransDiff = 0.0;
                thisConstruct.TransDiffVis = 0.0;
                thisConstruct.ReflectSolDiffBack = 0.0;
                thisConstruct.ReflectSolDiffFront = 0.0;
                thisConstruct.ReflectVisDiffBack = 0.0;
                thisConstruct.ReflectVisDiffFront = 0.0;
                std::fill(thisConstruct.TransSolBeamCoef.begin(), thisConstruct.TransSolBeamCoef.end(), 0.0);
                std::fill(thisConstruct.TransVisBeamCoef.begin(), thisConstruct.TransVisBeamCoef.end(), 0.0);
                std::fill(thisConstruct.ReflSolBeamFrontCoef.begin(), thisConstruct.ReflSolBeamFrontCoef.end(), 0.0);
                std::fill(thisConstruct.ReflSolBeamBackCoef.begin(), thisConstruct.ReflSolBeamBackCoef.end(), 0.0);
                thisConstruct.W5FrameDivider = 0;
                thisConstruct.TotLayers = NGlass(IGlSys) + NGaps(IGlSys);
                thisConstruct.TotGlassLayers = NGlass(IGlSys);
                thisConstruct.TotSolidLayers = NGlass(IGlSys);

                for (int Layer = 1; Layer <= state.dataHeatBal->MaxSolidWinLayers; ++Layer) {
                    std::fill(thisConstruct.AbsBeamCoef(Layer).begin(), thisConstruct.AbsBeamCoef(Layer).end(), 0.0);
                    std::fill(thisConstruct.AbsBeamBackCoef(Layer).begin(), thisConstruct.AbsBeamBackCoef(Layer).end(), 0.0);
                }

                for (IGlass = 1; IGlass <= NGlass(IGlSys); ++IGlass) {
                    thisConstruct.LayerPoint(2 * IGlass - 1) = MaterNumSysGlass(IGlass, IGlSys);
                    if (IGlass < NGlass(IGlSys)) {
                        thisConstruct.LayerPoint(2 * IGlass) = MaterNumSysGap(IGlass, IGlSys);
                    }
                }

                thisConstruct.OutsideRoughness = Material::SurfaceRoughness::VerySmooth;
                thisConstruct.InsideAbsorpThermal = s_mat->materials(thisConstruct.LayerPoint(thisConstruct.TotLayers))->AbsorpThermalBack;
                thisConstruct.OutsideAbsorpThermal = s_mat->materials(thisConstruct.LayerPoint(1))->AbsorpThermalFront;
                thisConstruct.TypeIsWindow = true;
                thisConstruct.FromWindow5DataFile = true;
                thisConstruct.W5FileGlazingSysHeight = WinHeight(IGlSys);
                thisConstruct.W5FileGlazingSysWidth = WinWidth(IGlSys);

                thisConstruct.W5FileMullionOrientation = static_cast<DataWindowEquivalentLayer::Orientation>(
                    getEnumValue(DataWindowEquivalentLayer::orientationNamesUC, MullionOrientation));

                thisConstruct.W5FileMullionWidth = MullionWidth;

                // Fill Construct with system transmission, reflection and absorption properties

                NextLine = W5DataFile.readLine();
                if (NextLine.eof) {
                    goto Label1000;
                }
                ++FileLineCount;
                if (IGlSys == 1) {
                    NextLine = W5DataFile.readLine();
                    if (NextLine.eof) {
                        goto Label1000;
                    }
                    ++FileLineCount;
                }
                NextLine = W5DataFile.readLine();
                if (NextLine.eof) {
                    goto Label1000;
                }
                ++FileLineCount;
                if (!readItem(NextLine.data.substr(5), TsolTemp)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of TSol values.");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount, NextLine.data.substr(0, 100)));
                    ErrorsFound = true;
                } else if (any_lt(TsolTemp, 0.0) || any_gt(TsolTemp, 1.0)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of TSol values. (out of range [0,1])");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount, NextLine.data.substr(0, 100)));
                    ErrorsFound = true;
                }

                for (IGlass = 1; IGlass <= NGlass(IGlSys); ++IGlass) {
                    NextLine = W5DataFile.readLine();
                    ++FileLineCount;
                    if (!readItem(NextLine.data.substr(5), AbsSolTemp(IGlass, _))) {
                        ShowSevereError(state,
                                        format("HeatBalanceManager: SearchWindow5DataFile: Error in Read of AbsSol values. For Glass={}", IGlass));
                        ShowContinueError(state,
                                          format("Line (~{}) in error (first 100 characters)={}", FileLineCount, NextLine.data.substr(0, 100)));
                        ErrorsFound = true;
                    } else if (any_lt(AbsSolTemp(IGlass, _), 0.0) || any_gt(AbsSolTemp(IGlass, _), 1.0)) {
                        ShowSevereError(
                            state,
                            format("HeatBalanceManager: SearchWindow5DataFile: Error in Read of AbsSol values. (out of range [0,1]) For Glass={}",
                                   IGlass));
                        ShowContinueError(state,
                                          format("Line (~{}) in error (first 100 characters)={}", FileLineCount, NextLine.data.substr(0, 100)));
                        ErrorsFound = true;
                    }
                }
                for (ILine = 1; ILine <= 5; ++ILine) {
                    NextLine = W5DataFile.readLine();
                    DataLine(ILine) = NextLine.data;
                }

                if (!readItem(DataLine(1).substr(5), RfsolTemp)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of RfSol values.");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 1, DataLine(1).substr(0, 100)));
                    ErrorsFound = true;
                } else if (any_lt(RfsolTemp, 0.0) || any_gt(RfsolTemp, 1.0)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of RfSol values. (out of range [0,1])");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 1, DataLine(1).substr(0, 100)));
                    ErrorsFound = true;
                }

                if (!readItem(DataLine(2).substr(5), RbsolTemp)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of RbSol values.");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 2, DataLine(2).substr(0, 100)));
                    ErrorsFound = true;
                } else if (any_lt(RbsolTemp, 0.0) || any_gt(RbsolTemp, 1.0)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of RbSol values. (out of range [0,1])");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 2, DataLine(2).substr(0, 100)));
                    ErrorsFound = true;
                }
                if (!readItem(DataLine(3).substr(5), TvisTemp)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of Tvis values.");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 3, DataLine(3).substr(0, 100)));
                    ErrorsFound = true;
                } else if (any_lt(TvisTemp, 0.0) || any_gt(TvisTemp, 1.0)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of Tvis values. (out of range [0,1])");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 3, DataLine(3).substr(0, 100)));
                    ErrorsFound = true;
                }
                if (!readItem(DataLine(4).substr(5), RfvisTemp)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of Rfvis values.");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 4, DataLine(4).substr(0, 100)));
                    ErrorsFound = true;
                } else if (any_lt(RfvisTemp, 0.0) || any_gt(RfvisTemp, 1.0)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of Rfvis values. (out of range [0,1])");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 4, DataLine(4).substr(0, 100)));
                    ErrorsFound = true;
                }
                if (!readItem(DataLine(5).substr(5), RbvisTemp)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of Rbvis values.");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 5, DataLine(5).substr(0, 100)));
                    ErrorsFound = true;
                } else if (any_lt(RbvisTemp, 0.0) || any_gt(RbvisTemp, 1.0)) {
                    ShowSevereError(state, "HeatBalanceManager: SearchWindow5DataFile: Error in Read of Rbvis values. (out of range [0,1])");
                    ShowContinueError(state, format("Line (~{}) in error (first 100 characters)={}", FileLineCount + 5, DataLine(5).substr(0, 100)));
                    ErrorsFound = true;
                }
                FileLineCount += 5;

                if (ErrorsFound) {
                    ShowFatalError(
                        state,
                        format("HeatBalanceManager: SearchWindow5DataFile: Construction={} from the Window5 data file cannot be used because "
                               "of above errors",
                               DesiredConstructionName));
                }

                for (int iPhi = 0; iPhi < Window::numPhis; ++iPhi) {
                    Tsol[iPhi] = TsolTemp(iPhi + 1);
                    Tvis[iPhi] = TvisTemp(iPhi + 1);
                    Rfsol[iPhi] = RfsolTemp(iPhi + 1);
                    Rbsol[iPhi] = RbsolTemp(iPhi + 1);
                    Rfvis[iPhi] = RfvisTemp(iPhi + 1);
                    Rbvis[iPhi] = RbvisTemp(iPhi + 1);
                }

                for (IGlass = 1; IGlass <= NGlass(IGlSys); ++IGlass) {
                    for (int iPhi = 0; iPhi < Window::numPhis; ++iPhi) {
                        AbsSol(IGlass)[iPhi] = AbsSolTemp(IGlass, iPhi + 1);
                    }
                }

                // Hemis
                thisConstruct.TransDiff = TsolTemp(11);
                thisConstruct.TransDiffVis = TvisTemp(11);
                thisConstruct.ReflectSolDiffFront = RfsolTemp(11);
                thisConstruct.ReflectSolDiffBack = RbsolTemp(11);
                thisConstruct.ReflectVisDiffFront = RfvisTemp(11);
                thisConstruct.ReflectVisDiffBack = RbvisTemp(11);

                // Using pre-calculated/hard-coded cosPhis in this module is okay.  Shrug.
                Window::W5LsqFit(Window::cosPhis, Tsol, thisConstruct.TransSolBeamCoef);
                Window::W5LsqFit(Window::cosPhis, Tvis, thisConstruct.TransVisBeamCoef);
                Window::W5LsqFit(Window::cosPhis, Rfsol, thisConstruct.ReflSolBeamFrontCoef);
                for (IGlass = 1; IGlass <= NGlass(IGlSys); ++IGlass) {
                    Window::W5LsqFit(Window::cosPhis, AbsSol(IGlass), thisConstruct.AbsBeamCoef(IGlass));
                }

                // For comparing fitted vs. input distribution in incidence angle
                for (int iPhi = 0; iPhi < Window::numPhis; ++iPhi) {
                    tsolFit[iPhi] = Window::POLYF(Window::cosPhis[iPhi], thisConstruct.TransSolBeamCoef);
                    tvisFit[iPhi] = Window::POLYF(Window::cosPhis[iPhi], thisConstruct.TransVisBeamCoef);
                    rfsolFit[iPhi] = Window::POLYF(Window::cosPhis[iPhi], thisConstruct.ReflSolBeamFrontCoef);
                    for (IGlass = 1; IGlass <= NGlass(IGlSys); ++IGlass) {
                        solabsFit(IGlass)[iPhi] = Window::POLYF(Window::cosPhis[iPhi], thisConstruct.AbsBeamCoef(IGlass));
                    }
                }
                // end

                // NominalRforNominalUCalculation of this construction (actually the total resistance of all of its layers; gas layer
                // conductivity here ignores convective effects in gap.)
                state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum) = 0.0;
                for (loop = 1; loop <= NGlass(IGlSys) + NGaps(IGlSys); ++loop) {
                    int matNum = thisConstruct.LayerPoint(loop);
                    auto const *matBase = s_mat->materials(matNum);
                    assert(matBase != nullptr);
                    if (matBase->group == Material::Group::Glass) {
                        state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum) += matBase->Thickness / matBase->Conductivity;
                    } else if (matBase->group == Material::Group::Gas || matBase->group == Material::Group::GasMixture) {
                        auto const *matGas = dynamic_cast<Material::MaterialGasMix const *>(matBase);
                        assert(matGas != nullptr);

                        // If mixture, use conductivity of first gas in mixture
                        state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum) +=
                            matGas->Thickness / (matGas->gases[0].con.c0 + matGas->gases[0].con.c1 * 300.0 + matGas->gases[0].con.c2 * 90000.0);
                    }
                }

            } // End of loop over glazing systems

            // WindowFrameAndDivider objects

            TotFrameDividerPrev = state.dataHeatBal->TotFrameDivider;
            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                if (FrameWidth > 0.0 || DividerWidth(IGlSys) > 0.0) {
                    ++state.dataHeatBal->TotFrameDivider;
                    state.dataConstruction->Construct(state.dataHeatBal->TotConstructs - NGlSys + IGlSys).W5FrameDivider =
                        state.dataHeatBal->TotFrameDivider;
                }
            }

            if (state.dataHeatBal->TotFrameDivider > TotFrameDividerPrev) {
                state.dataSurface->FrameDivider.redimension(state.dataHeatBal->TotFrameDivider);
            }

            for (IGlSys = 1; IGlSys <= NGlSys; ++IGlSys) {
                if (FrameWidth > 0.0 || DividerWidth(IGlSys) > 0.0) {
                    FrDivNum = state.dataConstruction->Construct(state.dataHeatBal->TotConstructs - NGlSys + IGlSys).W5FrameDivider;
                    state.dataSurface->FrameDivider(FrDivNum).FrameWidth = FrameWidth;
                    state.dataSurface->FrameDivider(FrDivNum).FrameProjectionOut = FrameProjectionOut;
                    state.dataSurface->FrameDivider(FrDivNum).FrameProjectionIn = FrameProjectionIn;
                    state.dataSurface->FrameDivider(FrDivNum).FrameConductance = FrameConductance;
                    state.dataSurface->FrameDivider(FrDivNum).FrEdgeToCenterGlCondRatio = FrEdgeToCenterGlCondRatio;
                    state.dataSurface->FrameDivider(FrDivNum).FrameSolAbsorp = FrameSolAbsorp;
                    state.dataSurface->FrameDivider(FrDivNum).FrameVisAbsorp = FrameVisAbsorp;
                    state.dataSurface->FrameDivider(FrDivNum).FrameEmis = FrameEmis;
                    state.dataSurface->FrameDivider(FrDivNum).FrameEdgeWidth = 0.06355; // 2.5 in

                    state.dataSurface->FrameDivider(FrDivNum).MullionOrientation = static_cast<DataWindowEquivalentLayer::Orientation>(
                        getEnumValue(DataWindowEquivalentLayer::orientationNamesUC, MullionOrientation));

                    if (Util::SameString(DividerType(IGlSys), "DividedLite")) {
                        state.dataSurface->FrameDivider(FrDivNum).DividerType = DataSurfaces::FrameDividerType::DividedLite;
                    } else if (Util::SameString(DividerType(IGlSys), "Suspended")) {
                        state.dataSurface->FrameDivider(FrDivNum).DividerType = DataSurfaces::FrameDividerType::Suspended;
                    }
                    state.dataSurface->FrameDivider(FrDivNum).DividerWidth = DividerWidth(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).HorDividers = HorDividers(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).VertDividers = VertDividers(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).DividerProjectionOut = DividerProjectionOut(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).DividerProjectionIn = DividerProjectionIn(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).DividerConductance = DividerConductance(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).DivEdgeToCenterGlCondRatio = DivEdgeToCenterGlCondRatio(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).DividerSolAbsorp = DividerSolAbsorp(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).DividerVisAbsorp = DividerVisAbsorp(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).DividerEmis = DividerEmis(IGlSys);
                    state.dataSurface->FrameDivider(FrDivNum).DividerEdgeWidth = 0.06355; // 2.5 in
                    if (NGlSys == 1) {
                        state.dataSurface->FrameDivider(FrDivNum).Name = "W5:" + DesiredConstructionName;
                    } else {
                        state.dataSurface->FrameDivider(FrDivNum).Name = "W5:" + DesiredConstructionName + ':' + NumName(IGlSys);
                    }
                }
            }

            if (FrameWidth > 0.0 && DividerWidth(1) > 0.0) {
                DisplayString(state, "--Construction and associated frame and divider found");
            } else if (FrameWidth > 0.0) {
                DisplayString(state, "--Construction and associated frame found");
            } else if (DividerWidth(1) > 0.0) {
                DisplayString(state, "--Construction and associated divider found");
            } else {
                DisplayString(state, "--Construction without frame or divider found");
            }
        }

        return;

    Label1000:;
        EOFonFile = true;
    }

    void SetStormWindowControl(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   Jan 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Sets the storm window flag for each window, which is:
        //  -1: if storm window is not applicable (this will always be the value for interior
        //        windows since storm windows can only be applied to exterior windows
        //   0: if the window has a storm window but it is off
        //   1: if the window has a storm window and it is on

        // A "storm window" is a single layer of exterior glass separated from the main window by air gap.
        // Whether the storm window is in place is determined by the following values, which
        // which are specified in the Storm Window object for the window:
        //  -Month that Storm Window Is Put On
        //  -Day of Month that Storm Window Is Put On
        //  -Month that Storm Window Is Taken Off
        //  -Day of Month that Storm Window Is Taken Off

        int StormWinFlag; // Storm window flag; this routine sets the following values:
        //   0: if the storm window is off this time step
        //   1: if the storm window is on this time step

        state.dataHeatBal->StormWinChangeThisDay = false;

        for (int StormWinNum = 1; StormWinNum <= state.dataSurface->TotStormWin; ++StormWinNum) {
            int SurfNum = state.dataSurface->StormWindow(StormWinNum).BaseWindowNum;
            state.dataSurface->SurfWinStormWinFlagPrevDay(SurfNum) = state.dataSurface->SurfWinStormWinFlag(SurfNum);
            int DateOff = state.dataSurface->StormWindow(StormWinNum).DateOff - 1;
            // Note: Dateon = Dateoff is not allowed and will have produced an error in getinput.
            if (DateOff == 0) {
                DateOff = 366;
            }
            if (General::BetweenDates(state.dataEnvrn->DayOfYear_Schedule, state.dataSurface->StormWindow(StormWinNum).DateOn, DateOff)) {
                StormWinFlag = 1;
            } else {
                StormWinFlag = 0;
            }
            state.dataSurface->SurfWinStormWinFlag(SurfNum) = StormWinFlag;
            if (state.dataGlobal->BeginSimFlag) {
                state.dataSurface->SurfWinStormWinFlagPrevDay(SurfNum) = StormWinFlag;
            }
            if (state.dataSurface->SurfWinStormWinFlag(SurfNum) != state.dataSurface->SurfWinStormWinFlagPrevDay(SurfNum)) {
                state.dataHeatBal->StormWinChangeThisDay = true;
            }
        }
    }

    void CreateFCfactorConstructions(EnergyPlusData &state,
                                     int &ConstrNum,   // Counter for Constructions
                                     bool &ErrorsFound // If errors found in input
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Tianzhen Hong
        //       DATE WRITTEN   July 2009

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine goes through each construction defined with Ffactor or Cfactor method,
        // and creates a construction (concrete + insulation) used in the heat transfer calculation.
        // This subroutine only gets called once in the GetConstructionData subroutine

        // ASHRAE Handbook Fundamental 2005
        // Thermal resistance of the inside air film, m2.K/W. Average of 0.16 (heat flow down) and 0.11 (heat flow up)
        Real64 constexpr Rfilm_in(0.135);
        // Thermal resistance of the outside air film used in calculating the Ffactor, m2.K/W. 0.17/5.678
        Real64 constexpr Rfilm_out(0.03);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ConstructNumAlpha;             // Number of construction alpha names being passed
        int DummyNumProp;                  // dummy variable for properties being passed
        int IOStat;                        // IO Status when calling get input subroutine
        Array1D_string ConstructAlphas(1); // Construction Alpha names defined
        Array1D<Real64> DummyProps(4);     // Temporary array to transfer construction properties

        int TotFfactorConstructs; // Number of slabs-on-grade or underground floor constructions defined with F factors
        int TotCfactorConstructs; // Number of underground wall constructions defined with C factors

        Real64 Ffactor;          // Ffactor in W/m-K, applies to deltaT of outside - indoor air temperature
        Real64 Cfactor;          // Cfactor in W/m2-K, does not include soil or air films
        Real64 Area;             // floor area in m2
        Real64 PerimeterExposed; // perimeter exposed in m
        Real64 Height;           // Height of the underground wall in m

        Real64 Reff;          // Effective thermal resistance, m2.K/W
        Real64 Rcon;          // Concrete layer thermal resistance, m2.K/W
        Real64 Rfic;          // Thermal resistance of the fictitious material, m2.K/W
        Real64 Rsoilequ;      // Effective R-value of soil for underground walls
        int iFCConcreteLayer; // Layer pointer to the materials array

        auto &s_mat = state.dataMaterial;
        // First get the concrete layer
        iFCConcreteLayer = Material::GetMaterialNum(state, "~FC_CONCRETE");
        Rcon = s_mat->materials(iFCConcreteLayer)->Resistance;

        // Count number of constructions defined with Ffactor or Cfactor method
        TotFfactorConstructs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:FfactorGroundFloor");
        TotCfactorConstructs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:CfactorUndergroundWall");

        if (TotFfactorConstructs > 0) {
            state.dataHeatBal->NoFfactorConstructionsUsed = false;
        }

        if (TotCfactorConstructs > 0) {
            state.dataHeatBal->NoCfactorConstructionsUsed = false;
        }

        // First create ground floor constructions defined with F factor method if any
        state.dataHeatBalMgr->CurrentModuleObject = "Construction:FfactorGroundFloor";

        // Loop through all constructs defined with Ffactor method
        for (int Loop = 1; Loop <= TotFfactorConstructs; ++Loop) {

            // Get the object names for each construction from the input processor
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     Loop,
                                                                     ConstructAlphas,
                                                                     ConstructNumAlpha,
                                                                     DummyProps,
                                                                     DummyNumProp,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataHeatBalMgr->UniqueConstructNames,
                                                         ConstructAlphas(1),
                                                         state.dataHeatBalMgr->CurrentModuleObject,
                                                         state.dataIPShortCut->cAlphaFieldNames(1),
                                                         ErrorsFound)) {
                continue;
            }

            ++ConstrNum;

            auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            thisConstruct.Name = ConstructAlphas(1);
            thisConstruct.TypeIsFfactorFloor = true;

            Ffactor = DummyProps(1);
            Area = DummyProps(2);
            PerimeterExposed = DummyProps(3);

            thisConstruct.Area = Area;
            thisConstruct.PerimeterExposed = PerimeterExposed;
            thisConstruct.FFactor = Ffactor;

            if (Ffactor <= 0.0) {
                ShowSevereError(state,
                                format("{}=\"{}\" has {} <= 0.0, must be > 0.0.",
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       ConstructAlphas(1),
                                       state.dataIPShortCut->cNumericFieldNames(1)));
                ShowContinueError(state, format("Entered value=[{:.2R}]", Ffactor));
                ErrorsFound = true;
            }

            if (Area <= 0.0) {
                ShowSevereError(state,
                                format("{}=\"{}\" has {} <= 0.0, must be > 0.0.",
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       ConstructAlphas(1),
                                       state.dataIPShortCut->cNumericFieldNames(2)));
                ShowContinueError(state, format("Entered value=[{:.2R}]", Area));
                ErrorsFound = true;
            }

            if (PerimeterExposed < 0.0) {
                ShowSevereError(state,
                                format("{}=\"{}\" has {} <= 0.0, must be > 0.0.",
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       ConstructAlphas(1),
                                       state.dataIPShortCut->cNumericFieldNames(3)));
                ShowContinueError(state, format("Entered value=[{:.2R}]", PerimeterExposed));
                ErrorsFound = true;
            }

            // The construction has two layers which have been created in GetMaterialData
            thisConstruct.TotLayers = 2;

            // The concrete is the inside layer
            thisConstruct.LayerPoint(2) = iFCConcreteLayer;

            // The fictitious insulation is the outside layer
            thisConstruct.LayerPoint(1) = Material::GetMaterialNum(state, format("~FC_INSULATION_{}", Loop));

            // Calculate the thermal resistance of the fictitious insulation layer
            // effective thermal resistance excludes inside and outside air films
            if (PerimeterExposed > 0.0) {
                Reff = Area / (PerimeterExposed * Ffactor) - Rfilm_in - Rfilm_out;
            } else { // PerimeterExposed = 0 for underground floor, assume R-1000 (IP)
                Reff = 177.0;
            }

            Rfic = Reff - Rcon;
            if (Rfic <= 0.0) {
                ShowSevereError(
                    state,
                    format("{}=\"{}\" has calculated R value <= 0.0, must be > 0.0.", state.dataHeatBalMgr->CurrentModuleObject, ConstructAlphas(1)));
                ShowContinueError(state, format("Calculated value=[{:.2R}] Check definition.", Rfic));
                ErrorsFound = true;
            }

            auto *mat = s_mat->materials(thisConstruct.LayerPoint(1));
            mat->Resistance = mat->NominalR = Rfic;

            // excluding thermal resistance of inside or outside air film
            // 1/Reff gets reported as the "U-Factor no Film" in the summary report Envelope Summary | Opaque Exterior
            state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum) = Reff;
        }

        // Then create underground wall constructions defined with C factor method if any
        state.dataHeatBalMgr->CurrentModuleObject = "Construction:CfactorUndergroundWall";

        for (int Loop = 1; Loop <= TotCfactorConstructs; ++Loop) { // Loop through all constructs defined with Ffactor method

            // Get the object names for each construction from the input processor
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHeatBalMgr->CurrentModuleObject,
                                                                     Loop,
                                                                     ConstructAlphas,
                                                                     ConstructNumAlpha,
                                                                     DummyProps,
                                                                     DummyNumProp,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataHeatBalMgr->UniqueConstructNames,
                                                         ConstructAlphas(1),
                                                         state.dataHeatBalMgr->CurrentModuleObject,
                                                         state.dataIPShortCut->cAlphaFieldNames(1),
                                                         ErrorsFound)) {
                continue;
            }

            ++ConstrNum;

            auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            thisConstruct.Name = ConstructAlphas(1);
            thisConstruct.TypeIsCfactorWall = true;

            Cfactor = DummyProps(1);
            Height = DummyProps(2);

            thisConstruct.Height = Height;
            thisConstruct.CFactor = Cfactor;

            if (Cfactor <= 0.0) {
                ShowSevereError(state,
                                format("{} {} has {} <= 0.0, must be > 0.0.",
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       ConstructAlphas(1),
                                       state.dataIPShortCut->cNumericFieldNames(1)));
                ShowContinueError(state, format("Entered value=[{:.2R}]", Cfactor));
                ErrorsFound = true;
            }

            if (Height <= 0.0) {
                ShowSevereError(state,
                                format("{} {} has {} <= 0.0, must be > 0.0.",
                                       state.dataHeatBalMgr->CurrentModuleObject,
                                       ConstructAlphas(1),
                                       state.dataIPShortCut->cNumericFieldNames(2)));
                ShowContinueError(state, format("Entered value=[{:.2R}]", Height));
                ErrorsFound = true;
            }

            // The construction has two layers which have been created in GetMaterialData
            thisConstruct.TotLayers = 2;

            // The concrete is the inside layer
            thisConstruct.LayerPoint(2) = iFCConcreteLayer;

            // The fictitious insulation is the outside layer
            thisConstruct.LayerPoint(1) = Material::GetMaterialNum(state, format("~FC_INSULATION_{}", Loop + TotFfactorConstructs));

            // CR 8886 Rsoil should be in SI unit. From ASHRAE 90.1-2010 SI
            if (Height <= 0.25) {
                Rsoilequ = 0.12; // m2K/W
            } else if (Height >= 2.5) {
                Rsoilequ = 0.92;
            } else { // regression from ASHRAE 90.1-2010 SI TABLE C6.10.1 Effective R-Value of Soil, R2 = 0.9967
                Rsoilequ = 0.0607 + 0.3479 * Height;
            }

            // effective thermal resistance excludes inside and outside air films
            Reff = 1.0 / Cfactor + Rsoilequ; // Cfactor does not include air films

            Rfic = Reff - Rcon;
            if (Rfic <= 0) {
                ShowSevereError(
                    state,
                    format("{}=\"{}\" has calculated R value <= 0.0, must be > 0.0.", state.dataHeatBalMgr->CurrentModuleObject, ConstructAlphas(1)));
                ShowContinueError(state, format("Calculated value=[{:.2R}] Check definition.", Rfic));
                ErrorsFound = true;
            }

            auto *mat = s_mat->materials(thisConstruct.LayerPoint(1));
            mat->Resistance = mat->NominalR = Rfic;

            // Reff includes the wall itself and soil, but excluding thermal resistance of inside or outside air film
            // 1/Reff gets reported as the "U-Factor no Film" in the summary report Envelope Summary | Opaque Exterior
            state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum) = Reff;
        }
    }

    void CreateAirBoundaryConstructions(EnergyPlusData &state,
                                        int &constrNum,   // Counter for Constructions
                                        bool &errorsFound // If errors found in input
    )
    {
        static constexpr std::string_view routineName = "CreateAirBoundaryConstructions";

        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "Construction:AirBoundary";
        int numAirBoundaryConstructs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (numAirBoundaryConstructs > 0) {
            auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(cCurrentModuleObject);
            if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
                // Cannot imagine how you would have numAirBoundaryConstructs > 0 and yet the instances is empty
                // this would indicate a major problem in the input processor, not a problem here
                // I'll still catch this with errorsFound but I cannot make a unit test for it so excluding the line from coverage
                ShowSevereError(state,
                                format("{}: Somehow getNumObjectsFound was > 0 but epJSON.find found 0", cCurrentModuleObject)); // LCOV_EXCL_LINE
                errorsFound = true;                                                                                              // LCOV_EXCL_LINE
            }
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, thisObjectName};

                state.dataInputProcessing->inputProcessor->markObjectAsUsed(cCurrentModuleObject, thisObjectName);

                if (GlobalNames::VerifyUniqueInterObjectName(
                        state, state.dataHeatBalMgr->UniqueConstructNames, thisObjectName, cCurrentModuleObject, "Name", errorsFound)) {
                    continue;
                }

                ++constrNum;
                auto &thisConstruct = state.dataConstruction->Construct(constrNum);

                thisConstruct.Name = Util::makeUPPER(thisObjectName);
                thisConstruct.TypeIsAirBoundary = true;
                thisConstruct.IsUsedCTF = false;

                // Air Exchange Method
                std::string airMethod = "None";
                if (auto found = fields.find("air_exchange_method");
                    found != fields.end()) { // find("x") followed by at("x") is the same lookup twice
                    airMethod = found.value().get<std::string>();
                }
                if (Util::SameString(airMethod, "SimpleMixing")) {
                    thisConstruct.TypeIsAirBoundaryMixing = true;
                    if (auto found = fields.find("simple_mixing_air_changes_per_hour"); found != fields.end()) {
                        thisConstruct.AirBoundaryACH = found.value().get<Real64>();
                    } else {
                        if (!state.dataInputProcessing->inputProcessor->getDefaultValue(
                                state, cCurrentModuleObject, "simple_mixing_air_changes_per_hour", thisConstruct.AirBoundaryACH)) {
                            errorsFound = true;
                        }
                    }

                    if (auto found = fields.find("simple_mixing_schedule_name"); found != fields.end()) {
                        std::string schedName = found.value().get<std::string>(); // .get<std::string>() creates and returns a new string, no &
                        if ((thisConstruct.airBoundaryMixingSched = Sched::GetSchedule(state, Util::makeUPPER(schedName))) == nullptr) {
                            ShowSevereItemNotFound(state, eoh, "Simple Mixing Schedule Name", schedName);
                            errorsFound = true;
                        }
                    } else {
                        thisConstruct.airBoundaryMixingSched =
                            Sched::GetScheduleAlwaysOn(state); // Not an availability manager, but defaults to constant-1.0
                    }
                }
            }
        }
    }

    void GetScheduledSurfaceGains(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   June 2013

        // PURPOSE OF THIS SUBROUTINE:
        // Loads scheduled surface gains for solar incident on interior side of the surfaces and absorbed solar energy in
        // window layers

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetScheduledSurfaceGains: ");
        static constexpr std::string_view routineName = "GetScheduledSurfaceGains";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumArgs;
        int NumAlpha;
        int NumNumeric;
        int Loop;
        int IOStat;
        int SurfNum;
        int ConstrNum;

        //-----------------------------------------------------------------------
        //                SurfaceProperty:SolarIncidentInside
        //-----------------------------------------------------------------------
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "SurfaceProperty:SolarIncidentInside";

        // Check if IDD definition is correct
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumeric);
        if (NumAlpha != 4) {
            ShowSevereError(
                state,
                format("{}{}: Object Definition indicates not = 4 Alpha Objects, Number Indicated={}", RoutineName, cCurrentModuleObject, NumAlpha));
            ErrorsFound = true;
        }

        state.dataSurface->TotSurfIncSolSSG = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataSurface->TotSurfIncSolSSG > 0) {
            if (!allocated(state.dataSurface->SurfIncSolSSG)) {
                state.dataSurface->SurfIncSolSSG.allocate(state.dataSurface->TotSurfIncSolSSG);
            }

            for (Loop = 1; Loop <= state.dataSurface->TotSurfIncSolSSG; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         Loop,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlpha,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumeric,
                                                                         IOStat,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         state.dataIPShortCut->lAlphaFieldBlanks,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

                state.dataSurface->SurfIncSolSSG(Loop).Name = state.dataIPShortCut->cAlphaArgs(1);

                // Assign surface number
                SurfNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataSurface->Surface);
                if (SurfNum == 0) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
                    ErrorsFound = true;
                } else {
                    state.dataSurface->SurfIncSolSSG(Loop).SurfPtr = SurfNum;
                    if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                        int repSurfNum = state.dataSurface->Surface(SurfNum).RepresentativeCalcSurfNum;
                        if (repSurfNum != SurfNum) {
                            // Do not use representative surfaces

                            // remove surface from representative constituent list
                            auto &vec = state.dataSurface->Surface(repSurfNum).ConstituentSurfaceNums;
                            vec.erase(std::remove(vec.begin(), vec.end(), SurfNum), vec.end());

                            // reset representative surface number
                            state.dataSurface->Surface(SurfNum).RepresentativeCalcSurfNum = SurfNum;
                        }
                    }
                }

                // Assign construction number
                ConstrNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(3), state.dataConstruction->Construct);
                if (ConstrNum == 0) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3));
                    ErrorsFound = true;
                } else {
                    state.dataSurface->SurfIncSolSSG(Loop).ConstrPtr = ConstrNum;
                }

                if ((state.dataSurface->SurfIncSolSSG(Loop).sched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(4))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4));
                    ErrorsFound = true;
                }
            }
        }

        //-----------------------------------------------------------------------
        //                SurfaceProperty:SolarIncidentInside
        //-----------------------------------------------------------------------
        cCurrentModuleObject = "ComplexFenestrationProperty:SolarAbsorbedLayers";

        state.dataSurface->TotFenLayAbsSSG = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataSurface->TotFenLayAbsSSG > 0) {
            if (!allocated(state.dataSurface->FenLayAbsSSG)) {
                state.dataSurface->FenLayAbsSSG.allocate(state.dataSurface->TotFenLayAbsSSG);
            }

            for (Loop = 1; Loop <= state.dataSurface->TotFenLayAbsSSG; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         Loop,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlpha,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumeric,
                                                                         IOStat,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         state.dataIPShortCut->lAlphaFieldBlanks,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

                state.dataSurface->FenLayAbsSSG(Loop).Name = state.dataIPShortCut->cAlphaArgs(1);

                // Assign surface number
                SurfNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataSurface->Surface);
                if (SurfNum == 0) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
                    ErrorsFound = true;
                } else {
                    state.dataSurface->FenLayAbsSSG(Loop).SurfPtr = SurfNum;
                }

                // Assign construction number
                ConstrNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(3), state.dataConstruction->Construct);
                auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum); // Why is this before the error check?
                if (ConstrNum == 0) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3));
                    ErrorsFound = true;
                } else {
                    state.dataSurface->FenLayAbsSSG(Loop).ConstrPtr = ConstrNum;
                    int NumOfScheduledLayers = NumAlpha - 3;
                    bool NumOfLayersMatch = false;
                    // Check if number of layers in construction matches number of layers in schedule surface gains object
                    if (NumOfScheduledLayers == thisConstruct.TotSolidLayers) {
                        NumOfLayersMatch = true;
                    }

                    if (!NumOfLayersMatch) {
                        ShowSevereError(
                            state,
                            format("{}{}=\"{}, object. Number of scheduled surface gains for each layer does not match number of layers in "
                                   "referenced construction.",
                                   RoutineName,
                                   cCurrentModuleObject,
                                   state.dataIPShortCut->cAlphaArgs(1)));
                        ShowContinueError(state,
                                          format("{} have {} schedule layers and {} have {} layers.",
                                                 state.dataIPShortCut->cAlphaArgs(1),
                                                 NumOfScheduledLayers,
                                                 state.dataIPShortCut->cAlphaArgs(3),
                                                 thisConstruct.TotSolidLayers));
                        ErrorsFound = true;
                    }

                    if (!allocated(state.dataSurface->FenLayAbsSSG(Loop).scheds)) {
                        state.dataSurface->FenLayAbsSSG(Loop).scheds.allocate(NumOfScheduledLayers);
                    }

                    state.dataSurface->FenLayAbsSSG(Loop).NumOfSched = NumOfScheduledLayers;

                    for (int i = 1; i <= NumOfScheduledLayers; ++i) {
                        if ((state.dataSurface->FenLayAbsSSG(Loop).scheds(i) = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(i + 3))) ==
                            nullptr) {
                            ShowSevereItemNotFound(state,
                                                   eoh,
                                                   state.dataIPShortCut->cAlphaFieldNames(NumOfScheduledLayers + 3),
                                                   state.dataIPShortCut->cAlphaArgs(NumOfScheduledLayers + 3));
                            ErrorsFound = true;
                        }
                    }
                }
            }
        }

        // Check if scheduled surface gains are assigined to each surface in every zone.  If not then warning message to user will be
        // issued
        if ((state.dataSurface->TotSurfIncSolSSG > 0) || (state.dataSurface->TotFenLayAbsSSG > 0)) {
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                CheckScheduledSurfaceGains(state, iZone);
            }
        }
    }

    void CheckScheduledSurfaceGains(EnergyPlusData &state, int const ZoneNum) // Zone number for which error check will be performed
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2013

        // PURPOSE OF THIS SUBROUTINE:
        // Check if all surfaces within zone are scheduled with surface gains. If not all surfaces within zone are scheduled,
        // warning message will be issued and program will continue to execute.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SchedPtr; // scheduled surface gains pointer

        bool ZoneUnscheduled = false; // true if all surfaces in the zone are unscheduled
        bool ZoneScheduled = false;   // true if all surfaces in the zone are scheduled

        bool firstZoneSurface = true;
        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int iSurf = thisSpace.HTSurfaceFirst; iSurf <= thisSpace.HTSurfaceLast; ++iSurf) {
                int iConst = state.dataSurface->Surface(iSurf).Construction;
                if (state.dataSurface->Surface(iSurf).Class == DataSurfaces::SurfaceClass::Window) {
                    SchedPtr = SolarShading::WindowScheduledSolarAbs(state, iSurf, iConst);
                } else {
                    SchedPtr = SolarShading::SurfaceScheduledSolarInc(state, iSurf, iConst);
                }
                if (firstZoneSurface) {
                    if (SchedPtr != 0) {
                        ZoneScheduled = true;
                        ZoneUnscheduled = false;
                    } else {
                        ZoneScheduled = false;
                        ZoneUnscheduled = true;
                    }
                    firstZoneSurface = false;
                } else {
                    if (SchedPtr != 0) {
                        ZoneUnscheduled = false;
                    } else {
                        ZoneScheduled = false;
                    }
                }
            }
        }
        if ((!ZoneScheduled) && (!ZoneUnscheduled)) {
            // zone is not scheduled nor unscheduled
            ShowWarningError(state,
                             format("Zone {} does not have all surfaces scheduled with surface gains.", state.dataHeatBal->Zone(ZoneNum).Name));
            ShowContinueError(state,
                              "If at least one surface in the zone is scheduled with surface gains, then all other surfaces within the same zone "
                              "should be scheduled as well.");
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                for (int iSurf = thisSpace.HTSurfaceFirst; iSurf <= thisSpace.HTSurfaceLast; ++iSurf) {
                    int iConst = state.dataSurface->Surface(iSurf).Construction;
                    if (state.dataSurface->Surface(iSurf).Class == DataSurfaces::SurfaceClass::Window) {
                        SchedPtr = SolarShading::WindowScheduledSolarAbs(state, iSurf, iConst);
                    } else {
                        SchedPtr = SolarShading::SurfaceScheduledSolarInc(state, iSurf, iConst);
                    }

                    if (SchedPtr == 0) {
                        ShowContinueError(state, format("Surface {} does not have scheduled surface gains.", state.dataSurface->Surface(iSurf).Name));
                    }
                }
            }
        }
    }

    void CreateTCConstructions(EnergyPlusData &state, [[maybe_unused]] bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Tianzhen Hong
        //       DATE WRITTEN   January 2009

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine goes through each TC master construction and creates a complete series
        // of the slave thermochromic constructions.
        // This subroutine only gets called once in the GetHeatBalanceInput subroutine
        //  after materials, constructions and building geometry data are read.
        auto &s_mat = state.dataMaterial;

        int NumNewConst = 0;
        for (int Loop = 1; Loop <= state.dataHeatBal->TotConstructs; ++Loop) {
            auto &constr = state.dataConstruction->Construct(Loop);

            if (!constr.isTCMaster) {
                continue;
            }

            auto const *matGlassTC = dynamic_cast<Material::MaterialGlassTC const *>(s_mat->materials(constr.TCMasterMatNum));
            assert(matGlassTC != nullptr);
            NumNewConst += matGlassTC->numMatRefs;
        }

        if (NumNewConst == 0) {
            return; // no need to go further
        }

        // Increase Construct() and copy the extra constructions
        state.dataConstruction->Construct.redimension(state.dataHeatBal->TotConstructs + NumNewConst);
        state.dataHeatBal->NominalRforNominalUCalculation.redimension(state.dataHeatBal->TotConstructs + NumNewConst);
        state.dataHeatBal->NominalU.redimension(state.dataHeatBal->TotConstructs + NumNewConst);
        state.dataHeatBal->NominalUBeforeAdjusted.redimension(state.dataHeatBal->TotConstructs + NumNewConst);
        state.dataHeatBal->CoeffAdjRatio.redimension(state.dataHeatBal->TotConstructs + NumNewConst) = 1.0;

        NumNewConst = state.dataHeatBal->TotConstructs;
        for (int Loop = 1; Loop <= state.dataHeatBal->TotConstructs; ++Loop) {
            auto &constr = state.dataConstruction->Construct(Loop);
            if (!constr.isTCMaster) {
                continue;
            }

            auto const *matGlassTC = dynamic_cast<Material::MaterialGlassTC *>(s_mat->materials(constr.TCMasterMatNum));
            assert(matGlassTC != nullptr);

            constr.numTCChildConstrs = matGlassTC->numMatRefs;
            constr.TCChildConstrs.allocate(constr.numTCChildConstrs);

            // The master thermochromic construction uses the first (i.e., lowest temp) glazing
            constr.LayerPoint(constr.TCLayerNum) = matGlassTC->matRefs(1).matNum;
            constr.specTemp = matGlassTC->matRefs(1).specTemp;

            for (int iTC = 1; iTC <= constr.numTCChildConstrs; ++iTC) {
                ++NumNewConst;
                auto &constrNew = state.dataConstruction->Construct(NumNewConst);

                constrNew = constr; // This should be a deep copy
                constrNew.Name = format("{}_TC_{:.0R}", constr.Name, matGlassTC->matRefs(iTC).specTemp);
                constrNew.LayerPoint(constrNew.TCLayerNum) = matGlassTC->matRefs(iTC).matNum;
                constrNew.specTemp = matGlassTC->matRefs(iTC).specTemp;

                constrNew.isTCWindow = true;
                constrNew.isTCMaster = false;
                constrNew.TCMasterConstrNum = Loop;

                constr.TCChildConstrs(iTC).specTemp = matGlassTC->matRefs(iTC).specTemp;
                constr.TCChildConstrs(iTC).constrNum = NumNewConst;
            }
        }
        state.dataHeatBal->TotConstructs = NumNewConst;
    }

    void SetupComplexFenestrationStateInput(EnergyPlusData &state,
                                            int &ConstrNum, // num of construction items thus far
                                            bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2010
        //       MODIFIED       January 2012 (Simon Vidanovic)
        //       MODIFIED       May 2012 (Simon Vidanovic)

        // PURPOSE OF THIS SUBROUTINE:
        // get input for complex fenestration construction

        // METHODOLOGY EMPLOYED:
        // usual GetInput processing.  Matrix input from MatrixDataManager

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view routineName = "SetupComlexFenestrationStateInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        int TotalArgs;  // Number of fields for each GetObjectItem call
        int IOStatus;   // Used in GetObjectItem
        int NumRows;    // temporary size of matrix
        int NumCols;    // temporary size of matrix
        int NBasis;     // temporary number of elements in basis
        int AlphaIndex;
        int NumOfTotalLayers;    // total number of layers in the construction
        int NumOfOpticalLayers;  // number of optical layers in the construction (excluding gasses and gas mixtures)
        int currentOpticalLayer; // current optical layer number.  This is important since optical structures should
        // be loaded only with optical layers

        // When reading Construction:ComplexFenestrationState, there is a call of GetMatrix2D which also uses same
        // variables from DataIPShortCuts.  Since this can cause some errors in reading, it is important
        // to declare local variables for reading Construction:ComplexFenestrationState object(s)
        Array1D_string locAlphaFieldNames;
        Array1D_string locNumericFieldNames;
        Array1D_bool locNumericFieldBlanks;
        Array1D_bool locAlphaFieldBlanks;
        Array1D_string locAlphaArgs;
        Array1D<Real64> locNumericArgs;
        std::string locCurrentModuleObject;

        auto &s_ipsc = state.dataIPShortCut;
        auto &s_mat = state.dataMaterial;

        // Reading WindowThermalModel:Params
        s_ipsc->cCurrentModuleObject = "WindowThermalModel:Params";
        state.dataBSDFWindow->TotThermalModels = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        s_mat->WindowThermalModel.allocate(state.dataBSDFWindow->TotThermalModels);

        for (int Loop = 1; Loop <= state.dataBSDFWindow->TotThermalModels; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     _,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            auto &windowThermalModel = s_mat->WindowThermalModel(Loop);
            windowThermalModel.Name = s_ipsc->cAlphaArgs(1);

            windowThermalModel.SDScalar = s_ipsc->rNumericArgs(1);
            if ((s_ipsc->rNumericArgs(1) < 0.0) || (s_ipsc->rNumericArgs(1) > 1.0)) {
                ShowSevereCustom(
                    state,
                    eoh,
                    format("{} should be >= 0.0 and <= 1.0, entered value = {:.2R}", s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1)));
                ErrorsFound = true;
            }

            windowThermalModel.CalculationStandard =
                static_cast<TARCOGGassesParams::Stdrd>(getEnumValue(TARCOGGassesParams::stdrdNamesUC, s_ipsc->cAlphaArgs(2)));
            windowThermalModel.ThermalModel =
                static_cast<TARCOGParams::TARCOGThermalModel>(getEnumValue(TARCOGParams::thermalModelNamesUC, s_ipsc->cAlphaArgs(3)));
            windowThermalModel.DeflectionModel =
                static_cast<TARCOGParams::DeflectionCalculation>(getEnumValue(TARCOGParams::deflectionCalculationNamesUC, s_ipsc->cAlphaArgs(4)));

            if (windowThermalModel.DeflectionModel == TARCOGParams::DeflectionCalculation::TEMPERATURE) {
                windowThermalModel.VacuumPressureLimit = s_ipsc->rNumericArgs(2);
                if (s_ipsc->rNumericArgs(2) <= 0.0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("{} must be > 0, entered value = {:.2R}", s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2)));
                }

                windowThermalModel.InitialTemperature = s_ipsc->rNumericArgs(3);
                if (s_ipsc->rNumericArgs(3) <= 0.0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("{} must be > 0, entered value = {:.2R}", s_ipsc->cNumericFieldNames(3), s_ipsc->rNumericArgs(3)));
                }

                windowThermalModel.InitialPressure = s_ipsc->rNumericArgs(4);
                if (s_ipsc->rNumericArgs(4) <= 0.0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("{} must be > 0, entered value = {:.2R}", s_ipsc->cNumericFieldNames(4), s_ipsc->rNumericArgs(4)));
                }
            }

        } // DO Loop = 1, TotThermalModels

        // Reading Construction:ComplexFenestrationState
        locCurrentModuleObject = "Construction:ComplexFenestrationState";
        state.dataBSDFWindow->TotComplexFenStates = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, locCurrentModuleObject);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, locCurrentModuleObject, TotalArgs, NumAlphas, NumNumbers);
        if (!allocated(locAlphaFieldNames)) {
            locAlphaFieldNames.allocate(NumAlphas);
        }
        if (!allocated(locNumericFieldNames)) {
            locNumericFieldNames.allocate(NumNumbers);
        }
        if (!allocated(locNumericFieldBlanks)) {
            locNumericFieldBlanks.allocate(NumNumbers);
        }
        if (!allocated(locAlphaFieldBlanks)) {
            locAlphaFieldBlanks.allocate(NumAlphas);
        }
        if (!allocated(locAlphaArgs)) {
            locAlphaArgs.allocate(NumAlphas);
        }
        if (!allocated(locNumericArgs)) {
            locNumericArgs.allocate(NumNumbers);
        }

        state.dataBSDFWindow->FirstBSDF = ConstrNum + 1; // Location of first BSDF construction input (They will be consecutive)
        for (int Loop = 1; Loop <= state.dataBSDFWindow->TotComplexFenStates; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     locCurrentModuleObject,
                                                                     Loop,
                                                                     locAlphaArgs,
                                                                     NumAlphas,
                                                                     locNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     locNumericFieldBlanks,
                                                                     _,
                                                                     locAlphaFieldNames,
                                                                     locNumericFieldNames);

            ErrorObjectHeader eoh{routineName, locCurrentModuleObject, locAlphaArgs(1)};

            if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataHeatBalMgr->UniqueConstructNames,
                                                         locAlphaArgs(1),
                                                         state.dataHeatBalMgr->CurrentModuleObject,
                                                         locAlphaFieldNames(1),
                                                         ErrorsFound)) {
                continue;
            }
            ++ConstrNum;
            auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            // Simon TODO: This is to be confirmed.  If this is just initial value, then we might want to make better guess
            state.dataHeatBal->NominalRforNominalUCalculation(ConstrNum) = 0.1;
            // Simon TODO: If I do not put this, then it is considered that surface is NOT window
            thisConstruct.TransDiff = 0.1; // This is a place holder to flag
            // the construction as a window until
            // the correct value is entered in WindowComplexManager

            // Now override the deraults as appropriate
            thisConstruct.Name = locAlphaArgs(1);

            //    ALLOCATE(Construct(ConstrNum)%BSDFInput)

            // Construct(ConstrNum)%BSDFInput%ThermalConstruction = ThConstNum

            thisConstruct.BSDFInput.BasisType = static_cast<DataBSDFWindow::Basis>(getEnumValue(DataBSDFWindow::basisNamesUC, locAlphaArgs(2)));
            thisConstruct.BSDFInput.BasisSymmetryType =
                static_cast<DataBSDFWindow::BasisSymmetry>(getEnumValue(DataBSDFWindow::basisSymmetryNamesUC, locAlphaArgs(3)));

            // Simon: Assign thermal model number
            thisConstruct.BSDFInput.ThermalModel = Util::FindItemInList(locAlphaArgs(4), s_mat->WindowThermalModel);
            if (thisConstruct.BSDFInput.ThermalModel == 0) {
                ShowSevereItemNotFound(state, eoh, locAlphaFieldNames(4), locAlphaArgs(4));
            }

            // ***************************************************************************************
            // Basis matrix
            // ***************************************************************************************
            thisConstruct.BSDFInput.BasisMatIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(5));
            MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.BasisMatIndex, NumRows, NumCols);
            thisConstruct.BSDFInput.BasisMatNrows = NumRows;
            thisConstruct.BSDFInput.BasisMatNcols = NumCols;

            if (NumCols != 2 && NumCols != 1) {
                ErrorsFound = true;
                ShowSevereCustom(state,
                                 eoh,
                                 format("{} entered value=\"{}\" invalid matrix dimensions.  Basis matrix dimension can only be 2 x 1.",
                                        locAlphaFieldNames(5),
                                        locAlphaArgs(5)));
            }
            thisConstruct.BSDFInput.BasisMat.allocate(NumCols, NumRows);
            MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.BasisMatIndex, thisConstruct.BSDFInput.BasisMat);
            if (thisConstruct.BSDFInput.BasisType == DataBSDFWindow::Basis::WINDOW) {
                WindowComplexManager::CalculateBasisLength(state, thisConstruct.BSDFInput, ConstrNum, thisConstruct.BSDFInput.NBasis);
            }

            // determine number of layers and optical layers
            NumOfTotalLayers = (NumAlphas - 9) / 3;
            thisConstruct.TotLayers = NumOfTotalLayers;

            NumOfOpticalLayers = NumOfTotalLayers / 2 + 1;

            thisConstruct.BSDFInput.NumLayers = NumOfOpticalLayers;
            thisConstruct.BSDFInput.Layer.allocate(NumOfOpticalLayers);

            // check for incomplete field set
            if (mod((NumAlphas - 9), 3) != 0) {
                // throw warning if incomplete field set
                ErrorsFound = true;
                ShowSevereCustom(state, eoh, format("{} is missing some of the layers or/and gaps.", locAlphaArgs(1)));
            }

            if (thisConstruct.BSDFInput.BasisSymmetryType == DataBSDFWindow::BasisSymmetry::None) {
                // Non-Symmetric basis

                NBasis = thisConstruct.BSDFInput.NBasis;

                // *******************************************************************************
                // Solar front transmittance
                // *******************************************************************************
                thisConstruct.BSDFInput.SolFrtTransIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(6));
                MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.SolFrtTransIndex, NumRows, NumCols);
                thisConstruct.BSDFInput.SolFrtTransNrows = NumRows;
                thisConstruct.BSDFInput.SolFrtTransNcols = NumCols;

                if (NumRows != NBasis) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Solar front transmittance matrix \"{}\" is not the same size as it is defined by basis definition. Basis "
                               "size is defined by Matrix:TwoDimension = \"{}\".",
                               locAlphaArgs(6),
                               locAlphaArgs(5)));
                }

                if (NumRows != NumCols) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Solar front transmittance matrix \"{}\" must have the same number of rows and columns.", locAlphaArgs(6)));
                }

                if (thisConstruct.BSDFInput.BasisType == DataBSDFWindow::Basis::Custom) {
                    thisConstruct.BSDFInput.NBasis = NumRows; // For custom basis, no rows in transmittance
                                                              // matrix defines the basis length
                }

                thisConstruct.BSDFInput.SolFrtTrans.allocate(NumCols, NumRows);
                if (thisConstruct.BSDFInput.SolFrtTransIndex == 0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Solar front transmittance Matrix:TwoDimension = \"{}\" is missing from the input file.", locAlphaArgs(6)));
                } else {
                    MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.SolFrtTransIndex, thisConstruct.BSDFInput.SolFrtTrans);
                }

                // *******************************************************************************
                // Solar back reflectance
                // *******************************************************************************
                thisConstruct.BSDFInput.SolBkReflIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(7));
                MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.SolBkReflIndex, NumRows, NumCols);
                thisConstruct.BSDFInput.SolBkReflNrows = NumRows;
                thisConstruct.BSDFInput.SolBkReflNcols = NumCols;

                if (NumRows != NBasis) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Solar back reflectance matrix \"{}\" is not the same size as it is defined by basis definition. Basis size "
                               "is defined by Matrix:TwoDimension = \"{}\".",
                               locAlphaArgs(7),
                               locAlphaArgs(5)));
                }

                if (NumRows != NumCols) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("Solar back reflectance matrix \"{}\" must have the same number of rows and columns.", locAlphaArgs(7)));
                }

                thisConstruct.BSDFInput.SolBkRefl.allocate(NumCols, NumRows);
                if (thisConstruct.BSDFInput.SolBkReflIndex == 0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("Solar back reflectance Matrix:TwoDimension = \"{}\" is missing from the input file.", locAlphaArgs(7)));
                } else {
                    MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.SolBkReflIndex, thisConstruct.BSDFInput.SolBkRefl);
                }

                // *******************************************************************************
                // Visible front transmittance
                // *******************************************************************************
                thisConstruct.BSDFInput.VisFrtTransIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(8));
                MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.VisFrtTransIndex, NumRows, NumCols);
                thisConstruct.BSDFInput.VisFrtTransNrows = NumRows;
                thisConstruct.BSDFInput.VisFrtTransNcols = NumCols;

                if (NumRows != NBasis) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Visible front transmittance matrix \"{}\" is not the same size as it is defined by basis definition. Basis "
                               "size is defined by Matrix:TwoDimension = \"{}\".",
                               locAlphaArgs(8),
                               locAlphaArgs(5)));
                }

                if (NumRows != NumCols) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Visible front transmittance matrix \"{}\" must have the same number of rows and columns.", locAlphaArgs(8)));
                }

                thisConstruct.BSDFInput.VisFrtTrans.allocate(NumCols, NumRows);
                if (thisConstruct.BSDFInput.VisFrtTransIndex == 0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Visible front transmittance Matrix:TwoDimension = \"{}\" is missing from the input file.", locAlphaArgs(8)));
                } else {
                    MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.VisFrtTransIndex, thisConstruct.BSDFInput.VisFrtTrans);
                }

                // *******************************************************************************
                // Visible back reflectance
                // *******************************************************************************
                thisConstruct.BSDFInput.VisBkReflIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(9));
                MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.VisBkReflIndex, NumRows, NumCols);
                thisConstruct.BSDFInput.VisBkReflNrows = NumRows;
                thisConstruct.BSDFInput.VisBkReflNcols = NumCols;

                if (NumRows != NBasis) {
                    ErrorsFound = true;
                    ShowSevereCustom(state,
                                     eoh,
                                     format("Visible back reflectance matrix \"{}\" is not the same size as it is defined by basis definition. Basis "
                                            "size is defined by Matrix:TwoDimension = \"{}\".",
                                            locAlphaArgs(9),
                                            locAlphaArgs(5)));
                }

                if (NumRows != NumCols) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("Visible back reflectance \"{}\" must have the same number of rows and columns.", locAlphaArgs(9)));
                }

                thisConstruct.BSDFInput.VisBkRefl.allocate(NumCols, NumRows);
                if (thisConstruct.BSDFInput.VisBkReflIndex == 0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("Visble back reflectance Matrix:TwoDimension = \"{}\" is missing from the input file.", locAlphaArgs(9)));
                } else {
                    MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.VisBkReflIndex, thisConstruct.BSDFInput.VisBkRefl);
                }

                // ALLOCATE(Construct(ConstrNum)%BSDFInput%Layer(NumOfOpticalLayers))
                for (int Layer = 1; Layer <= thisConstruct.TotLayers; ++Layer) {
                    AlphaIndex = 9 + (Layer * 3) - 2;
                    currentOpticalLayer = int(Layer / 2) + 1;
                    // Material info is contained in the thermal construct
                    thisConstruct.LayerPoint(Layer) = Material::GetMaterialNum(state, locAlphaArgs(AlphaIndex));

                    // Simon: Load only if optical layer
                    if (mod(Layer, 2) != 0) {
                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).MaterialIndex = thisConstruct.LayerPoint(Layer);

                        ++AlphaIndex;
                        // *******************************************************************************
                        // Front absorptance matrix
                        // *******************************************************************************
                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbsIndex =
                            MatrixDataManager::MatrixIndex(state, locAlphaArgs(AlphaIndex));
                        MatrixDataManager::Get2DMatrixDimensions(
                            state, thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbsIndex, NumRows, NumCols);

                        if (NumRows != 1) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Front absorbtance Matrix:TwoDimension = \"{}\" for layer {} must have only one row.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer));
                        }

                        if (NumCols != NBasis) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Front absorbtance Matrix:TwoDimension = \"{}\" for layer {} must have same number of columns "
                                                    "as it is defined by basis matrix."
                                                    "Matrix has {} number of columns, while basis definition specifies {} number of columns.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer,
                                                    NumCols,
                                                    NBasis));
                        }

                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).AbsNcols = NumCols;
                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbs.allocate(NumCols, NumRows);
                        if (thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbsIndex == 0) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Front absorbtance Matrix:TwoDimension = \"{}\" for layer {} is missing from the input file.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer));
                        } else {
                            MatrixDataManager::Get2DMatrix(state,
                                                           thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbsIndex,
                                                           thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbs);
                        }

                        ++AlphaIndex;
                        // *******************************************************************************
                        // Back absorptance matrix
                        // *******************************************************************************
                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbsIndex =
                            MatrixDataManager::MatrixIndex(state, locAlphaArgs(AlphaIndex));
                        MatrixDataManager::Get2DMatrixDimensions(
                            state, thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbsIndex, NumRows, NumCols);

                        if (NumRows != 1) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Back absorbtance Matrix:TwoDimension = \"{}\" for layer {} must have only one row.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer));
                        }

                        if (NumCols != NBasis) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Back absorbtance Matrix:TwoDimension = \"{}\" for layer {} must have same number of columns as "
                                                    "it is defined by basis matrix."
                                                    "Matrix has {} number of columns, while basis definition specifies {} number of columns.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer,
                                                    NumCols,
                                                    NBasis));
                        }

                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbs.allocate(NumCols, NumRows);
                        if (thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbsIndex == 0) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Back absorbtance Matrix:TwoDimension = \"{}\" for layer {} is missing from the input file.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer));
                        } else {
                            MatrixDataManager::Get2DMatrix(state,
                                                           thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbsIndex,
                                                           thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbs);
                        }
                    } // if (Mod(Layer, 2) <> 0) then
                }
            } else {
                // Axisymmetric basis
                NBasis = thisConstruct.BSDFInput.NBasis; // Basis length has already been calculated
                state.dataBSDFWindow->BSDFTempMtrx.allocate(NBasis, 1);

                // *******************************************************************************
                // Solar front transmittance
                // *******************************************************************************
                thisConstruct.BSDFInput.SolFrtTransIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(6));
                MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.SolFrtTransIndex, NumRows, NumCols);
                thisConstruct.BSDFInput.SolFrtTransNrows = NBasis;
                thisConstruct.BSDFInput.SolFrtTransNcols = NBasis;

                if (NumRows != NBasis) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Solar front transmittance matrix \"{}\" is not the same size as it is defined by basis definition. Basis "
                               "size is defined by Matrix:TwoDimension = \"{}\".",
                               locAlphaArgs(6),
                               locAlphaArgs(5)));
                }

                if (NumRows != NumCols) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Solar front transmittance matrix \"{}\" must have the same number of rows and columns.", locAlphaArgs(6)));
                }

                thisConstruct.BSDFInput.SolFrtTrans.allocate(NBasis, NBasis);
                if (thisConstruct.BSDFInput.SolFrtTransIndex == 0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Solar front transmittance Matrix:TwoDimension = \"{}\" is missing from the input file.", locAlphaArgs(6)));
                } else {
                    MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.SolFrtTransIndex, state.dataBSDFWindow->BSDFTempMtrx);

                    thisConstruct.BSDFInput.SolFrtTrans = 0.0;
                    for (int I = 1; I <= NBasis; ++I) {
                        thisConstruct.BSDFInput.SolFrtTrans(I, I) = state.dataBSDFWindow->BSDFTempMtrx(I, 1);
                    }
                }

                // *******************************************************************************
                // Solar back reflectance
                // *******************************************************************************
                thisConstruct.BSDFInput.SolBkReflIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(7));
                MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.SolBkReflIndex, NumRows, NumCols);
                thisConstruct.BSDFInput.SolBkReflNrows = NBasis;
                thisConstruct.BSDFInput.SolBkReflNcols = NBasis;

                if (NumRows != NBasis) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Solar back reflectance matrix \"{}\" is not the same size as it is defined by basis definition. Basis size "
                               "is defined by Matrix:TwoDimension = \"{}\".",
                               locAlphaArgs(7),
                               locAlphaArgs(5)));
                }

                if (NumRows != NumCols) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("Solar back reflectance matrix \"{}\" must have the same number of rows and columns.", locAlphaArgs(7)));
                }

                thisConstruct.BSDFInput.SolBkRefl.allocate(NBasis, NBasis);
                if (thisConstruct.BSDFInput.SolBkReflIndex == 0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("Solar back reflectance Matrix:TwoDimension = \"{}\" is missing from the input file.", locAlphaArgs(7)));
                } else {
                    MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.SolBkReflIndex, state.dataBSDFWindow->BSDFTempMtrx);
                    thisConstruct.BSDFInput.SolBkRefl = 0.0;
                    for (int I = 1; I <= NBasis; ++I) {
                        thisConstruct.BSDFInput.SolBkRefl(I, I) = state.dataBSDFWindow->BSDFTempMtrx(I, 1);
                    }
                }

                // *******************************************************************************
                // Visible front transmittance
                // *******************************************************************************
                thisConstruct.BSDFInput.VisFrtTransIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(8));
                MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.VisFrtTransIndex, NumRows, NumCols);
                thisConstruct.BSDFInput.VisFrtTransNrows = NBasis;
                thisConstruct.BSDFInput.VisFrtTransNcols = NBasis;

                if (NumRows != NBasis) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Visible front transmittance matrix \"{}\" is not the same size as it is defined by basis definition. Basis "
                               "size is defined by Matrix:TwoDimension = \"{}\".",
                               locAlphaArgs(8),
                               locAlphaArgs(5)));
                }

                if (NumRows != NumCols) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Visible front transmittance matrix \"{}\" must have the same number of rows and columns.", locAlphaArgs(8)));
                }

                thisConstruct.BSDFInput.VisFrtTrans.allocate(NBasis, NBasis);
                if (thisConstruct.BSDFInput.VisFrtTransIndex == 0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state,
                        eoh,
                        format("Visible front transmittance Matrix:TwoDimension = \"{}\" is missing from the input file.", locAlphaArgs(8)));
                } else {
                    MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.VisFrtTransIndex, state.dataBSDFWindow->BSDFTempMtrx);
                    thisConstruct.BSDFInput.VisFrtTrans = 0.0;
                    for (int I = 1; I <= NBasis; ++I) {
                        thisConstruct.BSDFInput.VisFrtTrans(I, I) = state.dataBSDFWindow->BSDFTempMtrx(I, 1);
                    }
                }

                // *******************************************************************************
                // Visible back reflectance
                // *******************************************************************************
                thisConstruct.BSDFInput.VisBkReflIndex = MatrixDataManager::MatrixIndex(state, locAlphaArgs(9));
                MatrixDataManager::Get2DMatrixDimensions(state, thisConstruct.BSDFInput.VisBkReflIndex, NumRows, NumCols);
                thisConstruct.BSDFInput.VisBkReflNrows = NBasis;
                thisConstruct.BSDFInput.VisBkReflNcols = NBasis;

                if (NumRows != NBasis) {
                    ErrorsFound = true;
                    ShowSevereCustom(state,
                                     eoh,
                                     format("Visible back reflectance matrix \"{}\" is not the same size as it is defined by basis definition. Basis "
                                            "size is defined by Matrix:TwoDimension = \"{}\".",
                                            locAlphaArgs(9),
                                            locAlphaArgs(5)));
                }

                if (NumRows != NumCols) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("Visible back reflectance matrix \"{}\" must have the same number of rows and columns.", locAlphaArgs(9)));
                }

                thisConstruct.BSDFInput.VisBkRefl.allocate(NBasis, NBasis);
                if (thisConstruct.BSDFInput.VisBkReflIndex == 0) {
                    ErrorsFound = true;
                    ShowSevereCustom(
                        state, eoh, format("Visible back reflectance Matrix:TwoDimension = \"{}\" is missing from the input file.", locAlphaArgs(9)));
                } else {
                    MatrixDataManager::Get2DMatrix(state, thisConstruct.BSDFInput.VisBkReflIndex, state.dataBSDFWindow->BSDFTempMtrx);
                    thisConstruct.BSDFInput.VisBkRefl = 0.0;
                    for (int I = 1; I <= NBasis; ++I) {
                        thisConstruct.BSDFInput.VisBkRefl(I, I) = state.dataBSDFWindow->BSDFTempMtrx(I, 1);
                    }
                }

                // determine number of layers
                // Construct(ConstrNum)%TotLayers = (NumAlphas - 9)/3

                // check for incomplete field set
                // IF (Mod((NumAlphas - 9), 3) /= 0) Then
                // throw warning if incomplete field set
                //  CALL ShowWarningError(state, 'Construction:ComplexFenestrationState: Axisymmetric properties have incomplete field &
                //   & set')
                // ENDIF

                // ALLOCATE(Construct(ConstrNum)%BSDFInput%Layer(NumOfOpticalLayers))
                for (int Layer = 1; Layer <= thisConstruct.TotLayers; ++Layer) {

                    if (mod(Layer, 2) != 0) {
                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).MaterialIndex = thisConstruct.LayerPoint(Layer);

                        // *******************************************************************************
                        // Front absorptance matrix
                        // *******************************************************************************
                        ++AlphaIndex;
                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbsIndex =
                            MatrixDataManager::MatrixIndex(state, locAlphaArgs(AlphaIndex));
                        MatrixDataManager::Get2DMatrixDimensions(
                            state, thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbsIndex, NumRows, NumCols);

                        if (NumRows != 1) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Front absorbtance Matrix:TwoDimension = \"{}\" for layer {} must have only one row.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer));
                        }

                        if (NumCols != NBasis) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Front absorbtance Matrix:TwoDimension = \"{}\" for layer {} must have same number of columns "
                                                    "as it is defined by basis matrix."
                                                    "Matrix has {} number of columns, while basis definition specifies {} number of columns.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer,
                                                    NumCols,
                                                    NBasis));
                        }

                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).AbsNcols = NumCols;
                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbs.allocate(NumCols, NumRows);

                        if (thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbsIndex == 0) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Front absorbtance Matrix:TwoDimension = \"{}\" for layer {} is missing from the input file.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer));
                        } else {
                            MatrixDataManager::Get2DMatrix(state,
                                                           thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbsIndex,
                                                           thisConstruct.BSDFInput.Layer(currentOpticalLayer).FrtAbs);
                        }

                        // *******************************************************************************
                        // Back absorptance matrix
                        // *******************************************************************************
                        ++AlphaIndex;
                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbsIndex =
                            MatrixDataManager::MatrixIndex(state, locAlphaArgs(AlphaIndex));
                        MatrixDataManager::Get2DMatrixDimensions(
                            state, thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbsIndex, NumRows, NumCols);

                        if (NumRows != 1) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Back absorbtance Matrix:TwoDimension = \"{}\" for layer {} must have only one row.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer));
                        }

                        if (NumCols != NBasis) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Back absorbtance Matrix:TwoDimension = \"{}\" for layer {} must have same number of columns as "
                                                    "it is defined by basis matrix."
                                                    "Matrix has {} number of columns, while basis definition specifies {} number of columns.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer,
                                                    NumCols,
                                                    NBasis));
                        }

                        thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbs.allocate(NumCols, NumRows);

                        if (thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbsIndex == 0) {
                            ErrorsFound = true;
                            ShowSevereCustom(state,
                                             eoh,
                                             format("Back absorbtance Matrix:TwoDimension = \"{}\" for layer {} is missing from the input file.",
                                                    locAlphaArgs(AlphaIndex),
                                                    currentOpticalLayer));
                        } else {
                            MatrixDataManager::Get2DMatrix(state,
                                                           thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbsIndex,
                                                           thisConstruct.BSDFInput.Layer(currentOpticalLayer).BkAbs);
                        }
                    } // if (Mod(Layer, 2) <> 0) then
                }

                state.dataBSDFWindow->BSDFTempMtrx.deallocate();
            }
            thisConstruct.TypeIsWindow = true;
            thisConstruct.WindowTypeBSDF = true;
        }

        // Do not forget to deallocate localy allocated variables
        if (allocated(locAlphaFieldNames)) {
            locAlphaFieldNames.deallocate();
        }
        if (allocated(locNumericFieldNames)) {
            locNumericFieldNames.deallocate();
        }
        if (allocated(locNumericFieldBlanks)) {
            locNumericFieldBlanks.deallocate();
        }
        if (allocated(locAlphaFieldBlanks)) {
            locAlphaFieldBlanks.deallocate();
        }
        if (allocated(locAlphaArgs)) {
            locAlphaArgs.deallocate();
        }
        if (allocated(locNumericArgs)) {
            locNumericArgs.deallocate();
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Error in complex fenestration input.");
        }
    }

    void InitConductionTransferFunctions(EnergyPlusData &state)
    {
        bool ErrorsFound(false); // Flag for input error condition
        bool DoCTFErrorReport(false);
        for (auto &construction : state.dataConstruction->Construct) {
            construction.calculateTransferFunction(state, ErrorsFound, DoCTFErrorReport);
            if (construction.NumHistories > 1) {
                state.dataHeatBal->SimpleCTFOnly = false;
            }
            if (construction.NumCTFTerms > state.dataHeatBal->MaxCTFTerms) {
                state.dataHeatBal->MaxCTFTerms = construction.NumCTFTerms;
            }
        }
        if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
            state.dataHeatBal->SimpleCTFOnly = false;
        }

        for (auto &construction : state.dataConstruction->Construct) {
            if (!construction.IsUsedCTF) {
                continue;
            }
            construction.reportLayers(state);
        }

        bool InitCTFDoReport;
        General::ScanForReports(state, "Constructions", InitCTFDoReport, "Constructions");
        if (InitCTFDoReport || DoCTFErrorReport) {
            print(state.files.eio,
                  "! <Construction CTF>,Construction Name,Index,#Layers,#CTFs,Time Step {{hours}},ThermalConductance "
                  "{{w/m2-K}},OuterThermalAbsorptance,InnerThermalAbsorptance,OuterSolarAbsorptance,InnerSolarAbsorptance,Roughness\n");
            print(state.files.eio,
                  "! <Material CTF Summary>,Material Name,Thickness {{m}},Conductivity {{w/m-K}},Density {{kg/m3}},Specific Heat "
                  "{{J/kg-K}},ThermalResistance {{m2-K/w}}\n");
            print(state.files.eio, "! <Material:Air CTF Summary>,Material Name,ThermalResistance {{m2-K/w}}\n");
            print(state.files.eio, "! <CTF>,Time,Outside,Cross,Inside,Flux (except final one)\n");

            int cCounter = 0; // just used to keep construction index in output report
            for (auto &construction : state.dataConstruction->Construct) {
                cCounter++;
                if (!construction.IsUsedCTF) {
                    continue;
                }
                construction.reportTransferFunction(state, cCounter);
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Program terminated for reasons listed (InitConductionTransferFunctions)");
        }
    }

} // namespace HeatBalanceManager

} // namespace EnergyPlus
