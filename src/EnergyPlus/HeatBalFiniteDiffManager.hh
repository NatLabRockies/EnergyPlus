// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

#ifndef HeatBalFiniteDiffManager_hh_INCLUDED
#define HeatBalFiniteDiffManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/PhaseChangeModeling/HysteresisModel.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HeatBalFiniteDiffManager {

    Real64 constexpr TempInitValue(23.0);   // Initialization value for Temperature
    Real64 constexpr RhovInitValue(0.0115); // Initialization value for Rhov
    Real64 constexpr EnthInitValue(100.0);  // Initialization value for Enthalpy
    constexpr Real64 smalldiff(1.e-8);      // Used in places where "equality" tests should not be used.
    constexpr Real64 MinTempLimit = -100.0; // lower limit check, degree C
    constexpr Real64 MaxTempLimit = 100.0;  // upper limit check, degree C

    enum class CondFDScheme
    {
        Invalid = -1,
        CrankNicholsonSecondOrder, // original CondFD scheme.  semi implicit, second order in time
        FullyImplicitFirstOrder,   // fully implicit scheme, first order in time.
        Num
    };

    static constexpr std::array<std::string_view, static_cast<int>(CondFDScheme::Num)> CondFDSchemeTypeNamesCC = {"CrankNicholsonSecondOrder",
                                                                                                                  "FullyImplicitFirstOrder"};
    static constexpr std::array<std::string_view, static_cast<int>(CondFDScheme::Num)> CondFDSchemeTypeNamesUC = {"CRANKNICHOLSONSECONDORDER",
                                                                                                                  "FULLYIMPLICITFIRSTORDER"};

    struct ConstructionDataFD
    {
        // Members
        Array1D_string Name; // Name of construction
        Array1D<Real64> DelX;
        Array1D<Real64> TempStability;
        Array1D<Real64> MoistStability;
        Array1D_int NodeNumPoint;
        Array1D<Real64> Thickness;
        Array1D<Real64> NodeXlocation; // sized to TotNode, contains X distance in m from outside face
        int TotNodes;
        int DeltaTime;

        // Default Constructor
        ConstructionDataFD() : TotNodes(0), DeltaTime(0)
        {
        }
    };

    struct MaterialActuatorData
    {
        std::string actuatorName;
        bool isActuated;
        Real64 actuatedValue;

        MaterialActuatorData() : isActuated(false), actuatedValue(0.0)
        {
        }
    };

    struct SurfaceDataFD
    {
        // Members
        Array1D<Real64> T;
        Array1D<Real64> TOld;
        Array1D<Real64> TT;
        Array1D<Real64> Rhov;
        Array1D<Real64> RhovOld;
        Array1D<Real64> RhoT;
        Array1D<Real64> TD;
        Array1D<Real64> TDT;
        Array1D<Real64> TDTLast;
        Array1D<Real64> TDOld;
        Array1D<Real64> TDreport; // Node temperatures for reporting [C]
        Array1D<Real64> RH;
        Array1D<Real64> RHreport;
        Array1D<Real64> EnthOld; // Current node enthalpy
        Array1D<Real64> EnthNew; // Node enthalpy at new time
        Array1D<Real64> EnthLast;
        Array1D<Real64> QDreport;        // Node heat flux for reporting [W/m2] positive is flow towards inside face of surface
        Array1D<Real64> CpDelXRhoS1;     // Current outer half-node Cp * DelX * RhoS / Delt
        Array1D<Real64> CpDelXRhoS2;     // Current inner half-node Cp * DelX * RhoS / Delt
        Array1D<Real64> TDpriortimestep; // Node temperatures from previous timestep
        int SourceNodeNum;               // Node number for internal source layer (zero if no source)
        Real64 QSource;                  // Internal source flux [W/m2]
        int GSloopCounter;               // count of inner loop iterations
        Real64 MaxNodeDelTemp;           // largest change in node temps after calc
        int indexNodeMaxTempLimit = 0;   // index for recurring error message if node temperature exceeds maximum node temperature for this surface
        int indexNodeMinTempLimit = 0;   // index for recurring error message if node temperature is below minimum node temperature for this surfac
        Real64 EnthalpyM;                // Melting enthalpy at a particular temperature
        Real64 EnthalpyF;                // Freezing enthalpy at a particular temperature
        Array1D<Material::Phase> PhaseChangeState;
        Array1D<Material::Phase> PhaseChangeStateOld;
        Array1D<Material::Phase> PhaseChangeStateOldOld;
        Array1D<int> PhaseChangeStateRep;
        Array1D<int> PhaseChangeStateOldRep;
        Array1D<int> PhaseChangeStateOldOldRep;
        Array1D<Real64> PhaseChangeTemperatureReverse;
        Array1D<MaterialActuatorData> condMaterialActuators;
        Array1D<MaterialActuatorData> specHeatMaterialActuators;
        Array1D<MaterialActuatorData> heatSourceFluxMaterialActuators;
        Array1D<Real64> condNodeReport;
        Array1D<Real64> specHeatNodeReport;
        // Includes the internal heat source
        Array1D<Real64> heatSourceInternalFluxLayerReport;
        Array1D<Real64> heatSourceInternalFluxEnergyLayerReport;
        // Includes the EMS heat source
        Array1D<Real64> heatSourceEMSFluxLayerReport;
        Array1D<Real64> heatSourceEMSFluxEnergyLayerReport;

        // Default Constructor
        SurfaceDataFD() : SourceNodeNum(0), QSource(0.0), GSloopCounter(0), MaxNodeDelTemp(0.0), EnthalpyM(0.0), EnthalpyF(0.0), PhaseChangeState(0)
        {
        }

        inline void UpdateMoistureBalance()
        {
            // Based on UpdateMoistureBalanceFD by Richard Liesen
            // Brought into class for performance
            TOld = T;
            RhovOld = Rhov;
            TDOld = TDreport;
        }
    };

    struct MaterialDataFD
    {
        // Members
        Real64 tk1;               // Temperature coefficient for thermal conductivity
        int numTempEnth;          // number of Temperature/Enthalpy pairs
        int numTempCond;          // number of Temperature/Conductivity pairs
        Array2D<Real64> TempEnth; // Temperature enthalpy Function Pairs,
        //  TempEnth(1,1)= first Temp, TempEnth(1,2) = First Enthalpy,
        //  TempEnth(2,1) = secomd Temp, etc.
        Array2D<Real64> TempCond; // Temperature thermal conductivity Function Pairs,
        //  TempCond(1,1)= first Temp, Tempcond(1,2) = First conductivity,
        //  TempEnth(2,1) = secomd Temp, etc.

        // Default Constructor
        MaterialDataFD() : tk1(0.0), numTempEnth(0), numTempCond(0)
        {
        }
    };

    void ManageHeatBalFiniteDiff(EnergyPlusData &state,
                                 int SurfNum,
                                 Real64 &SurfTempInTmp, // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                                 Real64 &TempSurfOutTmp // Outside Surface Temperature of each Heat Transfer Surface
    );

    void GetCondFDInput(EnergyPlusData &state);

    int setSizeMaxProperties(EnergyPlusData &state);

    void InitHeatBalFiniteDiff(EnergyPlusData &state);

    void InitialInitHeatBalFiniteDiff(EnergyPlusData &state);

    int numNodesInMaterialLayer(EnergyPlusData &state, std::string const &surfName, std::string const &matName);

    void CalcHeatBalFiniteDiff(EnergyPlusData &state,
                               int Surf,
                               Real64 &SurfTempInTmp, // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                               Real64 &TempSurfOutTmp // Outside Surface Temperature of each Heat Transfer Surface
    );

    void ReportFiniteDiffInits(EnergyPlusData &state);

    void CalcNodeHeatFlux(EnergyPlusData &state,
                          int Surf,    // surface number
                          int TotNodes // number of nodes in surface
    );

    Real64 terpld(Array2<Real64> const &a, Real64 x1, int nind, int ndep);

    void ExteriorBCEqns(EnergyPlusData &state,
                        int Delt,                    // Time Increment
                        int i,                       // Node Index
                        int Lay,                     // Layer Number for Construction
                        int Surf,                    // Surface number
                        Array1D<Real64> const &T,    // Old node Temperature in MFD finite difference solution
                        Array1D<Real64> &TT,         // New node Temperature in MFD finite difference solution.
                        Array1D<Real64> const &Rhov, // MFD Nodal Vapor Density[kg/m3] and is the old or last time step result.
                        Array1D<Real64> &RhoT,       // MFD vapor density for the new time step.
                        Array1D<Real64> &RH,         // Nodal relative humidity
                        Array1D<Real64> const &TD,   // The old dry Temperature at each node for the CondFD algorithm..
                        Array1D<Real64> &TDT,        // The current or new Temperature at each node location for the CondFD solution..
                        Array1D<Real64> &EnthOld,    // Old Nodal enthalpy
                        Array1D<Real64> &EnthNew,    // New Nodal enthalpy
                        int TotNodes,                // Total nodes in layer
                        Real64 HMovInsul             // Conductance of movable(transparent) insulation.
    );

    void InteriorNodeEqns(EnergyPlusData &state,
                          int Delt,                    // Time Increment
                          int i,                       // Node Index
                          int Lay,                     // Layer Number for Construction
                          int Surf,                    // Surface number
                          Array1D<Real64> const &T,    // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> &TT,         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> const &Rhov, // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> &RhoT,       // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> &RH,         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> const &TD,   // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> &TDT,        // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> &EnthOld,    // Old Nodal enthalpy
                          Array1D<Real64> &EnthNew     // New Nodal enthalpy
    );

    void IntInterfaceNodeEqns(EnergyPlusData &state,
                              int Delt,                       // Time Increment
                              int i,                          // Node Index
                              int Lay,                        // Layer Number for Construction
                              int Surf,                       // Surface number
                              Array1D<Real64> const &T,       // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                              Array1D<Real64> &TT,            // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                              Array1D<Real64> const &Rhov,    // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                              Array1D<Real64> &RhoT,          // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                              Array1D<Real64> &RH,            // RELATIVE HUMIDITY.
                              Array1D<Real64> const &TD,      // OLD NODE TEMPERATURES OF EACH HEAT TRANSFER SURF IN CONDFD.
                              Array1D<Real64> &TDT,           // NEW NODE TEMPERATURES OF EACH HEAT TRANSFER SURF IN CONDFD.
                              Array1D<Real64> const &EnthOld, // Old Nodal enthalpy
                              Array1D<Real64> &EnthNew,       // New Nodal enthalpy
                              int GSiter                      // Iteration number of Gauss Seidell iteration
    );

    void InteriorBCEqns(EnergyPlusData &state,
                        int Delt,                    // Time Increment
                        int i,                       // Node Index
                        int Lay,                     // Layer Number for Construction
                        int Surf,                    // Surface number
                        Array1D<Real64> const &T,    // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF (Old).
                        Array1D<Real64> &TT,         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF (New).
                        Array1D<Real64> const &Rhov, // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        Array1D<Real64> &RhoT,       // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        Array1D<Real64> &RH,         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        Array1D<Real64> const &TD,   // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        Array1D<Real64> &TDT,        // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        Array1D<Real64> &EnthOld,    // Old Nodal enthalpy
                        Array1D<Real64> &EnthNew,    // New Nodal enthalpy
                        Array1D<Real64> &TDreport    // Temperature value from previous HeatSurfaceHeatManager titeration's value
    );

    void CheckFDSurfaceTempLimits(EnergyPlusData &state,
                                  int SurfNum,            // surface number
                                  Real64 CheckTemperature // calculated temperature, not reset
    );

    void CheckFDNodeTempLimits(EnergyPlusData &state,
                               int surfNum,     // surface number
                               int nodeNum,     // node number
                               Real64 &nodeTemp // calculated temperature, not reset
    );

    void adjustPropertiesForPhaseChange(EnergyPlusData &state,
                                        int finiteDifferenceLayerIndex,
                                        int surfaceIndex,
                                        Material::MaterialPhaseChange *mat,
                                        Real64 temperaturePrevious,
                                        Real64 temperatureUpdated,
                                        Real64 &updatedSpecificHeat,
                                        Real64 &updatedDensity,
                                        Real64 &updatedThermalConductivity);

    bool findAnySurfacesUsingConstructionAndCondFD(EnergyPlusData const &state, int const constructionNum);

} // namespace HeatBalFiniteDiffManager

struct HeatBalFiniteDiffMgr : BaseGlobalStruct
{
    Array1D<Real64> SigmaR; // Total Resistance of construction layers
    Array1D<Real64> SigmaC; // Total Capacitance of construction layers

    Array1D<Real64> QHeatInFlux;  // HeatFlux on Surface for reporting
    Array1D<Real64> QHeatOutFlux; // HeatFlux on Surface for reporting

    HeatBalFiniteDiffManager::CondFDScheme CondFDSchemeType =
        HeatBalFiniteDiffManager::CondFDScheme::FullyImplicitFirstOrder; // solution scheme for CondFD - default
    Real64 SpaceDescritConstant = 3.0;                                   // spatial descritization constant,
    int MaxGSiter = 30;                                                  // maximum number of Gauss Seidel iterations
    Real64 fracTimeStepZone_Hour = 0.0;
    bool GetHBFiniteDiffInputFlag = true;
    int WarmupSurfTemp = 0;

    // Object Data
    Array1D<HeatBalFiniteDiffManager::ConstructionDataFD> ConstructFD;
    Array1D<HeatBalFiniteDiffManager::SurfaceDataFD> SurfaceFD;
    Array1D<HeatBalFiniteDiffManager::MaterialDataFD> MaterialFD;
    bool MyEnvrnFlag = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->SigmaR.deallocate();
        this->SigmaC.deallocate();
        this->QHeatInFlux.deallocate();
        this->QHeatOutFlux.deallocate();
        this->CondFDSchemeType = HeatBalFiniteDiffManager::CondFDScheme::FullyImplicitFirstOrder;
        this->SpaceDescritConstant = 3.0;
        this->MaxGSiter = 30;
        this->fracTimeStepZone_Hour = 0.0;
        this->GetHBFiniteDiffInputFlag = true;
        this->WarmupSurfTemp = 0;
        this->ConstructFD.deallocate();
        this->SurfaceFD.deallocate();
        this->MaterialFD.deallocate();
        this->MyEnvrnFlag = true;
    }
};

} // namespace EnergyPlus

#endif
