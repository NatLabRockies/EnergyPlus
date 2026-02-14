// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
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

#ifndef ElectricBaseboardRadiator_hh_INCLUDED
#define ElectricBaseboardRadiator_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ElectricBaseboardRadiator {

    struct ElecBaseboardParams
    {
        // Members
        std::string EquipName;
        int EquipType = 0;
        std::string Schedule;
        Array1D_string SurfaceName;
        Array1D_int SurfacePtr;
        int ZonePtr = 0;
        Sched::Schedule *availSched = nullptr; // Assuming availability schedule
        int TotSurfToDistrib = 0;
        Real64 NominalCapacity = 0.0;
        Real64 BaseboardEfficiency = 0.0;
        Real64 AirInletTemp = 0.0;
        Real64 AirInletHumRat = 0.0;
        Real64 AirOutletTemp = 0.0;
        Real64 ElecUseLoad = 0.0;
        Real64 ElecUseRate = 0.0;
        Real64 FracRadiant = 0.0;
        Real64 FracConvect = 0.0;
        Real64 FracDistribPerson = 0.0;
        Real64 TotPower = 0.0;
        Real64 Power = 0.0;
        Real64 ConvPower = 0.0;
        Real64 RadPower = 0.0;
        Real64 TotEnergy = 0.0;
        Real64 Energy = 0.0;
        Real64 ConvEnergy = 0.0;
        Real64 RadEnergy = 0.0;
        Array1D<Real64> FracDistribToSurf;
        int HeatingCapMethod = 0;           // - Method for electric baseboard heating capacity scalable sizing calculation
        Real64 ScaledHeatingCapacity = 0.0; // - scaled maximum heating capacity {W} or scalable variable for sizing in {-}, or {W/m2}
        bool MySizeFlag = true;
        bool MyEnvrnFlag = true;
        bool CheckEquipName = true;
        Real64 ZeroBBSourceSumHATsurf = 0.0; // used in baseboard energy balance
        // Record keeping variables used to calculate QBBRadSrcAvg locally
        Real64 QBBElecRadSource = 0.0;   // Need to keep the last value in case we are still iterating
        Real64 QBBElecRadSrcAvg = 0.0;   // Need to keep the last value in case we are still iterating
        Real64 LastSysTimeElapsed = 0.0; // Need to keep the last value in case we are still iterating
        Real64 LastTimeStepSys = 0.0;    // Need to keep the last value in case we are still iterating
        Real64 LastQBBElecRadSrc = 0.0;  // Need to keep the last value in case we are still iterating
    };

    struct ElecBaseboardNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        ElecBaseboardNumericFieldData()
        {
        }
    };

    void SimElecBaseboard(
        EnergyPlusData &state, std::string const &EquipName, int const ZoneNum, bool const FirstHVACIteration, Real64 &PowerMet, int &CompIndex);

    void GetElectricBaseboardInput(EnergyPlusData &state);

    void InitElectricBaseboard(EnergyPlusData &state, int const BaseboardNum, int const ControlledZoneNum, bool const FirstHVACIteration);

    void SizeElectricBaseboard(EnergyPlusData &state, int const BaseboardNum);

    void CalcElectricBaseboard(EnergyPlusData &state, int const BaseboardNum, int const ControlledZoneNum);

    void UpdateElectricBaseboardOff(Real64 &LoadMet,
                                    Real64 &QBBCap,
                                    Real64 &RadHeat,
                                    Real64 &QBBElecRadSrc,
                                    Real64 &ElecUseRate,
                                    Real64 &AirOutletTemp,
                                    Real64 const AirInletTemp);

    void UpdateElectricBaseboardOn(
        Real64 &AirOutletTemp, Real64 &ElecUseRate, Real64 const AirInletTemp, Real64 const QBBCap, Real64 const CapacitanceAir, Real64 const Effic);

    void UpdateElectricBaseboard(EnergyPlusData &state, int const BaseboardNum);

    void UpdateBBElecRadSourceValAvg(EnergyPlusData &state, bool &ElecBaseboardSysOn); // .TRUE. if the radiant system has run this zone time step

    void DistributeBBElecRadGains(EnergyPlusData &state);

    void ReportElectricBaseboard(EnergyPlusData &state, int const BaseboardNum);

} // namespace ElectricBaseboardRadiator

struct ElectricBaseboardRadiatorData : BaseGlobalStruct
{
    std::string const cCMO_BBRadiator_Electric = "ZoneHVAC:Baseboard:RadiantConvective:Electric";

    // Object Data
    int NumElecBaseboards = 0;
    Array1D<ElectricBaseboardRadiator::ElecBaseboardParams> ElecBaseboard;
    Array1D<ElectricBaseboardRadiator::ElecBaseboardNumericFieldData> ElecBaseboardNumericFields;
    bool GetInputFlag = true; // One time get input flag

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumElecBaseboards = 0;
        this->GetInputFlag = true;
        this->ElecBaseboard.clear();
        this->ElecBaseboardNumericFields.clear();
    }
};

} // namespace EnergyPlus

#endif
