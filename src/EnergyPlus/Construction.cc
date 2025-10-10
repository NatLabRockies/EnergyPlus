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

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataConversions.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::Construction {

void ConstructionProps::calculateTransferFunction(EnergyPlusData &state, bool &ErrorsFound, bool &DoCTFErrorReport)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   June 1990
    //       MODIFIED       July 1994, LKL, cosmetic and improve execution time
    //                      Dec 1995, Apr 1996, RKS, cosmetic and clean-up changes, changes to allow proper
    //                       handling of resistive layers
    //                      June 2000, RKS, addition of QTFs (both 1- and 2-D solutions for constructions
    //                       with embedded/internal heat sources/sinks)
    //                      July 2010-August 2011, RKS, R-value only layer enhancement
    //       RE-ENGINEERED  June 1996, February 1997, August-October 1997, RKS; Nov 1999, LKL

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine serves as the main drive for the
    // calculation of Conduction Transfer Functions (CTFs)
    // using the state space method.

    // METHODOLOGY EMPLOYED:
    // The basic steps of this routine (which may be a little difficult
    // to decipher until another major revision is done) are:
    //   1. Determine if enough material info has been entered
    //   2. Determine whether construct is (a) all resistive,
    //      (b) the reverse of a previously calculated construct, or
    //      (c) neither (a) nor (b), i.e. a layer for which CTFs must
    //      be calculated.
    //   3. If the answer to 2 is (a), calculate the overall resistance
    //      and use this as the CTF (steady state conduction).
    //   4. If the answer to 2 is (b), transfer the CTFs for the reverse
    //      construction to the CTF arrays for this construct (reversing
    //      the inside and outside terms).
    //   5. If the answer to 2 is (c), calculate the CTFs using the state
    //      space method described below.
    // The state space method of calculating CTFs involves
    // applying a finite difference grid to a multilayered
    // building element and performing linear algebra on the
    // resulting system of equations (in matrix form).
    // CTFs must be calculated for non-reversed layers which
    // have an appreciable thermal mass.  A conversion from
    // SI units to English units is made due to concerns
    // about round off problems noted in earlier version of
    // this subroutine.

    // REFERENCES:
    // Seem, J.E.  "Modeling of Heat Transfer in Buildings",
    //  Department of Mechanical Engineering, University of
    //  Wisconsin-Madison, 1987.
    // Strand, R.K. "Testing Design Description for the CTF
    //  Calculation Code in BEST", BSO internal document,
    //  May/June 1996.
    // Strand, R.K. "Heat Source Transfer Functions and Their
    //  Applicatoin to Low Temperature Radiant Heating System",
    //  Ph.D. Dissertation, Department of Mechanical and
    //  Industrial Engineering, University of Illinois at
    //  Urbana-Champaign, 1995.

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr Real64 PhysPropLimit(1.0e-6); // Physical properties limit.
    // This is more or less the traditional value from BLAST.

    constexpr Real64 RValueLowLimit(1.0e-3); // Physical properties limit for R-value only layers
    // This value was based on trial and error related to CR 7791 where a
    // user had entered a "no insulation" layer with an R-value of 1.0E-05.
    // Some trial and error established this as a potential value though
    // there is no guarantee that this is a good value.

    constexpr int MinNodes(6); // Minimum number of state space nodes
    // per layer.  This value was chosen based on experience with IBLAST.

    constexpr Real64 MaxAllowedCTFSumError(0.01); // Allow a 1 percent
    // difference between the CTF series summations.  If the difference is
    // greater than this, then the coefficients will not yield a valid steady
    // state solution.

    constexpr Real64 MaxAllowedTimeStep = 7.0; // Sets the maximum allowed time step
    // for CTF calculations to be 7 hours.  This is done in response to some
    // rare situations where odd or faulty input will cause the routine to
    // go off and get some huge time step (in excess of 20 hours).  This value
    // is a compromise that does not really solve any input problems.  One run
    // indicated that 2 meters of concrete will result in a time step of slightly
    // more than 3 hours.  So, 7 hours was arbitrarily picked as a ceiling for
    // time steps so that an error message can be produced to warn the user
    // that something isn't right.  Note that the 7 hour limit does not guarantee
    // that problems won't exist and it does not necessarily avoid any problems
    // that interpolated temperature histories might cause.

    this->CTFCross.fill(0.0);
    this->CTFFlux.fill(0.0);
    this->CTFInside.fill(0.0);
    this->CTFOutside.fill(0.0);
    this->CTFSourceIn.fill(0.0);
    this->CTFSourceOut.fill(0.0);
    this->CTFTimeStep = 0.0;
    this->CTFTSourceOut.fill(0.0);
    this->CTFTSourceIn.fill(0.0);
    this->CTFTSourceQ.fill(0.0);
    this->CTFTUserOut.fill(0.0);
    this->CTFTUserIn.fill(0.0);
    this->CTFTUserSource.fill(0.0);
    this->NumHistories = 0;
    this->NumCTFTerms = 0;
    this->UValue = 0.0;

    if (!this->IsUsedCTF) {
        return;
    }

    Array1D<Real64> cp(Construction::MaxLayersInConstruct);              // Specific heat of a material layer
    Array1D<Real64> dl(Construction::MaxLayersInConstruct);              // Thickness of a material layer
    Array1D<Real64> dx(Construction::MaxLayersInConstruct);              // Distance between nodes in a particular material layer
    Array1D<Real64> lr(Construction::MaxLayersInConstruct);              // R value of a material layer
    Array1D_int Nodes(Construction::MaxLayersInConstruct);               // Array containing the number of nodes per layer
    Array1D_bool ResLayer(Construction::MaxLayersInConstruct);           // Set true if the layer must be handled as a resistive
    Array1D<Real64> rho(Construction::MaxLayersInConstruct);             // Density of a material layer
    Array1D<Real64> rk(Construction::MaxLayersInConstruct);              // Thermal conductivity of a material layer
    Array1D_int AdjacentResLayerNum(Construction::MaxLayersInConstruct); // Layers that are adjacent to each other which are resistive

    Real64 amatx;      // Intermediate calculation variable
    Real64 amatxx;     // Intermediate calculation variable
    Real64 amaty;      // Intermediate calculation variable
    Real64 BiggestSum; // Largest CTF series summation (maximum of SumXi, SumYi, and SumZi)
    Real64 cap;        // Thermal capacitance of a node (intermediate calculation)
    Real64 capavg;     // Thermal capacitance of a node (average value for a node at an interface)
    Real64 cnd;        // Total thermal conductance (1/Rtot) of the bldg element
    Real64 dtn;        // Intermediate calculation of the time step
    Real64 dxn;        // Intermediate calculation of nodal spacing
    Real64 dxtmp;      // Intermediate calculation variable ( = 1/dx/cap)
    Real64 dyn;        // Nodal spacing in the direction perpendicular to the main direction
    Real64 SumXi;      // Summation of all of the Xi terms (inside CTFs) for a construction
    Real64 SumYi;      // Summation of all of the Xi terms (cross CTFs) for a construction
    Real64 SumZi;      // Summation of all of the Xi terms (outside CTFs) for a construction

    Real64 DeltaTimestep; // zone timestep in seconds, for local check of properties

    this->CTFTimeStep = state.dataGlobal->TimeStepZone;
    Real64 rs = 0.0;
    int LayersInConstruct = 0;
    int NumResLayers = 0;
    ResLayer = false;
    AdjacentResLayerNum = 0; // Zero this out for each construct

    for (int Layer = 1; Layer <= this->TotLayers; ++Layer) { // Begin layer loop ...

        // Loop through all of the layers in the current construct. The purpose
        // of this loop is to define the thermal properties necessary to
        // calculate the CTFs.

        int CurrentLayer = this->LayerPoint(Layer);

        ++LayersInConstruct;

        // Obtain thermal properties from the Material derived type

        auto *thisMaterial = state.dataMaterial->materials(CurrentLayer);
        assert(thisMaterial != nullptr);

        dl(Layer) = thisMaterial->Thickness;
        rk(Layer) = thisMaterial->Conductivity;
        rho(Layer) = thisMaterial->Density;
        cp(Layer) = thisMaterial->SpecHeat; // Must convert
        // from kJ/kg-K to J/kg-k due to rk units

        if (this->SourceSinkPresent && !thisMaterial->WarnedForHighDiffusivity) {
            // check for materials that are too conductive or thin
            if ((rho(Layer) * cp(Layer)) > 0.0) {
                Real64 Alpha = rk(Layer) / (rho(Layer) * cp(Layer));
                if (Alpha > DataHeatBalance::HighDiffusivityThreshold) {
                    DeltaTimestep = state.dataGlobal->TimeStepZoneSec;
                    Real64 const ThicknessThreshold = std::sqrt(Alpha * DeltaTimestep * 3.0);
                    if (thisMaterial->Thickness < ThicknessThreshold) {
                        ShowSevereError(
                            state,
                            format(
                                "InitConductionTransferFunctions: Found Material that is too thin and/or too highly conductive, material name = {}",
                                thisMaterial->Name));
                        ShowContinueError(state,
                                          format("High conductivity Material layers are not well supported for internal source constructions, "
                                                 "material conductivity = {:.3R} [W/m-K]",
                                                 thisMaterial->Conductivity));
                        ShowContinueError(state, format("Material thermal diffusivity = {:.3R} [m2/s]", Alpha));
                        ShowContinueError(state,
                                          format("Material with this thermal diffusivity should have thickness > {:.5R} [m]", ThicknessThreshold));
                        if (thisMaterial->Thickness < DataHeatBalance::ThinMaterialLayerThreshold) {
                            ShowContinueError(state,
                                              format("Material may be too thin to be modeled well, thickness = {:.5R} [m]", thisMaterial->Thickness));
                            ShowContinueError(state,
                                              format("Material with this thermal diffusivity should have thickness > {:.5R} [m]",
                                                     DataHeatBalance::ThinMaterialLayerThreshold));
                        }
                        thisMaterial->WarnedForHighDiffusivity = true;
                    }
                }
            }
        }
        if (thisMaterial->Thickness > 3.0) {
            ShowSevereError(state, "InitConductionTransferFunctions: Material too thick for CTF calculation");
            ShowContinueError(state, format("material name = {}", thisMaterial->Name));
            ErrorsFound = true;
        }

        if (rk(Layer) <= PhysPropLimit) { // Thermal conductivity too small,
            // thus this must be handled as a resistive layer
            ResLayer(Layer) = true;
        } else {
            lr(Layer) = dl(Layer) / rk(Layer);
            ResLayer(Layer) = (dl(Layer) * std::sqrt(rho(Layer) * cp(Layer) / rk(Layer))) < PhysPropLimit;
        }

        // If not a resistive layer, nothing further is required
        // for this layer.

        if (ResLayer(Layer)) {                    // Resistive layer-check for R-value, etc.
            ++NumResLayers;                       // Increment number of resistive layers
            lr(Layer) = thisMaterial->Resistance; // User defined thermal resistivity
            if (lr(Layer) < RValueLowLimit) {     // User didn't define enough
                // parameters to calculate CTFs for a building element
                // containing this layer.

                ShowSevereError(state, format("InitConductionTransferFunctions: Material={}R Value below lowest allowed value", thisMaterial->Name));
                ShowContinueError(state, format("Lowest allowed value=[{:.3R}], Material R Value=[{:.3R}].", RValueLowLimit, lr(Layer)));
                ErrorsFound = true;

            } else { // A valid user defined R-value is available.
                // If this is either the first or last layer in the construction,
                // then assign other properties based on air at 1 atm, 300K.
                // Reference for air properties:  Incropera and DeWitt,
                // Introduction to Heat Transfer, Appendix A, Table A.4,
                // John Wiley & Sons, New York, 1985.
                // If this is not the first or last layer in the construction,
                // then use the "exact" approach to model a massless layer
                // based on the node equations for the state space method.

                if ((Layer == 1) || (Layer == this->TotLayers) || (!state.dataMaterial->materials(this->LayerPoint(Layer))->ROnly)) {
                    cp(Layer) = 1.007;
                    rho(Layer) = 1.1614;
                    rk(Layer) = 0.0263;
                    dl(Layer) = rk(Layer) * lr(Layer);
                } else {
                    cp(Layer) = 0.0;
                    rho(Layer) = 0.0;
                    rk(Layer) = 1.0;
                    dl(Layer) = lr(Layer);
                }
            }
        } // ... end of resistive layer determination IF-THEN block.
    } // ... end of layer loop.

    // If errors have been found, just return

    if (ErrorsFound) {
        return;
    }

    // Combine any adjacent resistive-only (no mass) layers together
    // to avoid a divide by zero error in the CTF calculations below.
    // Since the inner and outer layers cannot be resistive layers
    // (inner and outer layer still converted to equivalent air layer)
    // there can only be resistive layers adjacent to one another if
    // there are more than three total layers and more than one
    // resistive layer.
    if ((LayersInConstruct > 3) && (NumResLayers > 1)) {
        int NumAdjResLayers = 0;
        for (int Layer = 2; Layer <= LayersInConstruct - 2; ++Layer) {
            if ((ResLayer(Layer)) && (ResLayer(Layer + 1))) {
                ++NumAdjResLayers;
                // There is method to the next assignment statement.  As the layers get shifted, the layer
                // numbers will also shift.  Thus, we have to also shift which layer we are dealing with.
                AdjacentResLayerNum(NumAdjResLayers) = Layer + 1 - NumAdjResLayers;
            }
        }
        for (int AdjLayer = 1; AdjLayer <= NumAdjResLayers; ++AdjLayer) {
            int Layer = AdjacentResLayerNum(AdjLayer);
            // Double check to make sure we are in the right place...
            if ((ResLayer(Layer)) && (ResLayer(Layer + 1))) {
                // Shift layers forward after combining two adjacent layers.  Then
                // restart the do loop.
                cp(Layer) = 0.0;
                rho(Layer) = 0.0;
                rk(Layer) = 1.0;
                lr(Layer) += lr(Layer + 1);
                dl(Layer) = lr(Layer);
                --NumResLayers; // Combining layers so decrease number of resistive layers
                for (int Layer1 = Layer + 1; Layer1 <= LayersInConstruct - 1; ++Layer1) {
                    lr(Layer1) = lr(Layer1 + 1);
                    dl(Layer1) = dl(Layer1 + 1);
                    rk(Layer1) = rk(Layer1 + 1);
                    rho(Layer1) = rho(Layer1 + 1);
                    cp(Layer1) = cp(Layer1 + 1);
                    ResLayer(Layer1) = ResLayer(Layer1 + 1);
                }
                // Then zero out the layer that got shifted forward
                cp(LayersInConstruct) = 0.0;
                rho(LayersInConstruct) = 0.0;
                rk(LayersInConstruct) = 0.0;
                lr(LayersInConstruct) = 0.0;
                dl(LayersInConstruct) = 0.0;
                // Now reduce the number of layers in construct since merger is complete
                --LayersInConstruct;
                // Also adjust layers with source/sinks if two layers are merged
                if (this->SourceSinkPresent) {
                    --this->SourceAfterLayer;
                    --this->TempAfterLayer;
                }
            } else { // These are not adjacent layers and there is a logic flaw here (should not happen)
                ShowFatalError(state, format("Combining resistance layers failed for {}", this->Name));
                ShowContinueError(state, "This should never happen.  Contact EnergyPlus Support for further assistance.");
            }
        }
    }

    // Convert SI units to English.  In theory, conversion to English
    // units is not necessary; however, Russ Taylor noted that some
    // numerical problems when SI units were used and decided to continue
    // calculating CTFs in English units.

    for (int Layer = 1; Layer <= LayersInConstruct; ++Layer) { // Begin units conversion loop ...

        lr(Layer) *= DataConversions::CFU;
        dl(Layer) /= DataConversions::CFL;
        rk(Layer) /= DataConversions::CFK;
        rho(Layer) /= DataConversions::CFD;
        cp(Layer) /= (DataConversions::CFC * 1000.0);

    } // ... end of layer loop for units conversion.

    if (this->SolutionDimensions == 1) {
        dyn = 0.0;
    } else {
        dyn = (this->ThicknessPerpend / DataConversions::CFL) / double(NumOfPerpendNodes - 1);
    }

    // Compute total construct conductivity and resistivity.

    for (int Layer = 1; Layer <= LayersInConstruct; ++Layer) {
        rs += lr(Layer); // Resistances in series sum algebraically
    }

    cnd = 1.0 / rs; // Conductivity is the inverse of resistivity

    bool RevConst = false;

    if (LayersInConstruct > NumResLayers) {

        // One or more are not simple resistive layers so CTFs will have to be
        // calculated unless this is a reverse of a previously defined
        // this->

        // Check for reversed construction of interzone surfaces by checking
        // previous constructions for same number of layers as first indicator.

        // previously this loop would go from 1..currentConstructionIndex-1
        // instead of that, we'll loop through the list and stop when we get to the current construction
        // should be the same behavior, we're just checking it by address
        for (auto &otherConstruction : state.dataConstruction->Construct) {
            if (&otherConstruction == this) {
                break;
            }

            // If a source or sink is present in this construction, do not allow any
            // checks for reversed constructions, i.e., always force EnergyPlus to
            // calculate CTF/QTFs.  So, don't even check for reversed constructions.
            if (this->SourceSinkPresent) {
                break; // Constr DO loop
            }

            if (this->TotLayers == otherConstruction.TotLayers) { // Same number of layers--now | check for reversed construct.

                RevConst = true;

                for (int Layer = 1; Layer <= this->TotLayers; ++Layer) { // Begin layers loop ...

                    // RevConst is set to FALSE anytime a mismatch in materials is found.
                    // This will exit this DO immediately and go on to the next construct
                    // (if any remain).

                    int OppositeLayer = this->TotLayers - Layer + 1;

                    if (this->LayerPoint(Layer) != otherConstruction.LayerPoint(OppositeLayer)) {

                        RevConst = false;
                        break; // Layer DO loop
                    }

                } // ... end of layers loop.

                // If the reverse construction isn't used by any surfaces then the CTFs
                // still need to be defined.
                if (RevConst && !otherConstruction.IsUsedCTF) {
                    RevConst = false;
                }

                if (RevConst) { // Curent construction is a reverse of
                    // construction Constr.  Thus, CTFs do not need to be re-
                    // calculated.  Copy CTF info for construction Constr to
                    // construction ConstrNum.

                    this->CTFTimeStep = otherConstruction.CTFTimeStep;
                    this->NumHistories = otherConstruction.NumHistories;
                    this->NumCTFTerms = otherConstruction.NumCTFTerms;

                    // Transfer the temperature and flux history terms to CTF arrays.
                    // Loop through the number of CTF history terms ...
                    for (int HistTerm = 0; HistTerm <= this->NumCTFTerms; ++HistTerm) {

                        this->CTFInside[HistTerm] = otherConstruction.CTFOutside[HistTerm];
                        this->CTFCross[HistTerm] = otherConstruction.CTFCross[HistTerm];
                        this->CTFOutside[HistTerm] = otherConstruction.CTFInside[HistTerm];
                        if (HistTerm != 0) {
                            this->CTFFlux[HistTerm] = otherConstruction.CTFFlux[HistTerm];
                        }

                    } // ... end of CTF history terms loop.

                    break; // Constr DO loop

                } // ... end of reversed construction found block

            } // ... end of reversed construct (same number of layers) block.

        } // ... end of construct loop (check reversed--Constr)

        if (!RevConst) { // Calculate CTFs (non-reversed constr)

            // Estimate number of nodes each layer of the construct will require
            // and calculate the nodal spacing from that

            for (int Layer = 1; Layer <= LayersInConstruct; ++Layer) { // Begin loop thru layers ...

                // The calculation of dxn used here is based on a standard stability
                // criteria for explicit finite difference solutions.  This criteria
                // was chosen not because it is viewed to be correct, but rather for
                // lack of any better criteria at this time.  The use of a Fourier
                // number based criteria such as this is probably physically correct,
                // though the coefficient (2.0) may not be.

                // If this is a "resistive" layer, only need a single node
                if ((ResLayer(Layer)) && (Layer > 1) && (Layer < LayersInConstruct)) {
                    Nodes(Layer) = 1;
                    dx(Layer) = dl(Layer);
                } else {
                    dxn = std::sqrt(2.0 * (rk(Layer) / rho(Layer) / cp(Layer)) * this->CTFTimeStep);

                    int ipts1 = int(dl(Layer) / dxn); // number of nodes=thickness/spacing

                    // Limit the upper and lower bounds of the number of
                    // nodes to MaxCTFTerms and MinNodes respectively.

                    if (ipts1 > Construction::MaxCTFTerms) { // Too many nodes
                        Nodes(Layer) = Construction::MaxCTFTerms;
                    } else if (ipts1 < MinNodes) { // Too few nodes
                        Nodes(Layer) = MinNodes;
                    } else { // Calculated number of nodes ok
                        Nodes(Layer) = ipts1;
                    }

                    dx(Layer) = dl(Layer) / double(Nodes(Layer)); // calc node spacing
                }

            } // . .. end of layers in construction loop (calculating #nodes per layer)

            // Determine the total number of nodes (rcmax)

            this->rcmax = 0;
            for (int Layer = 1; Layer <= LayersInConstruct; ++Layer) {
                this->rcmax += Nodes(Layer);
            }

            // Nodes are placed throughout layers and at the interface between
            // layers.  As a result, the end layers share a node with the adjacent
            // layer-leaving one less node total for all layers.

            --this->rcmax;
            if (this->SolutionDimensions > 1) {
                this->rcmax *= NumOfPerpendNodes;
            }

            // This section no longer needed as rcmax/number of total nodes is allowed to float.
            // If reinstated, this node reduction section would have to be modified to account for
            // the possibility that a 2-D solution is potentially being performed.
            // Check to see if the maximum number of nodes for the construct has
            // been exceeded.  Reduce the nodes per layer if necessary, but only
            // if the number of nodes in a particular layer is greater than the
            // minimum node limit.

            //        DO WHILE (rcmax > MaxTotNodes)     ! Begin total node reduction loop ...

            //          rcmax = 0

            //          DO Layer = 1, LayersInConstruct   ! Begin layer node reduction ...

            //          ! If more nodes than the minimum limit for a layer, reduce the
            //          ! number of nodes.

            //            IF (Nodes(Layer) > MinNodes) THEN
            //              Nodes(Layer) = Nodes(Layer)-1
            //              dx(Layer) = dl(Layer)/DBLE(Nodes(Layer)) ! Recalc node spacing
            //            END IF

            //            rcmax = rcmax + Nodes(Layer) ! Recalculate total number of nodes

            //          END DO        ! ... end of layer loop for node reduction.

            //          rcmax = rcmax-1 ! See note above on counting rcmax

            //        END DO      ! ... end of total node reduction loop.

            // For constructions that have sources or sinks present, determine which
            // node the source/sink is applied at and also where the temperature
            // calculation has been requested.
            this->setNodeSourceAndUserTemp(Nodes);

            // "Adjust time step to ensure stability."  If the time step is too
            // small, it will result in too many history terms which can lead to
            // solution instability.  The method used here to determine whether or
            // not the time step will produce a stable solution is based on a pure
            // Fourier number calculation (Fo = 1) and has not proven to be
            // completely effective.  If too many history terms are calculated,
            // the time step is adjusted and the CTFs end up being recalculated
            // (see later code in this routine).

            dtn = 0.0;
            this->CTFTimeStep = 0.0;
            for (int Layer = 1; Layer <= LayersInConstruct; ++Layer) {
                if (Nodes(Layer) >= Construction::MaxCTFTerms) {
                    if (this->SolutionDimensions == 1) {
                        dtn = rho(Layer) * cp(Layer) * pow_2(dx(Layer)) / rk(Layer);
                    } else { // 2-D solution requested-->this changes length parameter in Fourier number calculation
                        dtn = rho(Layer) * cp(Layer) * (pow_2(dx(Layer)) + pow_2(dyn)) / rk(Layer);
                    }
                    if (dtn > this->CTFTimeStep) {
                        this->CTFTimeStep = dtn;
                    }
                }
            }

            // If the user defined time step is significantly different than the
            // calculated time step for this construct, then CTFTimeStep must be
            // revised.

            if (std::abs((state.dataGlobal->TimeStepZone - this->CTFTimeStep) / state.dataGlobal->TimeStepZone) > 0.1) {

                if (this->CTFTimeStep > state.dataGlobal->TimeStepZone) {

                    // CTFTimeStep larger than TimeStepZone:  Make sure TimeStepZone
                    // divides evenly into CTFTimeStep
                    this->NumHistories = int((this->CTFTimeStep / state.dataGlobal->TimeStepZone) + 0.5);
                    this->CTFTimeStep = state.dataGlobal->TimeStepZone * double(this->NumHistories);

                } else {

                    // CTFTimeStep smaller than TimeStepZone:  Set to TimeStepZone
                    this->CTFTimeStep = state.dataGlobal->TimeStepZone;
                    this->NumHistories = 1;
                }
            }

            // Calculate the CTFs using the state space method
            // outlined in Seem's dissertation.  The main matrices
            // AMat, BMat, CMat, and DMat must be derived from
            // applying a finite difference network to the layers of
            // each bldg element.

            // This section must continue looping until the CTFs
            // calculated here will produce a stable solution (less
            // history terms than MaxCTFTerms).

            // This first subsection calculates the elements of AMat
            // which characterizes the heat transfer inside the
            // building element.

            // Initialize loop control logical
            // Set after CTFs are calculated, based on whether there are too many CTF terms
            bool CTFConvrg = false;

            this->AExp.allocate(this->rcmax, this->rcmax);
            this->AExp = 0.0;
            this->AMat.allocate(this->rcmax, this->rcmax);
            this->AMat = 0.0;
            this->AInv.allocate(this->rcmax, this->rcmax);
            this->AInv = 0.0;
            this->IdenMatrix.allocate(this->rcmax, this->rcmax);
            this->IdenMatrix = 0.0;
            for (int ir = 1; ir <= this->rcmax; ++ir) {
                this->IdenMatrix(ir, ir) = 1.0;
            }
            this->e.dimension(this->rcmax, 0.0);
            this->Gamma1.allocate(3, this->rcmax);
            this->Gamma1 = 0.0;
            this->Gamma2.allocate(3, this->rcmax);
            this->Gamma2 = 0.0;
            this->s.allocate(3, 4, this->rcmax);
            this->s = 0.0;

            while (!CTFConvrg) { // Begin CTF calculation loop ...

                this->BMat(3) = 0.0;

                if (this->SolutionDimensions == 1) {

                    // Set up intermediate calculations for the first layer.
                    cap = rho(1) * cp(1) * dx(1);
                    cap *= 1.5; // For the first node, account for the fact that the
                    // half-node at the surface results in a "loss" of some
                    // thermal mass.  Therefore, for simplicity, include it
                    // at this node.  Same thing done at the last node...
                    dxtmp = 1.0 / dx(1) / cap;

                    this->AMat(1, 1) = -2.0 * rk(1) * dxtmp; // Assign the matrix values for the
                    this->AMat(2, 1) = rk(1) * dxtmp;        // first node.
                    this->BMat(1) = rk(1) * dxtmp;           // Assign non-zero value of BMat.

                    int Layer = 1; // Initialize the "layer" counter

                    int NodeInLayer = 2; // Initialize the node (in a layer) counter (already
                    // on the second node for the first layer

                    for (int Node = 2; Node <= this->rcmax - 1; ++Node) { // Begin nodes loop (includes all nodes except the
                        // first/last which have special equations) ...

                        if ((NodeInLayer == Nodes(Layer)) && (LayersInConstruct != 1)) { // For a node at
                            // the interface between two adjacent layers, the
                            // capacitance of the node must be calculated from the 2
                            // halves which may be made up of 2 different materials.

                            cap = (rho(Layer) * cp(Layer) * dx(Layer) + rho(Layer + 1) * cp(Layer + 1) * dx(Layer + 1)) * 0.5;

                            this->AMat(Node - 1, Node) = rk(Layer) / dx(Layer) / cap;                                      // Assign matrix
                            this->AMat(Node, Node) = -1.0 * (rk(Layer) / dx(Layer) + rk(Layer + 1) / dx(Layer + 1)) / cap; // values for | the current
                            this->AMat(Node + 1, Node) = rk(Layer + 1) / dx(Layer + 1) / cap;                              // node.

                            NodeInLayer = 0; // At an interface, reset nodes in layer counter
                            ++Layer;         // Also increment the layer counter

                        } else { // Standard node within any layer

                            cap = rho(Layer) * cp(Layer) * dx(Layer);          // Intermediate
                            dxtmp = 1.0 / dx(Layer) / cap;                     // calculations.
                            this->AMat(Node - 1, Node) = rk(Layer) * dxtmp;    // Assign matrix
                            this->AMat(Node, Node) = -2.0 * rk(Layer) * dxtmp; // values for the
                            this->AMat(Node + 1, Node) = rk(Layer) * dxtmp;    // current node.
                        }

                        ++NodeInLayer; // Increment nodes in layer counter
                        if (Node == this->NodeSource) {
                            this->BMat(3) = 1.0 / cap;
                        }

                    } // ... end of nodes loop.

                    // Intermediate calculations for the last node.
                    cap = rho(LayersInConstruct) * cp(LayersInConstruct) * dx(LayersInConstruct);
                    cap *= 1.5; // For the last node, account for the fact that the
                    // half-node at the surface results in a "loss" of some
                    // thermal mass.  Therefore, for simplicity, include it
                    // at this node.  Same thing done at the first node...
                    dxtmp = 1.0 / dx(LayersInConstruct) / cap;

                    this->AMat(this->rcmax, this->rcmax) = -2.0 * rk(LayersInConstruct) * dxtmp; // Assign matrix
                    this->AMat(this->rcmax - 1, this->rcmax) = rk(LayersInConstruct) * dxtmp;    // values for the
                    this->BMat(2) = rk(LayersInConstruct) * dxtmp;                               // last node.

                    this->CMat(1) = -rk(1) / dx(1);                                 // Compute the necessary elements
                    this->CMat(2) = rk(LayersInConstruct) / dx(LayersInConstruct);  // of all other
                    this->DMat(1) = rk(1) / dx(1);                                  // matrices for the state
                    this->DMat(2) = -rk(LayersInConstruct) / dx(LayersInConstruct); // space method

                } else { // 2-D solution requested (assign matrices appropriately)

                    // As with the 1-D solution, we are accounting for the thermal mass
                    // of the half-node at the surface by adding it to the first row
                    // of interior nodes at both sides of the construction.  This is not
                    // exact, but it does take all of the thermal mass into account.
                    amatx = rk(1) / (1.5 * rho(1) * cp(1) * dx(1) * dx(1));
                    amaty = rk(1) / (1.5 * rho(1) * cp(1) * dyn * dyn);

                    // FIRST ROW OF NODES: This first row within the first material layer
                    // is special in that it is exposed to a boundary condition.  Thus,
                    // the equations are slightly different.
                    // Note also that the first and last nodes in a row are slightly
                    // different from the rest since they are on an adiabatic plane in
                    // the direction perpendicular to the main direction of heat transfer.
                    this->AMat(1, 1) = -2.0 * (amatx + amaty);
                    this->AMat(2, 1) = 2.0 * amaty;
                    this->AMat(this->NumOfPerpendNodes + 1, 1) = amatx;

                    for (int Node = 2; Node <= this->NumOfPerpendNodes - 1; ++Node) {
                        this->AMat(Node - 1, Node) = amaty;
                        this->AMat(Node, Node) = -2.0 * (amatx + amaty);
                        this->AMat(Node + 1, Node) = amaty;
                        this->AMat(Node + this->NumOfPerpendNodes, Node) = amatx;
                    }

                    this->AMat(this->NumOfPerpendNodes, this->NumOfPerpendNodes) = -2.0 * (amatx + amaty);
                    this->AMat(this->NumOfPerpendNodes - 1, this->NumOfPerpendNodes) = 2.0 * amaty;
                    this->AMat(this->NumOfPerpendNodes + this->NumOfPerpendNodes, this->NumOfPerpendNodes) = amatx;

                    BMat(1) = amatx;

                    int Layer = 1;
                    int NodeInLayer = 2;
                    amatx = rk(1) / (rho(1) * cp(1) * dx(1) * dx(1)); // Reset these to the normal capacitance
                    amaty = rk(1) / (rho(1) * cp(1) * dyn * dyn);     // Reset these to the normal capacitance
                    assert(this->NumOfPerpendNodes > 0);              // Autodesk:F2C++ Loop setup assumption
                    int const Node_stop(this->rcmax + 1 - 2 * this->NumOfPerpendNodes);
                    for (int Node = this->NumOfPerpendNodes + 1; Node <= Node_stop; Node += this->NumOfPerpendNodes) {
                        // INTERNAL ROWS OF NODES: This is the majority of nodes which are all within
                        // a solid layer and not exposed to a boundary condition.
                        if ((LayersInConstruct == 1) || (NodeInLayer != Nodes(Layer))) {
                            // Single material row: This row of nodes are all contained within a material
                            // and thus there is no special considerations necessary.
                            if (NodeInLayer == 1) {
                                // These intermediate variables only need to be reassigned when a new layer is started.
                                // When this is simply another row of the same material, these have already been assigned correctly.
                                amatx = rk(Layer) / (rho(Layer) * cp(Layer) * dx(Layer) * dx(Layer));
                                amaty = rk(Layer) / (rho(Layer) * cp(Layer) * dyn * dyn);
                            }

                            // Note that the first and last layers in a row are slightly different
                            // from the rest since they are on an adiabatic plane in the direction
                            // perpendicular to the main direction of heat transfer.
                            this->AMat(Node, Node) = -2.0 * (amatx + amaty);
                            this->AMat(Node + 1, Node) = 2.0 * amaty;
                            this->AMat(Node - this->NumOfPerpendNodes, Node) = amatx;
                            this->AMat(Node + this->NumOfPerpendNodes, Node) = amatx;

                            for (int NodeInRow = 2; NodeInRow <= this->NumOfPerpendNodes - 1; ++NodeInRow) {
                                int Node2 = Node + NodeInRow - 1;
                                this->AMat(Node2 - 1, Node2) = amaty;
                                this->AMat(Node2, Node2) = -2.0 * (amatx + amaty);
                                this->AMat(Node2 + 1, Node2) = amaty;
                                this->AMat(Node2 - this->NumOfPerpendNodes, Node2) = amatx;
                                this->AMat(Node2 + this->NumOfPerpendNodes, Node2) = amatx;
                            }

                            int Node2 = Node - 1 + this->NumOfPerpendNodes;
                            this->AMat(Node2, Node2) = -2.0 * (amatx + amaty);
                            this->AMat(Node2 - 1, Node2) = 2.0 * amaty;
                            this->AMat(Node2 - this->NumOfPerpendNodes, Node2) = amatx;
                            this->AMat(Node2 + this->NumOfPerpendNodes, Node2) = amatx;

                        } else { // Row at a two-layer interface (half of node consists of one layer's materials
                            // and the other half consist of the next layer's materials)
                            capavg = 0.5 * (rho(Layer) * cp(Layer) * dx(Layer) + rho(Layer + 1) * cp(Layer + 1) * dx(Layer + 1));
                            amatx = rk(Layer) / (capavg * dx(Layer));
                            amatxx = rk(Layer + 1) / (capavg * dx(Layer + 1));
                            amaty = (rk(Layer) * dx(Layer) + rk(Layer + 1) * dx(Layer + 1)) / (capavg * dyn * dyn);

                            this->AMat(Node, Node) = -amatx - amatxx - 2.0 * amaty;
                            this->AMat(Node + 1, Node) = 2.0 * amaty;
                            this->AMat(Node - this->NumOfPerpendNodes, Node) = amatx;
                            this->AMat(Node + this->NumOfPerpendNodes, Node) = amatxx;

                            for (int NodeInRow = 2; NodeInRow <= this->NumOfPerpendNodes - 1; ++NodeInRow) {
                                int Node2 = Node + NodeInRow - 1;
                                this->AMat(Node2 - 1, Node2) = amaty;
                                this->AMat(Node2, Node2) = -amatx - amatxx - 2.0 * amaty;
                                this->AMat(Node2 + 1, Node2) = amaty;
                                this->AMat(Node2 - this->NumOfPerpendNodes, Node2) = amatx;
                                this->AMat(Node2 + this->NumOfPerpendNodes, Node2) = amatxx;
                            }

                            int Node2 = Node - 1 + this->NumOfPerpendNodes;
                            this->AMat(Node2, Node2) = -amatx - amatxx - 2.0 * amaty;
                            this->AMat(Node2 - 1, Node2) = 2.0 * amaty;
                            this->AMat(Node2 - this->NumOfPerpendNodes, Node2) = amatx;
                            this->AMat(Node2 + this->NumOfPerpendNodes, Node2) = amatxx;

                            if (Node == this->NodeSource) {
                                BMat(3) = 2.0 * double(this->NumOfPerpendNodes - 1) / capavg;
                            }
                            NodeInLayer = 0;
                            ++Layer;
                        }
                        ++NodeInLayer;
                    }

                    // LAST ROW OF NODES: Like the first row of nodes, this row is exposed to a boundary
                    // condition and thus has slightly modified nodal equations.

                    // As with the 1-D solution, we are accounting for the thermal mass
                    // of the half-node at the surface by adding it to the first row
                    // of interior nodes at both sides of the construction.  This is not
                    // exact, but it does take all of the thermal mass into account.
                    amatx /= 1.5;
                    amaty /= 1.5;

                    int Node = this->rcmax + 1 - this->NumOfPerpendNodes;
                    this->AMat(Node, Node) = -2.0 * (amatx + amaty);
                    this->AMat(Node + 1, Node) = 2.0 * amaty;
                    this->AMat(Node - this->NumOfPerpendNodes, Node) = amatx;

                    for (int thisNode = this->rcmax + 2 - this->NumOfPerpendNodes; thisNode <= this->rcmax - 1; ++thisNode) {
                        this->AMat(thisNode - 1, thisNode) = amaty;
                        this->AMat(thisNode, thisNode) = -2.0 * (amatx + amaty);
                        this->AMat(thisNode + 1, thisNode) = amaty;
                        this->AMat(thisNode - this->NumOfPerpendNodes, thisNode) = amatx;
                    }

                    this->AMat(this->rcmax, this->rcmax) = -2.0 * (amatx + amaty);
                    this->AMat(this->rcmax - 1, this->rcmax) = 2.0 * amaty;
                    this->AMat(this->rcmax - this->NumOfPerpendNodes, this->rcmax) = amatx;

                    this->BMat(2) = amatx;

                    this->CMat(1) = -rk(1) / dx(1) / double(this->NumOfPerpendNodes - 1);
                    this->CMat(2) = rk(LayersInConstruct) / dx(LayersInConstruct) / double(this->NumOfPerpendNodes - 1);

                    this->DMat(1) = rk(1) / dx(1) / double(this->NumOfPerpendNodes - 1);
                    this->DMat(2) = -rk(LayersInConstruct) / dx(LayersInConstruct) / double(this->NumOfPerpendNodes - 1);
                }

                // Calculation of the CTFs based on the state space
                // method.  This process involves finding the exponential
                // and inverse of AMat and using these results to
                // determine the CTFs.  The Gammas are an intermediate
                // calculations which are necessary before the CTFs can
                // be computed in TransFuncCoeffs.
                DisplayString(state, "Calculating CTFs for \"" + this->Name + "\"");

                //          CALL DisplayNumberAndString(ConstrNum,'Matrix exponential for Construction #')
                this->calculateExponentialMatrix(); // Compute exponential of AMat

                //          CALL DisplayNumberAndString(ConstrNum,'Invert Matrix for Construction #')
                this->calculateInverseMatrix(); // Compute inverse of AMat

                //          CALL DisplayNumberAndString(ConstrNum,'Gamma calculation for Construction #')
                this->calculateGammas();
                // Compute "gamma"s from AMat, AExp, and AInv

                //          CALL DisplayNumberAndString(ConstrNum,'Compute CTFs for Construction #')
                this->calculateFinalCoefficients(); // Compute CTFs

                // Now check to see if the number of transfer functions
                // is greater than MaxCTFTerms.  If it is, then increase the
                // time step and the number of history terms and
                // recalculate.  Whether or not it will be necessary to
                // recalculate the CTFs is controlled by this DO WHILE
                // loop and the logical CTFConvrg.

                CTFConvrg = true; // Assume solution convergence

                // If too many terms, then solution did not converge.  Increase the
                // number of histories and the time step.  Reset CTFConvrg to continue
                // the DO loop.
                if (this->NumCTFTerms > (Construction::MaxCTFTerms - 1)) {
                    ++this->NumHistories;
                    this->CTFTimeStep += state.dataGlobal->TimeStepZone;
                    CTFConvrg = false;
                }

                // If the number of terms is okay, then do a further check on the summation of
                // the various series summations.  In theory, Sum(Xi) = Sum(Yi) = Sum(Zi).  If
                // this is not the case, then the terms have not reached a valid solution, and
                // we need to increase the number of histories and the time step as above.
                if (CTFConvrg) {
                    SumXi = this->s0(2, 2);
                    SumYi = this->s0(1, 2);
                    SumZi = this->s0(1, 1);
                    for (int HistTerm = 1; HistTerm <= this->NumCTFTerms; ++HistTerm) {
                        SumXi += this->s(2, 2, HistTerm);
                        SumYi += this->s(1, 2, HistTerm);
                        SumZi += this->s(1, 1, HistTerm);
                    }
                    SumXi = std::abs(SumXi);
                    SumYi = std::abs(SumYi);
                    SumZi = std::abs(SumZi);
                    BiggestSum = max(SumXi, SumYi, SumZi);
                    if (BiggestSum > 0.0) {
                        if (((std::abs(SumXi - SumYi) / BiggestSum) > MaxAllowedCTFSumError) ||
                            ((std::abs(SumZi - SumYi) / BiggestSum) > MaxAllowedCTFSumError)) {
                            ++this->NumHistories;
                            this->CTFTimeStep += state.dataGlobal->TimeStepZone;
                            CTFConvrg = false;
                        }
                    } else { // Something terribly wrong--the surface has no CTFs, not even an R-value
                        ShowFatalError(state, format("Illegal construction definition, no CTFs calculated for {}", this->Name));
                    }
                }

                // Once the time step has reached a certain point, it is highly likely that
                // there is either a problem with the input or the solution.  This should
                // be extremely rare since other checks should flag most bad user input.
                // Thus, if the time step reaches a certain point, error out and let the
                // user know that something needs to be checked in the input file.
                if (this->CTFTimeStep >= MaxAllowedTimeStep) {
                    ShowSevereError(state, format("CTF calculation convergence problem for Construction=\"{}\".", this->Name));
                    ShowContinueError(state, "...with Materials (outside layer to inside)");
                    ShowContinueError(state, format("(outside)=\"{}\"", state.dataMaterial->materials(this->LayerPoint(1))->Name));
                    for (int Layer = 2; Layer <= this->TotLayers; ++Layer) {
                        if (Layer != this->TotLayers) {
                            ShowContinueError(state, format("(next)=\"{}\"", state.dataMaterial->materials(this->LayerPoint(Layer))->Name));
                        } else {
                            ShowContinueError(state, format("(inside)=\"{}\"", state.dataMaterial->materials(this->LayerPoint(Layer))->Name));
                        }
                    }
                    ShowContinueError(state,
                                      "The Construction report will be produced. This will show more details on Constructions and their materials.");
                    ShowContinueError(state, "Attempts will be made to complete the CTF process but the report may be incomplete.");
                    ShowContinueError(state, "Constructs reported after this construction may appear to have all 0 CTFs.");
                    ShowContinueError(state, "The potential causes of this problem are related to the input for the construction");
                    ShowContinueError(state, "listed in the severe error above.  The CTF calculate routine is unable to come up");
                    ShowContinueError(state, "with a series of CTF terms that have a reasonable time step and this indicates an");
                    ShowContinueError(state, "error.  Check the definition of this construction and the materials that make up");
                    ShowContinueError(state, "the construction.  Very thin, highly conductive materials may cause problems.");
                    ShowContinueError(state, "This may be avoided by ignoring the presence of those materials since they probably");
                    ShowContinueError(state, "do not effect the heat transfer characteristics of the construction.  Highly");
                    ShowContinueError(state, "conductive or highly resistive layers that are alternated with high mass layers");
                    ShowContinueError(state, "may also result in problems.  After confirming that the input is correct and");
                    ShowContinueError(state, "realistic, the user should contact the EnergyPlus support team.");
                    DoCTFErrorReport = true;
                    ErrorsFound = true;
                    break;
                    //            CALL ShowFatalError(state, 'Program terminated for reasons listed (InitConductionTransferFunctions) ')
                }

            } // ... end of CTF calculation loop.

        } // ... end of IF block for non-reversed constructs.

    } else { // Construct has only resistive layers (no thermal mass).
        // CTF calculation not necessary, overall resistance
        // (R-value) is all that is needed.

        // Set time step for construct to user time step and the number of
        // inter-time step interpolations to 1
        this->CTFTimeStep = state.dataGlobal->TimeStepZone;
        this->NumHistories = 1;
        this->NumCTFTerms = 1;

        this->s0(1, 1) = cnd;  // CTFs for current time
        this->s0(2, 1) = -cnd; // step are set to the
        this->s0(1, 2) = cnd;  // overall conductance
        this->s0(2, 2) = -cnd; // of the construction.

        this->e.allocate(1);
        this->e = 0.0;
        this->s.allocate(2, 2, 1);
        this->s = 0.0;
        this->s(1, 1, 1) = 0.0; // CTF temperature
        this->s(2, 1, 1) = 0.0; // and flux
        this->s(1, 2, 1) = 0.0; // history terms
        this->s(2, 2, 1) = 0.0; // are all
        this->e(1) = 0.0;       // zero.

        if (this->SourceSinkPresent) {
            ShowSevereError(state, format("Sources/sinks not allowed in purely resistive constructions --> {}", this->Name));
            ErrorsFound = true;
        }

        RevConst = false; // In the code that follows, handle a resistive
        // layer as a non-reversed this->

    } // ... end of resistive construction IF block.

    // Transfer the CTFs to the storage arrays for all non-reversed
    // constructions.  This transfer was done earlier in the routine for
    // reversed constructions.

    if (!RevConst) { // If this is either a new construction or a non-
        // reversed construction, the CTFs must be stored
        // in the proper arrays.  If this is a reversed
        // construction, nothing further needs to be done.

        // Copy the CTFs into the storage arrays, converting them back to SI
        // units in the process.  First the "zero" terms and then the history terms...
        this->CTFOutside[0] = this->s0(1, 1) * DataConversions::CFU;
        this->CTFCross[0] = this->s0(1, 2) * DataConversions::CFU;
        this->CTFInside[0] = -this->s0(2, 2) * DataConversions::CFU;
        if (this->SourceSinkPresent) {
            // QTFs...
            this->CTFSourceOut[0] = this->s0(3, 1);
            this->CTFSourceIn[0] = this->s0(3, 2);
            // QTFs for temperature calculation at source/sink location
            this->CTFTSourceOut[0] = this->s0(1, 3);
            this->CTFTSourceIn[0] = this->s0(2, 3);
            this->CTFTSourceQ[0] = this->s0(3, 3) / DataConversions::CFU;
            if (this->TempAfterLayer != 0) {
                // QTFs for user specified interior temperature calculations...
                this->CTFTUserOut[0] = this->s0(1, 4);
                this->CTFTUserIn[0] = this->s0(2, 4);
                this->CTFTUserSource[0] = this->s0(3, 4) / DataConversions::CFU;
            }
        }

        for (int HistTerm = 1; HistTerm <= this->NumCTFTerms; ++HistTerm) {
            // "REGULAR" CTFs...
            this->CTFOutside[HistTerm] = this->s(1, 1, HistTerm) * DataConversions::CFU;
            this->CTFCross[HistTerm] = this->s(1, 2, HistTerm) * DataConversions::CFU;
            this->CTFInside[HistTerm] = -this->s(2, 2, HistTerm) * DataConversions::CFU;
            if (HistTerm != 0) {
                this->CTFFlux[HistTerm] = -e(HistTerm);
            }
            if (this->SourceSinkPresent) {
                // QTFs...
                this->CTFSourceOut[HistTerm] = this->s(3, 1, HistTerm);
                this->CTFSourceIn[HistTerm] = this->s(3, 2, HistTerm);
                // QTFs for temperature calculation at source/sink location
                this->CTFTSourceOut[HistTerm] = this->s(1, 3, HistTerm);
                this->CTFTSourceIn[HistTerm] = this->s(2, 3, HistTerm);
                this->CTFTSourceQ[HistTerm] = this->s(3, 3, HistTerm) / DataConversions::CFU;
                if (this->TempAfterLayer != 0) {
                    // QTFs for user specified interior temperature calculations...
                    this->CTFTUserOut[HistTerm] = this->s(1, 4, HistTerm);
                    this->CTFTUserIn[HistTerm] = this->s(2, 4, HistTerm);
                    this->CTFTUserSource[HistTerm] = this->s(3, 4, HistTerm) / DataConversions::CFU;
                }
            }
        }

    } // ... end of the reversed construction IF block.

    this->UValue = cnd * DataConversions::CFU;

    if (allocated(this->AExp)) {
        this->AExp.deallocate();
    }
    if (allocated(this->AMat)) {
        AMat.deallocate();
    }
    if (allocated(this->AInv)) {
        this->AInv.deallocate();
    }
    if (allocated(this->IdenMatrix)) {
        this->IdenMatrix.deallocate();
    }
    if (allocated(this->e)) {
        this->e.deallocate();
    }
    if (allocated(this->Gamma1)) {
        this->Gamma1.deallocate();
    }
    if (allocated(this->Gamma2)) {
        this->Gamma2.deallocate();
    }
    if (allocated(this->s)) {
        this->s.deallocate();
    }
}

void ConstructionProps::calculateExponentialMatrix()
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   June 1990
    //       MODIFIED       Dec 1995, Apr 1996, RKS; June 2000 RKS
    //       RE-ENGINEERED  June 1996, RKS; Nov 1999, LKL;

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes the exponential matrix exp(AMat*delt) for
    // use in the state space method for the calculation of CTFs.

    // METHODOLOGY EMPLOYED:
    // Uses the method of Taylor expansion combined with scaling and
    // squaring to most efficiently compute the exponential matrix.  The
    // steps in the procedure are outlined in Seem's dissertation in
    // Appendix A, page 128.  Exponential matrix multiplication modified
    // to take advantage of the characteristic form of AMat.  AMat starts
    // out as a tri-diagonal matrix.  Each time AMat is raised to a higher
    // power two extra non-zero diagonals are added.  ExponMatrix now
    // recognizes this.  This should speed up the calcs somewhat.  Also, a
    // new cut-off criteria based on the significant figures of double-
    // precision variables has been added.  The main loop for higher powers
    // of AMat is now stopped whenever these powers of AMat will no longer
    // add to the summation (AExp) instead ofstopping potentially at the
    // artifical limit of AMat**100.

    // REFERENCES:
    // Seem, J.E.  "Modeling of Heat Transfer in Buildings",
    //  Department of Mechanical Engineering, University of
    //  Wisconsin-Madison, 1987.
    // Strand, R.K. "Testing Design Description for the CTF
    //  Calculation Code in BEST", BSO internal document,
    //  May/June 1996.

    constexpr Real64 DPLimit(1.0e-20);
    // This argument is nice, but not sure it's accurate -- LKL Nov 1999.
    // Parameter set to the significant figures limit of double
    // precision variables plus a safety factor.- The argument for setting this parameter to 1E-20 involves the
    // number of significant figures for REAL(r64) variables which is 16 and the largest power to which
    // AMat will be raised which is 100.  This would be a factor of 1E-18.  A factor of "safety" of another 100
    // arrives at the value chosen.  It is argued that if one number is 1E-16 larger than a second number, then
    // adding the second to the first will not effect the first.  However, on the conservative side, there could
    // be up to 100 numbers which might, added together, still could effect the original number.  Each
    // successive power of AMat will have terms smaller than the previous power.  Thus, when the ratio between
    // the terms of the latest power of AMat and the total (AExp) is less than DPLim, all further powers of
    // AMat will have absolutely no effect on the REAL(r64) value of AExp.  Thus, there is no need to
    // continue the calculation.  In effect, AExp has "converged".  In REAL(r64)ity, 1E-16 would probably guarantee
    // convergence since AMat terms drop off quickly, but the extra powers allows for differences between
    // computer platforms.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 AMatRowNorm;    // Row norm for AMat
    Real64 AMatRowNormMax; // Largest row norm for AMat
    Array2D<Real64> AMat1; // AMat factored by (delt/2^k)
    Array2D<Real64> AMato; // AMat raised to the previous power (power of AMat1-1)
    Array2D<Real64> AMatN; // Current value of AMat raised to power n (n = 1,2...)
    Real64 CheckVal;       // Used to avoid possible overflow from Double->REAL(r64)->Integer
    Real64 fact;           // Intermediate calculation variable (delt/2^k)
    int i;                 // Loop counter
    int ic;                // Loop counter
    int ict;               // Loop counter
    int idm;               // Loop counter
    int ir;                // Loop counter
    int isq;               // Loop counter
    int j;                 // Loop counter
    int k;                 // Power of 2 which is used to factor AMat
    int l;                 // Theoretical power to which the A matrix must be
    // raised to accurately calculate the exponential matrix
    // when exponential calculation loop can be exited (i.e.
    // the significant figure limit for REAL(r64)
    // variables reached)

    AMat1.allocate(this->rcmax, this->rcmax);
    AMato.allocate(this->rcmax, this->rcmax);
    AMatN.allocate(this->rcmax, this->rcmax);

    // Subroutine initializations.  AMat is assigned to local variable AMat1 to
    // avoid the corruption of the original AMat 2-d array.
    AMat1 = AMat;

    //  Other arrays are initialized to zero.
    this->AExp = 0.0;
    AMato = 0.0;
    AMatN = 0.0;

    // Step 1, page 128 (Seem's thesis):  Compute the matrix row norm.
    // See equation (A.3) which states that the matrix row norm is the
    // maximum summation of the elements in a row of AMat multiplied by
    // the time step.

    // Note With change to row-major arrays "row" here now means "column"

    AMatRowNormMax = 0.0; // Start of Step 1 ...

    for (i = 1; i <= this->rcmax; ++i) {

        AMatRowNorm = 0.0;
        for (j = 1; j <= this->rcmax; ++j) {
            AMatRowNorm += std::abs(AMat1(j, i));
        }

        AMatRowNorm *= this->CTFTimeStep;

        AMatRowNormMax = max(AMatRowNormMax, AMatRowNorm);

    } // ... end of Step 1.

    // Step 2, page 128:  Find smallest integer k such that
    // AMatRowNormMax< = 2^k

    k = int(std::log(AMatRowNormMax) / std::log(2.0)) + 1; // Autodesk:Num Handle AMatRowNormMax=0

    // Step 3, page 128:  Divide (AMat*delt) by 2^k.  This section of code
    // takes advantage of the fact that AMat is tridiagonal.  Thus, it
    // only factors the elements of the AMat that are known to be non-zero.

    fact = this->CTFTimeStep / std::pow(2.0, k); // Start of Step 3 ...
    AMat1 *= fact;                               // ... end of Step 3.

    // Step 4, page 128:  Calculate l, the highest power to which AMat
    // must be taken theoretically to accurately calculate its exponential.
    // This is based on a paper by Cadzow and Martens ("Discrete-Time and
    // Computer Control Systems",Prentice-Hall, pp. 389-390, 1970).  This
    // number is now used as the maximum power to which AMat must be
    // raised in order to calculate the exponential matrix.  A new cut-off
    // criteria based on the number of significant figures in a double-
    // precision variable is used as a more practical limit on the
    // exponentiation algorithm.  One thing to note is that the EnergyPlus
    // code is slightly different here than what is presented in Seem's
    // dissertation.  In Seem's dissertation, AMatRowNormMax (delta, the
    // timestep, is already factored into this term above) is divided by
    // 2^k with k defined in the code above.  Dividing AMatRowNormMax by
    // 2^k would have the net effect of decreasing the value of
    // AMatRowNormMax and thus potentially CheckVal as well.  l, the integer
    // value of CheckVal, is used to define the number of terms in the
    // exponential matrix of AMat that are required for an accurate estimation
    // of that matrix.  In practice, additional terms probably won't have
    // a large effect but in general should make the calculation MORE accurate.
    // Also, if the new terms are too small, then as noted above the cut-off
    // criteria will not use them.

    CheckVal = min(3.0 * AMatRowNormMax + 6.0, 100.0);
    l = int(CheckVal);

    // Step 5, page 128:  Calculate the exponential.  First, add the
    // linear term to the identity matrix.
    this->AExp = AMat1 + this->IdenMatrix; // Start of Step 5 ...

    // Now, add successive terms to the expansion as per the standard
    // exponential formula.  AMato contains the last "power" of AMat
    // which saves the program from having to remultiply the entire power
    // of AMat each time.  Since this is still the linear power of AMat,
    // AMat1 is still tridiagonal in nature.
    AMato = AMat1;

    i = 1; // Initialize the counter for the following DO loop

    // The following DO WHILE loop continues to raise AMat to successive
    // powers and add it to the exponential matrix (AExp).
    while (i < l) { // Begin power raising loop ...

        ++i;                     // Increment the loop counter
        bool SigFigLimit = true; // Set the significant factor limit flag

        for (ir = 1; ir <= this->rcmax; ++ir) { // Begin matrix multiplication loop ...
            // The following matrix multiplication could be "optimized" since
            // for one-dimensional heat transfer AMat is 3-diagonal, AMat squared
            // is 5-diagonal, etc.  However, the code can be much simpler if we
            // ignore this fact and just do a generic matrix multiplication.
            // For 2-D heat transfer, the number of off-diagonal non-zero terms
            // is slightly more complicated as well.
            for (ic = 1; ic <= this->rcmax; ++ic) {
                AMatN(ic, ir) = 0.0;
                for (ict = 1; ict <= this->rcmax; ++ict) {
                    // Make sure the next term won't cause an underflow.  If it will end up being
                    // so small as to go below TinyLimit, then ignore it since it won't add anything
                    // to AMatN anyway.
                    if (std::abs(AMat1(ic, ict)) > Constant::rTinyValue) {
                        if (std::abs(AMato(ict, ir)) > std::abs(double(i) * Constant::rTinyValue / AMat1(ic, ict))) {
                            AMatN(ic, ir) += AMato(ict, ir) * AMat1(ic, ict) / double(i);
                        }
                    }
                }
            }
        } // ... end of matrix multiplication loop.

        // Update AMato and AExp matrices
        AMato = AMatN;
        this->AExp += AMato;

        // The next DO loop tests the significant figures limit criteria to
        // see if any values in AExp are still changing appreciably.
        for (ir = 1; ir <= this->rcmax; ++ir) {
            for (ic = 1; ic <= this->rcmax; ++ic) {
                // Test of limit criteria:
                if (std::abs(this->AExp(ic, ir)) > Constant::rTinyValue) { // Next line divides by AExp entry so it
                    // must be checked to avoid dividing by zero.
                    // If the ratio between any current element in the power
                    // of AMat and its corresponding element in AExp is
                    // greater than the number which might effect the overall
                    // exponential matrix based on stability criteria, then
                    // continue raising AMat to another power (SigFigLimit = false).

                    if (std::abs(AMato(ic, ir) / this->AExp(ic, ir)) > DPLimit) {
                        SigFigLimit = false;
                        break; // DO loop (anytime SigFigLimit is false, AMat must continue to be raised another power)
                    }

                } else { // There are still elements of AExp which are zero, so
                    // the raising of AMat to higher powers should continue.

                    SigFigLimit = false;
                    break; // DO loop (anytime SigFigLimit is false, AMat must continue to be raised another power)
                }
            }
            if (!SigFigLimit) {
                break; // DO loop (anytime SigFigLimit is false, AMat must continue to be raised another power)
            }
        }

        // Compute next term, only if necessary.  If SigFigLimit is still true,
        // then all of the new terms being added to AExp are too small to
        // affect it.  Thus, there is no need to continue this do loop further.

        if (SigFigLimit) {
            i = 100; // SigFigLimit is still true, set i to maximum possible
        }
        // value of l (100).

    } // ... end of power raising loop and Step 5.

    // Step 6, page 128:
    // Square AExp "k times" to obtain the actual exponential matrix
    // (remember that AExp was scaled earlier in this routine).

    for (isq = 1; isq <= k; ++isq) { // Begin squaring DO loop and Step 6 ...

        // Use AMato to store the old values of AExp
        AMato = this->AExp;
        bool Backup = true; // Used when numerics get to small in Exponentiation
        this->AExp = 0.0;

        // Multiply the old value of AExp (AMato) by itself and store in AExp.
        for (ir = 1; ir <= this->rcmax; ++ir) {
            for (ic = 1; ic <= this->rcmax; ++ic) {
                for (idm = 1; idm <= this->rcmax; ++idm) {
                    if (std::abs(AMato(idm, ir) * AMato(ic, idm)) > Constant::rTinyValue) {
                        this->AExp(ic, ir) += AMato(idm, ir) * AMato(ic, idm);
                        Backup = false;
                    }
                }
            }
        }
        // Backup is true when every item of AExp didnt pass the TinyLimit test
        if (Backup) {
            this->AExp = AMato;
            break;
        }

    } // ... end of squaring loop and Step 6.

    AMat1.deallocate();
    AMato.deallocate();
    AMatN.deallocate();
}

void ConstructionProps::calculateInverseMatrix()
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Dec 1995
    //       MODIFIED       June 2000 RKS (made routine generic to allow for 2-D solutions)
    //       RE-ENGINEERED  June 1996, February 1997 RKS

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes the inverse of AMat for use
    // in the calculation of the CTFs.

    // METHODOLOGY EMPLOYED:
    // Uses row elimination to zero the off-diagonal terms of
    // AMat while performing the same operations on another
    // matrix which starts as the identity matrix.  Once AMat
    // has been converted to an identity matrix(I), the other
    // matrix which started as the I will then be the inverse
    // of A.  This algorithm has been customized for a
    // tri-diagonal matrix.

    // REFERENCES:
    // Any linear algebra test (this is a generic routine).

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array2D<Real64> AMat1; // Intermediate calculation matrix equivalent at first to AMat

    // Subroutine initializations ...
    AMat1.allocate(this->rcmax, this->rcmax);

    AMat1 = AMat;                  // Set AMat1 = AMat to avoid AMat changes
    this->AInv = this->IdenMatrix; // Set AInv to Identity Matrix

    // Use Gaussian elimination to zero all of the elements of AMat left
    // of the diagonal.
    // This DO loop will cycle through each of the rows in AMat except the
    // last row which is handled later because it does not have to be used
    // to eliminate any other rows.  The index ir is the current row
    // number and also the column of the current diagonal element.

    for (int ir = 1; ir <= this->rcmax - 1; ++ir) { // Begin forward elimination loop ...

        // Factor all of the elements of the row being used to zero the next
        // row in both AMat and AInv by the diagonal element of this row.
        // We should only need to factor the elements to the right of the
        // diagonal since those to the right of it should be zero.
        for (int ic = ir + 1; ic <= this->rcmax; ++ic) {
            AMat1(ic, ir) /= AMat1(ir, ir);
        }

        // In the forward elimination process, all the elements in AInv to the
        // right of the diagonal are zero so they do not need to be factored.
        for (int ic = 1; ic <= ir; ++ic) {
            this->AInv(ic, ir) /= AMat1(ir, ir);
        }

        AMat1(ir, ir) = 1.0; // By definition, the diagonal of AMat is now 1.

        // Use this factored row to eliminate the off-diagonal element of the
        // rows below the current one (ir)...

        for (int irr = ir + 1; irr <= this->rcmax; ++irr) { // Start of row reduction loop...

            for (int ic = ir + 1; ic <= this->rcmax; ++ic) {
                AMat1(ic, irr) -= AMat1(ir, irr) * AMat1(ic, ir);
            }

            // Now, determine the effect on the next row of AInv.  Again, all of
            // the elements in AInv to the right of the diagonal are zero, so they
            // can be ignored.

            for (int ic = 1; ic <= ir; ++ic) {
                this->AInv(ic, irr) -= AMat1(ir, irr) * this->AInv(ic, ir);
            }

            AMat1(ir, irr) = 0.0; // By definition, the element to the left of the
            // diagonal in the next row of AMat is now zero.

        } // ...end of row reduction loop

    } // ... end of the forward elimination loop.

    // Factor the last row of AInv by the current value of the last
    // diagonal element of AMat. After this is done, all of the diagonal
    // elements of AMat are unity and all of the elements in AMat left of
    // the diagonal are zero.

    for (int ic = 1; ic <= this->rcmax; ++ic) {
        this->AInv(ic, this->rcmax) /= AMat1(this->rcmax, this->rcmax);
    }
    AMat1(this->rcmax, this->rcmax) = 1.0;

    // Now, use back substitution to eliminate the elements to the right
    // of the diagonal in AMat.  The procedure is similar to the forward
    // elimination process except that we only have to operate on AInv,
    // though now all of the columns of AInv may be non-zero.

    // This DO loop will cycle through the remaining rows which are not
    // yet diagonalized in reverse order.  Note that the only effect on
    // AMat is that the off-diagonal element is zeroed.  The diagonal
    // (which has already been set to unity) is not effected by this row
    // elimination process.
    // In the following code ir is the column being zeroed and irr is the
    // row being worked on

    for (int ir = this->rcmax; ir >= 2; --ir) { // Begin reverse elimination loop ...
        for (int irr = 1; irr <= ir - 1; ++irr) {
            for (int ic = 1; ic <= this->rcmax; ++ic) {
                this->AInv(ic, irr) -= AMat1(ir, irr) * this->AInv(ic, ir);
            }
            AMat1(ir, irr) = 0.0;
        }
    } // ... end of reverse elimination loop.

    // At this point, AMat1 is equal to the identity matrix (I)
    // and AInv is equal to the inverse of AMat.

    AMat1.deallocate();
}

void ConstructionProps::calculateGammas()
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   June 1990
    //       RE-ENGINEERED  July 1996, RKS

    // PURPOSE OF THIS SUBROUTINE:
    // Compute gammas as defined in Seem's dissertation.
    // Runs as a subroutine of the conduction transfer
    // function solver (InitializeCTFs).

    // METHODOLOGY EMPLOYED:
    // Determine the Gamma1 and Gamma2 based on the results
    // from the ExponMatrix and InvertMatrix subroutines.
    // This routine is specialized to take advantage of the
    // fact that most of BMat consists of zeroes.

    // REFERENCES:
    // The state space method of calculating CTFs is
    // outlined in the doctoral dissertation of John Seem,
    // "Modeling of Heat Transfer in Buildings", Department
    // of Mechanical Engineering, University of Wisconsin-
    // Madison, 1987.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array2D<Real64> ATemp; // Intermediate variable equal to AExp - I

    // Compute Gamma1 from equation (2.1.12) in Seem's dissertation which
    // states that:  Gamma1  =  [AInv] * ([AExp]-[I]) * [BMat]
    // noting that BMat contains only the non-zero values of the B Matrix.

    ATemp.allocate(this->rcmax, this->rcmax);
    ATemp = this->AExp - this->IdenMatrix;
    Gamma1 = 0.0;

    for (int i = 1; i <= this->rcmax; ++i) {

        for (int is1 = 1; is1 <= this->rcmax; ++is1) {

            if (this->SolutionDimensions == 1) {
                this->Gamma1(1, i) += this->AInv(is1, i) * ATemp(1, is1) * this->BMat(1);
                this->Gamma1(2, i) += this->AInv(is1, i) * ATemp(this->rcmax, is1) * this->BMat(2);
            } else { // SolutionDimensions = 2
                for (int SurfNode = 1; SurfNode <= this->NumOfPerpendNodes; ++SurfNode) {
                    this->Gamma1(1, i) += this->AInv(is1, i) * ATemp(SurfNode, is1) * this->BMat(1);
                    this->Gamma1(2, i) += this->AInv(is1, i) * ATemp(this->rcmax + 1 - SurfNode, is1) * this->BMat(2);
                }
            }

            if (this->NodeSource > 0) {
                this->Gamma1(3, i) += this->AInv(is1, i) * ATemp(this->NodeSource, is1) * this->BMat(3);
            }
        }
    }

    ATemp.deallocate();
    // Compute Gamma2 from equation (2.1.13) in Seem's dissertation which
    // states that:  Gamma2  =  [AInv] * ([Gamma1]/delt - [BMat])
    // again noting that BMat contains only the non-zero values of B.
    Gamma2 = 0.0;

    for (int i = 1; i <= this->rcmax; ++i) {

        for (int j = 1; j <= 3; ++j) {

            for (int is1 = 1; is1 <= this->rcmax; ++is1) {

                if (this->SolutionDimensions == 1) {
                    if ((j == 1) && (is1 == 1)) {
                        this->Gamma2(j, i) += this->AInv(is1, i) * (this->Gamma1(j, is1) / this->CTFTimeStep - this->BMat(1));
                    } else if ((j == 2) && (is1 == this->rcmax)) {
                        this->Gamma2(j, i) += this->AInv(is1, i) * (this->Gamma1(j, is1) / this->CTFTimeStep - this->BMat(2));
                    } else if ((j == 3) && (is1 == this->NodeSource)) {
                        this->Gamma2(j, i) += this->AInv(is1, i) * (this->Gamma1(j, is1) / this->CTFTimeStep - this->BMat(3));
                    } else { // the element of the actual BMat is zero
                        this->Gamma2(j, i) += this->AInv(is1, i) * (this->Gamma1(j, is1) / this->CTFTimeStep);
                    }
                } else { // SolutionDimensions = 2
                    if ((j == 1) && ((is1 >= 1) && (is1 <= this->NumOfPerpendNodes))) {
                        this->Gamma2(j, i) += this->AInv(is1, i) * (this->Gamma1(j, is1) / this->CTFTimeStep - this->BMat(1));
                    } else if ((j == 2) && ((is1 <= this->rcmax) && (is1 >= this->rcmax + 1 - this->NumOfPerpendNodes))) {
                        this->Gamma2(j, i) += this->AInv(is1, i) * (this->Gamma1(j, is1) / this->CTFTimeStep - this->BMat(2));
                    } else if ((j == 3) && (is1 == this->NodeSource)) {
                        this->Gamma2(j, i) += this->AInv(is1, i) * (this->Gamma1(j, is1) / this->CTFTimeStep - this->BMat(3));
                    } else { // the element of the actual BMat is zero
                        this->Gamma2(j, i) += this->AInv(is1, i) * (this->Gamma1(j, is1) / this->CTFTimeStep);
                    }
                }
            }
        }
    }
}

void ConstructionProps::calculateFinalCoefficients()
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   June 1990
    //       MODIFIED       Apr 96, RKS, cosmetic, algorithm neutral changes
    //       RE-ENGINEERED  July 1996, RKS; Nov 1999, LKL (allocatable arrays)

    // PURPOSE OF THIS SUBROUTINE:
    // Subprogram to calculate the Sj and ej coefficients of Seem's
    // dissertation.  Follows Seem's technique to compute the coefficients
    // in order with minimum storage requirements.

    // METHODOLOGY EMPLOYED:
    // Combine the results of the ExponMatrix, InvertMatrix, and
    // CalculateGammas routines together to arrive at the temperature
    // coefficients (s, s0) and the heat flux history coefficients (e) of
    // the CTFs.  The outline of this subroutine is based on step 5 of
    // Seem's suggested implementation of the state space method found on
    // pages 26+27 of his dissertation.

    // REFERENCES:
    // The state space method of calculating CTFs is outlined in the
    // doctoral dissertation of John Seem, "Modeling of Heat Transfer in
    // Buildings", Department of Mechanical Engineering, University of
    // Wisconsin-Madison, 1987.  In particular, the equations used for
    // these calculations are equations (2.1.24) through (2.1.26) in Seem's
    // dissertation.

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr Real64 ConvrgLim(1.0e-13); // Convergence limit (ratio) for cutting off the calculation of further
    // CTFs.  This value was found to give suitable accuracy in IBLAST.

    Real64 avg;            // Intermediate calculation variable (average)
    bool CTFConvrg;        // Set after CTFs are calculated, based on whether there are too many CTFs terms
    Array2D<Real64> PhiR0; // Product of Phi( = AExp) and R0 matrices from the state space method
    Real64 rat;            // Intermediate calculation variable (ratio of flux history terms)
    Array2D<Real64> Rnew;  // Current R matrix
    Array2D<Real64> Rold;  // R matrix from the last iteration
    Real64 SurfNodeFac;    // Multiplying factor applied to various surface nodes
    Real64 trace;          // Trace of the product of Phi( = AExp) and R0

    // Subroutine initializations
    PhiR0.allocate(this->rcmax, this->rcmax);
    Rnew.allocate(this->rcmax, this->rcmax);
    Rold.allocate(this->rcmax, this->rcmax);
    PhiR0 = 0.0;
    Rold = 0.0;

    this->s0 = 0.0;
    this->s = 0.0;
    this->e = 0.0;
    Rnew = this->IdenMatrix; // Rnew initialized to the identity matrix

    // Calculate Gamma1-Gamma2.  Gamma1 is not used by itself in the
    // equations, only Gamma1-Gamma2.  Thus, reset Gamma1 to:
    // Gamma1-Gamma2
    for (int i = 1; i <= this->rcmax; ++i) {
        for (int j = 1; j <= 3; ++j) {
            this->Gamma1(j, i) -= this->Gamma2(j, i);
        }
    }

    // Compute s0.  See Seem's thesis equation (2.1.24) which states that:
    // s0  =  (CMat*R0*Gamma2) + (DMat)
    // Note that for a two-dimensional solution, there is more than one
    // node at the surface and the effect of each of these must be added
    // together.
    if (this->SolutionDimensions == 1) {
        this->s0(1, 1) = this->CMat(1) * this->Gamma2(1, 1) + this->DMat(1);
        this->s0(2, 1) = this->CMat(1) * this->Gamma2(2, 1);
        this->s0(3, 1) = this->CMat(1) * this->Gamma2(3, 1);
        this->s0(1, 2) = this->CMat(2) * this->Gamma2(1, this->rcmax);
        this->s0(2, 2) = this->CMat(2) * this->Gamma2(2, this->rcmax) + this->DMat(2);
        this->s0(3, 2) = this->CMat(2) * this->Gamma2(3, this->rcmax);
    } else { // SolutionDimensions = 2
        for (int SurfNode = 1; SurfNode <= this->NumOfPerpendNodes; ++SurfNode) {
            if ((SurfNode == 1) || (SurfNode == this->NumOfPerpendNodes)) {
                SurfNodeFac = 0.5;
            } else {
                SurfNodeFac = 1.0;
            }
            this->s0(1, 1) += SurfNodeFac * this->CMat(1) * this->Gamma2(1, SurfNode);
            this->s0(2, 1) += SurfNodeFac * this->CMat(1) * this->Gamma2(2, SurfNode);
            this->s0(3, 1) += SurfNodeFac * this->CMat(1) * this->Gamma2(3, SurfNode);
            this->s0(1, 2) += SurfNodeFac * this->CMat(2) * this->Gamma2(1, this->rcmax + 1 - SurfNode);
            this->s0(2, 2) += SurfNodeFac * this->CMat(2) * this->Gamma2(2, this->rcmax + 1 - SurfNode);
            this->s0(3, 2) += SurfNodeFac * this->CMat(2) * this->Gamma2(3, this->rcmax + 1 - SurfNode);
        }
        this->s0(1, 1) += double(this->NumOfPerpendNodes - 1) * this->DMat(1);
        this->s0(2, 2) += double(this->NumOfPerpendNodes - 1) * this->DMat(2);
    }

    if (this->NodeSource > 0) {
        this->s0(1, 3) = this->Gamma2(1, this->NodeSource);
        this->s0(2, 3) = this->Gamma2(2, this->NodeSource);
        this->s0(3, 3) = this->Gamma2(3, this->NodeSource);
    }
    if (this->NodeUserTemp > 0) {
        this->s0(1, 4) = this->Gamma2(1, this->NodeUserTemp);
        this->s0(2, 4) = this->Gamma2(2, this->NodeUserTemp);
        this->s0(3, 4) = this->Gamma2(3, this->NodeUserTemp);
    }

    // Check for and enforce symmetry in the cross term (Y)
    if (std::abs(this->s0(2, 1)) != std::abs(this->s0(1, 2))) {
        avg = (std::abs(this->s0(2, 1)) + std::abs(this->s0(1, 2))) * 0.5;
        this->s0(2, 1) *= avg / std::abs(this->s0(2, 1));
        this->s0(1, 2) *= avg / std::abs(this->s0(1, 2));
    }

    // Compute S's and e's from 1 to n-1.  See equations (2.1.25) and
    // (2.1.26) and Appendix C.
    int inum = 1;      // Set history term counter
    CTFConvrg = false; // Set the convergence logical to false

    // The following DO WHILE loop calculates each successive set of time
    // history terms until there are rcmax number of history terms or the
    // latest flux history term is negligibly small compared to the first
    // flux history term.
    while ((!CTFConvrg) && (inum < this->rcmax)) { // Begin CTF calculation loop ...

        // Compute e(inum) based on Appendix C (Seem's dissertation). First,
        // compute the new PhiR0 and its trace.

        trace = 0.0;

        for (int ir = 1; ir <= this->rcmax; ++ir) {

            for (int ic = 1; ic <= this->rcmax; ++ic) {
                PhiR0(ic, ir) = 0.0;
                for (int is = 1; is <= this->rcmax; ++is) {
                    // Make sure the next term won't cause an underflow.  If it will end up being so small
                    // as to go below TinyLimit, then ignore it since it won't add anything to PhiR0 anyway.
                    if (std::abs(Rnew(ic, is)) > Constant::rTinyValue) {
                        if (std::abs(this->AExp(is, ir)) > std::abs(Constant::rTinyValue / Rnew(ic, is))) {
                            PhiR0(ic, ir) += this->AExp(is, ir) * Rnew(ic, is);
                        }
                    }
                }
            }

            trace += PhiR0(ir, ir);
        }

        // Now calculate ej from the trace.  According to Appendix C:
        // e(j) = -Trace[AExp*R(j-1)]/j

        this->e(inum) = -trace / double(inum);

        // Update Rold and compute Rnew.  Note:  PhiR0 = AExp*R(j-1) here.
        // According to Appendix C:  R(j) = AExp*R(j-1) + e(j-1)

        for (int ir = 1; ir <= this->rcmax; ++ir) {
            for (int ic = 1; ic <= this->rcmax; ++ic) {
                Rold(ic, ir) = Rnew(ic, ir);
                Rnew(ic, ir) = PhiR0(ic, ir);
            }
            Rnew(ir, ir) += this->e(inum);
        }

        // Compute S(inum) based on eq.(2.1.25) which states:
        // S(j)  =  CMat*[R(j-1)*(Gamma1-Gamma2)+R(j)*Gamma2]
        //          + e(j)*DMat
        if (this->SolutionDimensions == 1) {
            for (int j = 1; j <= 3; ++j) {
                for (int is2 = 1; is2 <= this->rcmax; ++is2) {
                    this->s(j, 1, inum) += this->CMat(1) * (Rold(is2, 1) * this->Gamma1(j, is2) + Rnew(is2, 1) * this->Gamma2(j, is2));
                    this->s(j, 2, inum) +=
                        this->CMat(2) * (Rold(is2, this->rcmax) * this->Gamma1(j, is2) + Rnew(is2, this->rcmax) * this->Gamma2(j, is2));
                    if (this->NodeSource > 0) {
                        this->s(j, 3, inum) +=
                            (Rold(is2, this->NodeSource) * this->Gamma1(j, is2) + Rnew(is2, this->NodeSource) * this->Gamma2(j, is2));
                    }
                    if (this->NodeUserTemp > 0) {
                        this->s(j, 4, inum) +=
                            (Rold(is2, this->NodeUserTemp) * this->Gamma1(j, is2) + Rnew(is2, this->NodeUserTemp) * this->Gamma2(j, is2));
                    }
                }
                if (j != 3) {
                    this->s(j, j, inum) += this->e(inum) * this->DMat(j);
                }
            }
        } else { // SolutionDimensions = 2
            for (int j = 1; j <= 3; ++j) {
                for (int is2 = 1; is2 <= this->rcmax; ++is2) {
                    for (int SurfNode = 1; SurfNode <= this->NumOfPerpendNodes; ++SurfNode) {
                        if ((SurfNode == 1) || (SurfNode == this->NumOfPerpendNodes)) {
                            SurfNodeFac = 0.5;
                        } else {
                            SurfNodeFac = 1.0;
                        }
                        this->s(j, 1, inum) +=
                            SurfNodeFac * this->CMat(1) * (Rold(is2, SurfNode) * this->Gamma1(j, is2) + Rnew(is2, SurfNode) * this->Gamma2(j, is2));
                        this->s(j, 2, inum) += SurfNodeFac * this->CMat(2) *
                                               (Rold(is2, this->rcmax + 1 - SurfNode) * this->Gamma1(j, is2) +
                                                Rnew(is2, this->rcmax + 1 - SurfNode) * this->Gamma2(j, is2));
                    }
                    if (this->NodeSource > 0) {
                        this->s(j, 3, inum) +=
                            (Rold(is2, this->NodeSource) * this->Gamma1(j, is2) + Rnew(is2, this->NodeSource) * this->Gamma2(j, is2));
                    }
                    if (this->NodeUserTemp > 0) {
                        this->s(j, 4, inum) +=
                            (Rold(is2, this->NodeUserTemp) * this->Gamma1(j, is2) + Rnew(is2, this->NodeUserTemp) * this->Gamma2(j, is2));
                    }
                }
            }
            this->s(1, 1, inum) += this->e(inum) * this->DMat(1) * double(this->NumOfPerpendNodes - 1);
            this->s(2, 2, inum) += this->e(inum) * this->DMat(2) * double(this->NumOfPerpendNodes - 1);
        }

        // Check for and enforce symmetry in the cross term (Y)
        if (std::abs(s(2, 1, inum)) != std::abs(s(1, 2, inum))) {
            avg = (std::abs(s(2, 1, inum)) + std::abs(s(1, 2, inum))) * 0.5;
            this->s(2, 1, inum) *= avg / std::abs(s(2, 1, inum));
            this->s(1, 2, inum) *= avg / std::abs(s(1, 2, inum));
        }

        // Check for convergence of the CTFs.
        if (e(1) == 0.0) {

            this->NumCTFTerms = 1; // e(1) is zero, so there are no history terms.
            CTFConvrg = true;      // CTF calculations have converged--set logical.

        } else {
            // e(1) is non-zero -- Calculate and compare the ratio of the flux
            // terms to the convergence limit.
            rat = std::abs(e(inum) / this->e(1));

            if (rat < ConvrgLim) {

                // If the ratio is less than the convergence limit, then any other
                // terms would have a neglible impact on the CTF-based energy balances.
                this->NumCTFTerms = inum;
                CTFConvrg = true; // CTF calculations have converged--set logical.
            }
        } // ... end of convergence check block.

        ++inum;

    } // ... end of CTF calculation loop.
    // Continue to the next coefficient if the solution has not converged
    if (!CTFConvrg) { // Compute last e and S, if still unconverged.

        // Compute e(inum) based on Appendix C (Seem's dissertation) or see
        // equation above.  First compute the new PhiR0 and its trace.

        trace = 0.0;

        for (int ir = 1; ir <= this->rcmax; ++ir) {
            for (int is = 1; is <= this->rcmax; ++is) {
                trace += this->AExp(is, ir) * Rnew(ir, is);
            }
        }

        this->e(this->rcmax) = -trace / double(this->rcmax); // Now calculate ej from the trace.

        // Compute S(inum) based on eq.(2.1.25) which states:
        //   S(last) = CMat*R(last-1)*(Gamma1-Gamma2)+e(last)*DMat

        if (this->SolutionDimensions == 1) {
            for (int j = 1; j <= 3; ++j) {
                for (int is2 = 1; is2 <= this->rcmax; ++is2) {
                    this->s(j, 1, this->rcmax) += this->CMat(1) * Rnew(is2, 1) * this->Gamma1(j, is2);
                    this->s(j, 2, this->rcmax) += this->CMat(2) * Rnew(is2, this->rcmax) * this->Gamma1(j, is2);
                    if (this->NodeSource > 0) {
                        this->s(j, 3, this->rcmax) += Rnew(is2, this->NodeSource) * this->Gamma1(j, is2);
                    }
                    if (this->NodeUserTemp > 0) {
                        this->s(j, 4, this->rcmax) += Rnew(is2, this->NodeUserTemp) * this->Gamma1(j, is2);
                    }
                }
            }
            this->s(1, 1, this->rcmax) += this->e(this->rcmax) * this->DMat(1);
            this->s(2, 2, this->rcmax) += this->e(this->rcmax) * this->DMat(2);
            this->NumCTFTerms = this->rcmax;
        } else { // SolutionDimensions = 2
            for (int j = 1; j <= 3; ++j) {
                for (int is2 = 1; is2 <= this->rcmax; ++is2) {
                    for (int SurfNode = 1; SurfNode <= this->NumOfPerpendNodes; ++SurfNode) {
                        if ((SurfNode == 1) || (SurfNode == this->NumOfPerpendNodes)) {
                            SurfNodeFac = 0.5;
                        } else {
                            SurfNodeFac = 1.0;
                        }
                        this->s(j, 1, this->rcmax) += SurfNodeFac * this->CMat(1) * Rnew(is2, SurfNode) * this->Gamma1(j, is2);
                        this->s(j, 2, this->rcmax) += SurfNodeFac * this->CMat(2) * Rnew(is2, this->rcmax + 1 - SurfNode) * this->Gamma1(j, is2);
                    }
                    if (this->NodeSource > 0) {
                        this->s(j, 3, this->rcmax) += Rnew(is2, this->NodeSource) * this->Gamma1(j, is2);
                    }
                    if (this->NodeUserTemp > 0) {
                        this->s(j, 4, this->rcmax) += Rnew(is2, this->NodeUserTemp) * this->Gamma1(j, is2);
                    }
                }
            }
            this->s(1, 1, this->rcmax) += this->e(this->rcmax) * this->DMat(1) * double(this->NumOfPerpendNodes - 1);
            this->s(2, 2, this->rcmax) += this->e(this->rcmax) * this->DMat(2) * double(this->NumOfPerpendNodes - 1);
        }

        // Check for and enforce symmetry in the cross term (Y)

        if (std::abs(s(2, 1, this->rcmax)) != std::abs(s(1, 2, this->rcmax))) {
            avg = (std::abs(s(2, 1, this->rcmax)) + std::abs(s(1, 2, this->rcmax))) * 0.5;
            this->s(2, 1, this->rcmax) *= avg / std::abs(s(2, 1, this->rcmax));
            this->s(1, 2, this->rcmax) *= avg / std::abs(s(1, 2, this->rcmax));
        }

    } // ... end of IF block for calculation of last e and S.

    PhiR0.deallocate();
    Rnew.deallocate();
    Rold.deallocate();
}

void ConstructionProps::reportTransferFunction(EnergyPlusData &state, int const cCounter)
{

    static constexpr std::string_view Format_700{" Construction CTF,{},{:4},{:4},{:4},{:8.3F},{:15.4N},{:8.3F},{:8.3F},{:8.3F},{:8.3F},{}\n"};
    print(state.files.eio,
          Format_700,
          this->Name,
          cCounter,
          this->TotLayers,
          this->NumCTFTerms,
          this->CTFTimeStep,
          this->UValue,
          this->OutsideAbsorpThermal,
          this->InsideAbsorpThermal,
          this->OutsideAbsorpSolar,
          this->InsideAbsorpSolar,
          Material::surfaceRoughnessNames[(int)this->OutsideRoughness]);

    for (int I = 1; I <= this->TotLayers; ++I) {
        int Layer = this->LayerPoint(I);
        auto const *thisMaterial = state.dataMaterial->materials(Layer);
        switch (thisMaterial->group) {
        case Material::Group::AirGap: {
            static constexpr std::string_view Format_702(" Material:Air CTF Summary,{},{:12.4N}\n");
            print(state.files.eio, Format_702, thisMaterial->Name, thisMaterial->Resistance);
        } break;
        default: {
            static constexpr std::string_view Format_701(" Material CTF Summary,{},{:8.4F},{:14.3F},{:11.3F},{:13.3F},{:12.4N}\n");
            Material::MaterialBase const *mp = thisMaterial;
            print(state.files.eio, Format_701, mp->Name, mp->Thickness, mp->Conductivity, mp->Density, mp->SpecHeat, mp->Resistance);
        } break;
        }
    }

    for (int I = this->NumCTFTerms; I >= 0; --I) {
        if (I != 0) {
            static constexpr std::string_view Format_703(" CTF,{:4},{:20.8N},{:20.8N},{:20.8N},{:20.8N}\n");
            print(state.files.eio, Format_703, I, this->CTFOutside[I], this->CTFCross[I], this->CTFInside[I], this->CTFFlux[I]);
        } else {
            static constexpr std::string_view Format_704(" CTF,{:4},{:20.8N},{:20.8N},{:20.8N}\n");
            print(state.files.eio, Format_704, I, this->CTFOutside[I], this->CTFCross[I], this->CTFInside[I]);
        }
    }

    if (this->SourceSinkPresent) {
        // QTFs...
        for (int I = this->NumCTFTerms; I >= 0; --I) {
            static constexpr std::string_view Format_705(" QTF,{:4},{:20.8N},{:20.8N}\n");
            print(state.files.eio, Format_705, I, this->CTFSourceOut[I], this->CTFSourceIn[I]);
        }
        // QTFs for source/sink location temperature calculation...
        for (int I = this->NumCTFTerms; I >= 0; --I) {
            static constexpr std::string_view Format_706(" Source/Sink Loc Internal Temp QTF,{:4},{:20.8N},{:20.8N},{:20.8N}\n");
            print(state.files.eio, Format_706, I, this->CTFTSourceOut[I], this->CTFTSourceIn[I], this->CTFTSourceQ[I]);
        }
        if (this->TempAfterLayer != 0) {
            // QTFs for user specified interior temperature calculation...
            for (int I = this->NumCTFTerms; I >= 0; --I) {
                static constexpr std::string_view Format_707(" User Loc Internal Temp QTF,{:4},{:20.8N},{:20.8N},{:20.8N}\n");
                print(state.files.eio, Format_707, I, this->CTFTUserOut[I], this->CTFTUserIn[I], this->CTFTUserSource[I]);
            }
        }
    }
}

void ConstructionProps::reportLayers(EnergyPlusData &state)
{
    // Report the layers for each opaque construction in predefined tabular report
    // J. Glazer March 2024
    if (state.dataOutRptPredefined->pdchOpqConsLayCol.size() > 0) {
        for (int i = 1; i <= this->TotLayers; ++i) {
            int layerIndex = this->LayerPoint(i);
            auto const *mat = state.dataMaterial->materials(layerIndex);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpqConsLayCol[i - 1], this->Name, mat->Name);
        }
    }
}

bool ConstructionProps::isGlazingConstruction(EnergyPlusData &state) const
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   September 2016

    // PURPOSE OF THIS SUBROUTINE:
    // Commonly used routine in several places in EnergyPlus which examines if current
    // construction is glazing construction
    const Material::Group group = state.dataMaterial->materials(LayerPoint(1))->group;
    return (group == Material::Group::Glass) || (group == Material::Group::Shade) || (group == Material::Group::Screen) ||
           (group == Material::Group::Blind) || (group == Material::Group::GlassSimple);
}

Real64 ConstructionProps::setThicknessPerpendicular(EnergyPlusData &state, Real64 userValue)
{
    // Limits set here are similar to limits used in HydronicSystemBaseData::sizeRadiantSystemTubeLength
    // which is for autosizing tube length.  The limits are not as strictly enforced here because they are
    // not being used for autosizing so more latitude with the values is desired.  Warnings will still alert
    // the user if values seem outside of the ordinary range anticipated.
    Real64 returnValue = userValue / 2.0; // Divide by two because internally the half distance will be used to calculate CTFs
    if (returnValue <= 0.001) {           // lowest reasonable value for "half" the tube spacing in meters
        ShowWarningError(state, "ConstructionProperty:InternalHeatSource has a tube spacing that is less than 2 mm.  This is not allowed.");
        ShowContinueError(
            state,
            format("Construction={} has this problem.  The tube spacing has been reset to 0.15m (~6 inches) for this construction.", this->Name));
        ShowContinueError(state, "As per the Input Output Reference, tube spacing is only used for 2-D solutions and autosizing.");
        returnValue = 0.075;          // default "half" tube spacing in meters (roughly equivalent to 15cm or 6 inches of tube spacing)
    } else if (returnValue < 0.005) { // below this value for "half" the tube spacing in meters throw a warning
        ShowWarningError(state, "ConstructionProperty:InternalHeatSource has a tube spacing that is less than 1 cm (0.4 inch).");
        ShowContinueError(state, format("Construction={} has this concern.  Please check this construction to make sure it is correct.", this->Name));
        ShowContinueError(state, "As per the Input Output Reference, tube spacing is only used for 2-D solutions and autosizing.");
    } else if (returnValue > 0.5) { // above this value for "half" the tube spacing in meters throw a warning
        ShowWarningError(state, "ConstructionProperty:InternalHeatSource has a tube spacing that is greater than 1 meter (39.4 inches).");
        ShowContinueError(state, format("Construction={} has this concern.  Please check this construction to make sure it is correct.", this->Name));
        ShowContinueError(state, "As per the Input Output Reference, tube spacing is only used for 2-D solutions and autosizing.");
    }
    return returnValue;
}

Real64 ConstructionProps::setUserTemperatureLocationPerpendicular(EnergyPlusData &state, Real64 userValue)
{
    if (userValue < 0.0) {
        ShowWarningError(state, "ConstructionProperty:InternalHeatSource has a perpendicular temperature location parameter that is less than zero.");
        ShowContinueError(state, format("Construction={} has this error.  The parameter has been reset to 0.", this->Name));
        return 0.0;
    } else if (userValue > 1.0) {
        ShowWarningError(state,
                         "ConstructionProperty:InternalHeatSource has a perpendicular temperature location parameter that is greater than one.");
        ShowContinueError(state, format("Construction={} has this error.  The parameter has been reset to 1.", this->Name));
        return 1.0;
    } else { // Valid value between 0 and 1
        return userValue;
    }
}

void ConstructionProps::setNodeSourceAndUserTemp(Array1D_int &Nodes)
{
    this->NodeSource = 0;
    this->NodeUserTemp = 0;
    if (!this->SourceSinkPresent) {
        return;
    }

    for (int Layer = 1; Layer <= this->SourceAfterLayer; ++Layer) {
        this->NodeSource += Nodes(Layer);
    }

    if ((this->NodeSource > 0) && (this->SolutionDimensions > 1)) {
        this->NodeSource = (this->NodeSource - 1) * this->NumOfPerpendNodes + 1;
    }

    for (int Layer = 1; Layer <= this->TempAfterLayer; ++Layer) {
        this->NodeUserTemp += Nodes(Layer);
    }

    if ((this->NodeUserTemp > 0) && (this->SolutionDimensions > 1)) {
        this->NodeUserTemp = (this->NodeUserTemp - 1) * this->NumOfPerpendNodes +
                             round(this->userTemperatureLocationPerpendicular * (this->NumOfPerpendNodes - 1)) + 1;
    }
}

void ConstructionProps::setArraysBasedOnMaxSolidWinLayers(EnergyPlusData &state)
{
    this->AbsDiff.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->AbsDiffBack.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->layerSlatBlindDfAbs.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->AbsBeamCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->AbsBeamBackCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->tBareSolCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->tBareVisCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->rfBareSolCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->rfBareVisCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->rbBareSolCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->rbBareVisCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->afBareSolCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);
    this->abBareSolCoef.allocate(state.dataHeatBal->MaxSolidWinLayers);

    for (int Layer = 1; Layer <= state.dataHeatBal->MaxSolidWinLayers; ++Layer) {
        this->AbsDiff(Layer) = 0.0;
        this->AbsDiffBack(Layer) = 0.0;
    }
    for (int Layer = 1; Layer <= state.dataHeatBal->MaxSolidWinLayers; ++Layer) {
        auto &slatBlindDfAbs = this->layerSlatBlindDfAbs(Layer);
        for (int iSlatAng = 0; iSlatAng < Material::MaxSlatAngs; ++iSlatAng) {
            auto &blindDfAbs = slatBlindDfAbs[iSlatAng];
            blindDfAbs.Sol.Ft.Df.Abs = 0.0;
            blindDfAbs.Sol.Ft.Df.AbsGnd = 0.0;
            blindDfAbs.Sol.Ft.Df.AbsGnd = 0.0;
            blindDfAbs.Sol.Bk.Df.Abs = 0.0;
        }
    }
    for (int Layer = 1; Layer <= state.dataHeatBal->MaxSolidWinLayers; ++Layer) {
        std::fill(this->AbsBeamCoef(Layer).begin(), this->AbsBeamCoef(Layer).end(), 0.0);
        std::fill(this->AbsBeamBackCoef(Layer).begin(), this->AbsBeamBackCoef(Layer).end(), 0.0);
        std::fill(this->tBareSolCoef(Layer).begin(), this->tBareSolCoef(Layer).end(), 0.0);
        std::fill(this->tBareVisCoef(Layer).begin(), this->tBareVisCoef(Layer).end(), 0.0);
        std::fill(this->rfBareSolCoef(Layer).begin(), this->rfBareSolCoef(Layer).end(), 0.0);
        std::fill(this->rfBareVisCoef(Layer).begin(), this->rfBareVisCoef(Layer).end(), 0.0);
        std::fill(this->rbBareSolCoef(Layer).begin(), this->rbBareSolCoef(Layer).end(), 0.0);
        std::fill(this->rbBareVisCoef(Layer).begin(), this->rbBareVisCoef(Layer).end(), 0.0);
        std::fill(this->afBareSolCoef(Layer).begin(), this->afBareSolCoef(Layer).end(), 0.0);
        std::fill(this->abBareSolCoef(Layer).begin(), this->abBareSolCoef(Layer).end(), 0.0);
    }
}

} // namespace EnergyPlus::Construction
