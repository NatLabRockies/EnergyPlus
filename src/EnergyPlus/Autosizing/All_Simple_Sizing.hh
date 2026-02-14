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

#ifndef All_Simple_Sizing_hh_INCLUDED
#define All_Simple_Sizing_hh_INCLUDED

#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/Data/BaseData.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

struct AutoCalculateSizer : BaseSizer
{
    AutoCalculateSizer()
    {
        this->sizingType = AutoSizingType::AutoCalculateSizing;
        this->sizingString = "AutoCalculate: Set string in Component Model using overrideSizingString";
    }
    ~AutoCalculateSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct MaxHeaterOutletTempSizer : BaseSizer
{
    MaxHeaterOutletTempSizer()
    {
        this->sizingType = AutoSizingType::MaxHeaterOutletTempSizing;
        this->sizingString = "Maximum Supply Air Temperature [C]";
    }
    ~MaxHeaterOutletTempSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct ZoneCoolingLoadSizer : BaseSizer
{
    ZoneCoolingLoadSizer()
    {
        this->sizingType = AutoSizingType::ZoneCoolingLoadSizing;
        this->sizingString = "Zone Cooling Sensible Load [W]";
    }
    ~ZoneCoolingLoadSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct ZoneHeatingLoadSizer : BaseSizer
{
    ZoneHeatingLoadSizer()
    {
        this->sizingType = AutoSizingType::ZoneHeatingLoadSizing;
        this->sizingString = "Zone Heating Sensible Load [W]";
    }
    ~ZoneHeatingLoadSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct ASHRAEMinSATCoolingSizer : BaseSizer
{
    ASHRAEMinSATCoolingSizer()
    {
        this->sizingType = AutoSizingType::ASHRAEMinSATCoolingSizing;
        this->sizingString = "Minimum Supply Air Temperature in Cooling Mode [C]";
    }
    ~ASHRAEMinSATCoolingSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct ASHRAEMaxSATHeatingSizer : BaseSizer
{
    ASHRAEMaxSATHeatingSizer()
    {
        this->sizingType = AutoSizingType::ASHRAEMaxSATHeatingSizing;
        this->sizingString = "Maximum Supply Air Temperature in Heating Mode [C]";
    }
    ~ASHRAEMaxSATHeatingSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct DesiccantDehumidifierBFPerfDataFaceVelocitySizer : BaseSizer
{
    DesiccantDehumidifierBFPerfDataFaceVelocitySizer()
    {
        this->sizingType = AutoSizingType::DesiccantDehumidifierBFPerfDataFaceVelocitySizing;
        this->sizingString = "Nominal Air Face Velocity [m/s]";
    }
    ~DesiccantDehumidifierBFPerfDataFaceVelocitySizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct HeatingCoilDesAirInletTempSizer : BaseSizer
{
    HeatingCoilDesAirInletTempSizer()
    {
        this->sizingType = AutoSizingType::HeatingCoilDesAirInletTempSizing;
        this->sizingString = "Rated Inlet Air Temperature";
    }
    ~HeatingCoilDesAirInletTempSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct HeatingCoilDesAirOutletTempSizer : BaseSizer
{
    HeatingCoilDesAirOutletTempSizer()
    {
        this->sizingType = AutoSizingType::HeatingCoilDesAirOutletTempSizing;
        this->sizingString = "Rated Outlet Air Temperature";
    }
    ~HeatingCoilDesAirOutletTempSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

struct HeatingCoilDesAirInletHumRatSizer : BaseSizer
{
    HeatingCoilDesAirInletHumRatSizer()
    {
        this->sizingType = AutoSizingType::HeatingCoilDesAirInletHumRatSizing;
        this->sizingString = "Rated Inlet Air Humidity Ratio";
    }
    ~HeatingCoilDesAirInletHumRatSizer() = default;

    Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) override;
};

} // namespace EnergyPlus

#endif
