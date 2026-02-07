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

#ifndef PhotovoltaicThermalCollectors_hh_INCLUDED
#define PhotovoltaicThermalCollectors_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PhotovoltaicThermalCollectors {

    enum class PVTMode
    {
        Invalid = -1,
        Heating,
        Cooling,
        Num
    };

    enum struct WorkingFluidEnum
    {
        Invalid = -1,
        LIQUID,
        AIR,
        Num
    };

    enum struct ThermEfficEnum
    {
        Invalid = -1,
        SCHEDULED,
        FIXED,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(ThermEfficEnum::Num)> ThermEfficTypeNamesUC = {"SCHEDULED", "FIXED"};

    struct SimplePVTModelStruct
    {
        std::string Name;
        Real64 ThermalActiveFract = 0.0;                       // fraction of surface area with active thermal collection
        ThermEfficEnum ThermEfficMode = ThermEfficEnum::FIXED; // setting for how therm effic is determined
        Real64 ThermEffic = 0.0;                               // fixed or current Therm efficiency
        Sched::Schedule *thermEffSched = nullptr;              // pointer to schedule for therm effic (if any)
        Real64 SurfEmissivity = 0.0;                           // surface emittance in long wave IR
        Real64 LastCollectorTemp = 0.0;                        // store previous temperature
    };

    struct BIPVTModelStruct
    {
        std::string Name;
        std::string OSCMName;                  // OtherSideConditionsModel
        int OSCMPtr = 0;                       // OtherSideConditionsModel index
        Sched::Schedule *availSched = nullptr; // Availability schedule
        Real64 PVEffGapWidth = 0.0;            // Effective Gap Plenum Behind PV modules (m)
        Real64 PVCellTransAbsProduct = 0.0;    // PV cell Transmittance-Absorptance prodiuct
        Real64 BackMatTranAbsProduct = 0.0;    // Backing Material Normal Transmittance-Absorptance Product
        Real64 CladTranAbsProduct = 0.0;       // Cladding Normal Transmittance-Absorptance Product
        Real64 PVAreaFract = 0.0;              // Fraction of collector gross area covered by PV module
        Real64 PVCellAreaFract = 0.0;          // Fraction of PV cell area to module area
        Real64 PVRTop = 0.0;                   // PV module top thermal resistance (m2-DegC/W)
        Real64 PVRBot = 0.0;                   // PV module bottom thermal resistance (m2-DegC/W)
        Real64 PVGEmiss = 0.0;                 // Emissivity PV modules
        Real64 BackMatEmiss = 0.0;             // Emissivity of backing material
        Real64 ThGlass = 0.0;                  // Glass thickness (m)
        Real64 RIndGlass = 0.0;                // Glass refraction index
        Real64 ECoffGlass = 0.0;               // Glass extinction coefficient
        Real64 LastCollectorTemp = 0.0;        // store previous temperature (DegC)
        Real64 Tplen = 20.0;                   // modeled drybulb temperature for air through BIPVT channel (DegC)
        Real64 Tcoll = 20.0;                   // modeled temperature of BIPVT channel surface on PV side (DegC)
        Real64 HrPlen = 1.0;                   // Modeled radiation coef for OSCM (W/m2-C)
        Real64 HcPlen = 10.0;                  // Modeled Convection coef for OSCM (W/m2-C)
    };

    struct PVTReportStruct
    {
        Real64 ThermPower = 0.0;       // Heat gain or loss to collector fluid (W)
        Real64 ThermHeatGain = 0.0;    // Heat gain to collector fluid (W)
        Real64 ThermHeatLoss = 0.0;    // Heat loss from collector fluid (W)
        Real64 ThermEnergy = 0.0;      // Energy gained (or lost) to collector fluid (J)
        Real64 MdotWorkFluid = 0.0;    // working fluid mass flow rate (kg/s)
        Real64 TinletWorkFluid = 0.0;  // working fluid inlet temp (C)
        Real64 ToutletWorkFluid = 0.0; // working fluid outlet temp (C)
        Real64 BypassStatus = 0.0;     // 0 = no bypass, 1=full bypass
    };

    enum class PVTModelType
    {
        Invalid = -1,
        Simple = 1001,
        BIPVT = 1002,
        Num
    };

    struct PVTCollectorStruct : PlantComponent
    {
        std::string Name;                                                            // Name of PVT collector
        DataPlant::PlantEquipmentType Type = DataPlant::PlantEquipmentType::Invalid; // Plant Side Connection: 'Type' assigned in DataPlant
        PlantLocation WPlantLoc;                                                     // Water plant loop component location
        bool EnvrnInit = true;                                                       // manage begin environment inits
        bool SizingInit = true;                                                      // manage when sizing is complete
        std::string PVTModelName;                                                    // Name of PVT performance object
        PVTModelType ModelType = PVTModelType::Invalid;                              // model type indicator
        PVTMode OperatingMode = PVTMode::Invalid;                                    // PVT operating mode (heating or cooling)
        int SurfNum = 0;                                                             // surface index
        std::string PVname;                                                          // named Generator:Photovoltaic object
        int PVnum = 0;                                                               // PV index
        bool PVfound = false;                                                        // init, need to delay get input until PV gotten
        SimplePVTModelStruct Simple;                                                 // Simple performance data structure.
        BIPVTModelStruct BIPVT;                                                      // BIPVT performance data structure.
        WorkingFluidEnum WorkingFluidType = WorkingFluidEnum::LIQUID;
        int PlantInletNodeNum = 0;
        int PlantOutletNodeNum = 0;
        int HVACInletNodeNum = 0;
        int HVACOutletNodeNum = 0;
        Real64 DesignVolFlowRate = 0.0;
        bool DesignVolFlowRateWasAutoSized = false; // true if design volume flow rate was autosize on input
        Real64 MaxMassFlowRate = 0.0;
        Real64 MassFlowRate = 0.0;
        Real64 AreaCol = 0.0;
        bool BypassDamperOff = true;
        bool CoolingUseful = false;
        bool HeatingUseful = false;
        PVTReportStruct Report;
        bool MySetPointCheckFlag = true;
        bool MyOneTimeFlag = true;
        bool SetLoopIndexFlag = true;
        Real64 QdotSource = 0.0; // Source/sink term

        static PlantComponent *factory(EnergyPlusData &state, std::string_view objectName);

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void setupReportVars(EnergyPlusData &state);

        void initialize(EnergyPlusData &state, bool FirstHVACIteration);

        void size(EnergyPlusData &state);

        void control(EnergyPlusData &state);

        void calculate(EnergyPlusData &state);

        void calculateSimplePVT(EnergyPlusData &state);

        void calculateBIPVT(EnergyPlusData &state);

        void calculateBIPVTMaxHeatGain(EnergyPlusData &state, Real64 tsp, Real64 &bfr, Real64 &q, Real64 &tmixed, Real64 &ThEff, Real64 &tpv);

        void solveLinSysBackSub(std::array<Real64, 9> &jj, std::array<Real64, 3> &f, std::array<Real64, 3> &y);

        Real64 calc_k_taoalpha(Real64 theta, Real64 glass_thickness, Real64 refrac_index_glass, Real64 k_glass);

        Real64 calc_taoalpha(Real64 theta, Real64 glass_thickness, Real64 refrac_index_glass, Real64 k_glass);

        void update(EnergyPlusData &state);

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void GetPVTcollectorsInput(EnergyPlusData &state);

    void GetPVTSimpleCollectorsInput(EnergyPlusData &state, int NumSimplePVTPerform, Array1D<SimplePVTModelStruct> &tmpSimplePVTperf);

    void GetBIPVTCollectorsInput(EnergyPlusData &state, int NumBIPVTPerform, Array1D<BIPVTModelStruct> &tmpBIPVTperf);

    void GetMainPVTInput(EnergyPlusData &state,
                         int NumPVT,
                         Array1D<PVTCollectorStruct> &PVT,
                         Array1D<SimplePVTModelStruct> const &tmpSimplePVTperf,
                         Array1D<BIPVTModelStruct> const &tmpBIPVTperf);

    void simPVTfromOASys(EnergyPlusData &state, int index, bool FirstHVACIteration);

    int getPVTindexFromName(EnergyPlusData &state, std::string_view name);

    void GetPVTThermalPowerProduction(EnergyPlusData &state, int PVindex, Real64 &ThermalPower, Real64 &ThermalEnergy);

    int GetAirInletNodeNum(EnergyPlusData &state, std::string_view PVTName, bool &ErrorsFound);

    int GetAirOutletNodeNum(EnergyPlusData &state, std::string_view PVTName, bool &ErrorsFound);

    int GetPVTmodelIndex(EnergyPlusData &state, int SurfacePtr);

    void SetPVTQdotSource(EnergyPlusData &state, int PVTNum, Real64 QSource);

    void GetPVTTsColl(EnergyPlusData &state, int PVTNum, Real64 &TsColl);

} // namespace PhotovoltaicThermalCollectors

struct PhotovoltaicThermalCollectorsData : BaseGlobalStruct
{
    bool GetInputFlag = true; // First time, input is "gotten"
    int NumPVT = 0;           // count of all types of PVT in input file
    Array1D<PhotovoltaicThermalCollectors::PVTCollectorStruct> PVT;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) PhotovoltaicThermalCollectorsData();
    }
};

} // namespace EnergyPlus

#endif
