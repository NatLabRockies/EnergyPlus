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

#ifndef CostEstimateManager_hh_INCLUDED
#define CostEstimateManager_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace CostEstimateManager {

    enum class ParentObject
    {
        Invalid = -1,
        General,
        Construction,
        CoilDX,
        CoilCoolingDX,
        CoilCoolingDXSingleSpeed,
        CoilHeatingFuel,
        ChillerElectric,
        DaylightingControls,
        ShadingZoneDetailed,
        Lights,
        GeneratorPhotovoltaic,
        Num
    };

    struct CostLineItemStruct
    {
        // Members
        std::string LineName;       // object name (needed ?)
        ParentObject ParentObjType; // parent reference to IDD object type
        std::string ParentObjName;  // parent instance in IDF
        int ParentObjIDinList = 1;
        Real64 PerSquareMeter = 0.0;     // cost per square meter
        Real64 PerEach = 0.0;            // cost per each
        Real64 PerKiloWattCap = 0.0;     // cost per kW of nominal capacity
        Real64 PerKWCapPerCOP = 0.0;     // cost per kW of nominal capacity per COP
        Real64 PerCubicMeter = 0.0;      // cost per cubic meter
        Real64 PerCubMeterPerSec = 0.0;  // cost per cubic meter per second
        Real64 PerUAinWattperDelK = 0.0; // cost per (UA) in Watt/deltaK
        int LineNumber = -1;             // number of line item in detail list
        Real64 Qty = 0.0;                // quantity in calculations (can be input)
        std::string Units;               // Reported units
        Real64 ValuePer = 0.0;           // Cost used in final calculation
        Real64 LineSubTotal = 0.0;       // line item total  Qty * ValuePer
    };

    struct CostAdjustmentStruct
    {
        // Members
        Real64 LineItemTot;        // = 0.0 ! holds total from line item cost calculations
        Real64 MiscCostperSqMeter; // = 0.0 ! holds user-defined constant cost model
        Real64 DesignFeeFrac;      // = 0.0 ! holds user-defined fraction for design fees
        Real64 ContractorFeeFrac;  // = 0.0 ! holds user-defined fraction for contractor fees
        Real64 ContingencyFrac;    // = 0.0 ! holds user-defined fraction for contingencies
        Real64 BondCostFrac;       // = 0.0 ! holds user-defined fraction for bonding costs
        Real64 CommissioningFrac;  // = 0.0 ! holds user-defined fraction for commissioning costs
        Real64 RegionalModifier;   // = 1.0 ! holds user-defined multiplier to account for regional diffs
        Real64 GrandTotal;         // = 0.0 ! the Grand Total of all line items plus all other costs

        // Default Constructor
        CostAdjustmentStruct()
            : LineItemTot(0.0), MiscCostperSqMeter(0.0), DesignFeeFrac(0.0), ContractorFeeFrac(0.0), ContingencyFrac(0.0), BondCostFrac(0.0),
              CommissioningFrac(0.0), RegionalModifier(0.0), GrandTotal(0.0)
        {
        }

        // Member Constructor
        CostAdjustmentStruct(Real64 const LineItemTot,        // = 0.0 ! holds total from line item cost calculations
                             Real64 const MiscCostperSqMeter, // = 0.0 ! holds user-defined constant cost model
                             Real64 const DesignFeeFrac,      // = 0.0 ! holds user-defined fraction for design fees
                             Real64 const ContractorFeeFrac,  // = 0.0 ! holds user-defined fraction for contractor fees
                             Real64 const ContingencyFrac,    // = 0.0 ! holds user-defined fraction for contingencies
                             Real64 const BondCostFrac,       // = 0.0 ! holds user-defined fraction for bonding costs
                             Real64 const CommissioningFrac,  // = 0.0 ! holds user-defined fraction for commissioning costs
                             Real64 const RegionalModifier,   // = 1.0 ! holds user-defined multiplier to account for regional diffs
                             Real64 const GrandTotal          // = 0.0 ! the Grand Total of all line items plus all other costs
                             )
            : LineItemTot(LineItemTot), MiscCostperSqMeter(MiscCostperSqMeter), DesignFeeFrac(DesignFeeFrac), ContractorFeeFrac(ContractorFeeFrac),
              ContingencyFrac(ContingencyFrac), BondCostFrac(BondCostFrac), CommissioningFrac(CommissioningFrac), RegionalModifier(RegionalModifier),
              GrandTotal(GrandTotal)
        {
        }
    };

    struct monetaryUnitType
    {
        // Members
        std::string code; // ISO code for currency such as USD or EUR
        std::string txt;  // text representation of the currency
        std::string html; // representation for HTML file - contains unicode references

        // Default Constructor
        monetaryUnitType() : code(std::string()), txt(std::string()), html(std::string())
        {
        }
    };

    void SimCostEstimate(EnergyPlusData &state);

    void GetCostEstimateInput(EnergyPlusData &state);

    void CheckCostEstimateInput(EnergyPlusData &state, bool &ErrorsFound); // Set to true if errors in input, fatal at end of routine

    void CalcCostEstimate(EnergyPlusData &state);

} // namespace CostEstimateManager

struct CostEstimateManagerData : BaseGlobalStruct
{

    bool GetCostInput = true;
    bool DoCostEstimate = false; // set to true if any cost estimating needed
    int selectedMonetaryUnit = 0;

    Array1D<CostEstimateManager::CostLineItemStruct> CostLineItem;

    CostEstimateManager::CostAdjustmentStruct CurntBldg = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    // holds total from line item cost calculations | holds user-defined
    // constant cost model | holds user-defined fraction for design fees
    // | holds user-defined fraction for contractor fees | holds
    // user-defined fraction for contingencies | holds user-defined
    // fraction for bonding costs | holds user-defined fraction for
    // commissioning costs | holds user-defined multiplier to account for
    // regional diffs | the Grand Total of all line items plus all other
    // costs

    CostEstimateManager::CostAdjustmentStruct RefrncBldg{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    // holds total from line item cost calculations | holds user-defined
    // constant cost model | holds user-defined fraction for design fees
    // | holds user-defined fraction for contractor fees | holds
    // user-defined fraction for contingencies | holds user-defined
    // fraction for bonding costs | holds user-defined fraction for
    // commissioning costs | holds user-defined multiplier to account
    // for regional diffs | the Grand Total of all line items plus all
    // other costs

    Array1D<CostEstimateManager::monetaryUnitType> monetaryUnit;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) CostEstimateManagerData();
    }
};

} // namespace EnergyPlus

#endif
