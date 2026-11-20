#include <algorithm>
#include <functional>

#include "6par_newton.h"
#include "lib_shared_inverter.h"
#include "lib_util.h"

// NOTE: EnergyPlus: OND_INVERTER branches removed (lib_ondinv not built), see lib_shared_inverter.h

SharedInverter::SharedInverter(int inverterType, size_t numberOfInverters,
    sandia_inverter_t* sandiaInverter, partload_inverter_t* partloadInverter, ond_inverter* ondInverter)
{
    m_inverterType = inverterType;
    m_numInverters = numberOfInverters;
    m_sandiaInverter = sandiaInverter;
    m_partloadInverter = partloadInverter;
    m_ondInverter = ondInverter;
    m_tempEnabled = false;

    if (m_inverterType == SANDIA_INVERTER || m_inverterType == DATASHEET_INVERTER || m_inverterType == COEFFICIENT_GENERATOR)
        m_nameplateAC_kW = m_numInverters * m_sandiaInverter->Paco * util::watt_to_kilowatt;
    else if (m_inverterType == PARTLOAD_INVERTER)
        m_nameplateAC_kW = m_numInverters * m_partloadInverter->Paco * util::watt_to_kilowatt;

    powerDC_kW = 0.;
    powerAC_kW = 0.;
    efficiencyAC = 96.;
    powerClipLoss_kW = 0.;
    powerConsumptionLoss_kW = 0.;
    powerNightLoss_kW = 0.;
    powerTempLoss_kW = 0.;
    powerLossTotal_kW = 0.;
    dcWiringLoss_ond_kW = 0.;
    acWiringLoss_ond_kW = 0.;
}

SharedInverter::SharedInverter(const SharedInverter& orig) {
    m_inverterType = orig.m_inverterType;
    m_numInverters = orig.m_numInverters;
    m_nameplateAC_kW = orig.m_nameplateAC_kW;
    m_tempEnabled = orig.m_tempEnabled;
    m_thermalDerateCurves = orig.m_thermalDerateCurves;
    m_sandiaInverter = orig.m_sandiaInverter;
    m_partloadInverter = orig.m_partloadInverter;
    m_ondInverter = orig.m_ondInverter;
    efficiencyAC = orig.efficiencyAC;

    powerDC_kW = orig.powerDC_kW;
    powerAC_kW = orig.powerAC_kW;
    powerClipLoss_kW = orig.powerClipLoss_kW;
    powerConsumptionLoss_kW = orig.powerConsumptionLoss_kW;
    powerNightLoss_kW = orig.powerNightLoss_kW;
    powerTempLoss_kW = orig.powerTempLoss_kW;
    powerLossTotal_kW = orig.powerLossTotal_kW;
    dcWiringLoss_ond_kW = orig.dcWiringLoss_ond_kW;
    acWiringLoss_ond_kW = orig.acWiringLoss_ond_kW;
}

bool sortByVoltage(std::vector<double> i, std::vector<double> j)
{
    return (i[0] < j[0]);
}

int SharedInverter::setTempDerateCurves(std::vector<std::vector<double>> derateCurves)
{
    m_thermalDerateCurves.clear();

    // Check derate curves have V > 0, and that for each pair T > -273, slope < 0
    for (size_t r = 0; r < derateCurves.size(); r++) {
        if (derateCurves[r][0] <= 0.) return (int)r + 1;
        size_t tempSlopeEntries = derateCurves[r].size() - 1;
        if ((tempSlopeEntries % 2) != 0) return (int)r + 1;
        for (size_t p = 0; p < tempSlopeEntries / 2; p++) {
            if (derateCurves[r][2 * p + 1] <= -273. || derateCurves[r][2 * p + 2] > 0.) return (int)r + 1;
        }
        m_thermalDerateCurves.push_back(derateCurves[r]);
    }

    // Sort by DC voltage
    std::sort(m_thermalDerateCurves.begin(), m_thermalDerateCurves.end(), sortByVoltage);

    if (!m_thermalDerateCurves.empty())
        m_tempEnabled = true;
    return 0;
}

std::vector<std::vector<double>> SharedInverter::getTempDerateCurves() {
    return m_thermalDerateCurves;
}

void SharedInverter::findPointOnCurve(size_t idx, double T, double& startT, double& slope) {
    size_t p = 0;
    while (2 * p + 2 < m_thermalDerateCurves[idx].size() && T >= m_thermalDerateCurves[idx][2 * p + 1]) {
        p++;
    }
    if (2 * p + 2 >= m_thermalDerateCurves[idx].size()) {
        p--;
    }
    startT = m_thermalDerateCurves[idx][2 * p + 1];
    slope = m_thermalDerateCurves[idx][2 * p + 2];
}

void SharedInverter::calculateTempDerate(double V, double tempC, double& p_dc_rated, double& ratio, double& loss)
{
    if (ratio == 0. || p_dc_rated == 0.) return;

    double slope = 0.0;
    double startT = 0.0;
    double Vdc = 0.0;
    double slope2 = 0.0;
    double startT2 = 0.0;
    double Vdc2 = 0.0;

    double p_dc_max = getInverterDCMaxPower(p_dc_rated);

    // Find the appropriate derate curve depending on DC voltage
    size_t idx = 0;
    double deltaT = 0.0;
    double slopeInterpolated = 0.0;
    double startTInterpolated = 0.0;

    while (idx < m_thermalDerateCurves.size() && V > m_thermalDerateCurves[idx][0]) {
        idx++;
    }
    if (m_thermalDerateCurves.size() == 1) {
        Vdc2 = m_thermalDerateCurves[0][0];
        startTInterpolated = m_thermalDerateCurves[0][1];
        slopeInterpolated = m_thermalDerateCurves[0][2];
    }
    // Use temp and slope of lower and upper curves for interpolation if they both exist
    else if (idx > 0 && idx < m_thermalDerateCurves.size()) {
        Vdc2 = m_thermalDerateCurves[idx][0];
        Vdc = m_thermalDerateCurves[idx - 1][0];
        double startTGuess = 0.0;
        double slopeGuess = 0.0;
        size_t n = std::max(m_thermalDerateCurves[idx].size() / 2, m_thermalDerateCurves[idx - 1].size() / 2);
        size_t count = 0;
        while (tempC > startTGuess && count < n) {
            findPointOnCurve(idx, startT2, startT2, slope2);
            findPointOnCurve(idx - 1, startT, startT, slope);
            startTGuess = (startT2 - startT) / (Vdc2 - Vdc) * (V - Vdc2) + startT2;
            slopeGuess = (slope2 - slope) / (Vdc2 - Vdc) * (V - Vdc2) + slope2;
            if (tempC > startTGuess) {
                startTInterpolated = startTGuess;
                slopeInterpolated = slopeGuess;
                count++;
            }
        }
    }
    // otherwise extrapolate using first start temps of each curve in order to avoid inconsistent start temps
    else {
        if (idx == 0) {
            Vdc2 = m_thermalDerateCurves[idx][0];
            findPointOnCurve(idx, -273, startT2, slope2);
            Vdc = m_thermalDerateCurves[idx + 1][0];
            findPointOnCurve(idx + 1, -273, startT, slope);
            startTInterpolated = (startT2 - startT) / (Vdc2 - Vdc) * (V - Vdc2) + startT2;
            slopeInterpolated = (slope2 - slope) / (Vdc2 - Vdc) * (V - Vdc2) + slope2;
        }
        else {
            Vdc2 = m_thermalDerateCurves[idx - 1][0];
            findPointOnCurve(idx - 1, -273, startT2, slope2);
            Vdc = m_thermalDerateCurves[idx - 2][0];
            findPointOnCurve(idx - 2, -273, startT, slope);
            startTInterpolated = (startT2 - startT) / (Vdc2 - Vdc) * (V - Vdc2) + startT2;
            slopeInterpolated = (slope2 - slope) / (Vdc2 - Vdc) * (V - Vdc2) + slope2;
        }
    }
    deltaT = tempC - startTInterpolated;

    // If less than start temp, no derating
    if (deltaT <= 0) return;

    // If slope is positive, set to zero with no derating
    if (slopeInterpolated >= 0) return;
    if (slopeInterpolated < -1) slopeInterpolated = -1;

    // Power in units of W, ratio = max output / rated output
    ratio += deltaT * slopeInterpolated;
    if (ratio < 0) ratio = 0.;
    double p_dc_limit = p_dc_max * ratio;
    if (p_dc_rated > p_dc_limit) {
        loss = p_dc_rated - (p_dc_limit);
        p_dc_rated = p_dc_limit;
    }
    else {
        loss = 0;
    }
}

double SharedInverter::getInverterDCMaxPower(double p_dc_rated)
{
    double inv_dc_max_power;
    if (m_inverterType == SANDIA_INVERTER || m_inverterType == DATASHEET_INVERTER || m_inverterType == COEFFICIENT_GENERATOR)
        //m_sandiaInverter->acpower(std::fabs(powerDC_Watts) / m_numInverters, DCStringVoltage, &powerAC_Watts, &P_par, &P_lr, &efficiencyAC, &powerClipLoss_kW, &powerConsumptionLoss_kW, &powerNightLoss_kW);
        inv_dc_max_power = m_sandiaInverter->Pdco;
    else if (m_inverterType == PARTLOAD_INVERTER)
        //m_partloadInverter->acpower(std::fabs(powerDC_Watts) / m_numInverters, &powerAC_Watts, &P_lr, &P_par, &efficiencyAC, &powerClipLoss_kW, &powerNightLoss_kW);
        inv_dc_max_power = m_partloadInverter->Pdco;
    else if (m_inverterType == NONE) {
        inv_dc_max_power = p_dc_rated * util::kilowatt_to_watt;
    }

    return inv_dc_max_power;
}

//function that calculates AC power and inverter losses for a single inverter with one MPPT input
void SharedInverter::calculateACPower(const double powerDC_kW_in, const double DCStringVoltage, double tempC)
{
    double P_par, P_lr;
    bool negativePower = powerDC_kW_in < 0 ? true : false;


    dcWiringLoss_ond_kW = 0.0;
    acWiringLoss_ond_kW = 0.0;

    // Power quantities go in and come out in units of W
    double powerDC_Watts = powerDC_kW_in * util::kilowatt_to_watt;
    double powerAC_Watts = 0.0;
    Tdry_C = tempC;
    StringV = DCStringVoltage;
    double tempLoss = 0.0;
    double power_ratio = 1.0;
    if (m_tempEnabled) {
        calculateTempDerate(DCStringVoltage, tempC, powerDC_Watts, power_ratio, tempLoss);
    }


    if (m_inverterType == SANDIA_INVERTER || m_inverterType == DATASHEET_INVERTER || m_inverterType == COEFFICIENT_GENERATOR)
        m_sandiaInverter->acpower(std::fabs(powerDC_Watts) / m_numInverters, DCStringVoltage, &powerAC_Watts, &P_par, &P_lr, &efficiencyAC, &powerClipLoss_kW, &powerConsumptionLoss_kW, &powerNightLoss_kW);
    else if (m_inverterType == PARTLOAD_INVERTER)
        m_partloadInverter->acpower(std::fabs(powerDC_Watts) / m_numInverters, &powerAC_Watts, &P_lr, &P_par, &efficiencyAC, &powerClipLoss_kW, &powerNightLoss_kW);
    else if (m_inverterType == NONE) {
        powerClipLoss_kW = 0.;
        powerConsumptionLoss_kW = 0.;
        powerNightLoss_kW = 0.;
        efficiencyAC = NONE_INVERTER_EFF;
        powerAC_Watts = powerDC_Watts * efficiencyAC;
    }


    // Convert units to kW- no need to scale to system size because passed in as power to total number of inverters
    powerDC_kW = powerDC_Watts * util::watt_to_kilowatt;
    convertOutputsToKWandScale(tempLoss, powerAC_Watts);

    // In event shared inverter is charging a battery only, need to re-convert to negative power
    if (negativePower) {
        powerAC_kW = -1.0 * fabs(powerAC_kW);
    }
}

/* This function takes input inverter DC power (kW) per MPPT input for a SINGLE multi-mppt inverter, DC voltage (V) per input, and ambient temperature (deg C), and calculates output for the total number of inverters in the system */
void SharedInverter::calculateACPower(const std::vector<double> powerDC_kW_in, const std::vector<double> DCStringVoltage, double tempC)
{
    double P_par, P_lr;

    //need to convert to watts and divide power by m_num_inverters
    std::vector<double> powerDC_Watts_one_inv;
    std::vector<double> powerDC_Watts_one_inv_iter;
    for (size_t i = 0; i < powerDC_kW_in.size(); i++) {
        powerDC_Watts_one_inv.push_back(powerDC_kW_in[i] * util::kilowatt_to_watt / m_numInverters);
    }
    Tdry_C = tempC;
    StringV = DCStringVoltage[0];
    int size = DCStringVoltage.size();
    std::vector<double> tempLoss(size, 0);

    double power_ratio = 1.0;
    if (m_tempEnabled) {
        //use average of the DC voltages to pick which temp curve to use- a weighted average might be better but we don't have that information here
        double avgDCVoltage = 0;
        double avgDCPower_Watts = 0;
        for (size_t i = 0; i < powerDC_Watts_one_inv.size(); i++) {
            power_ratio = 1.0;
            calculateTempDerate(DCStringVoltage[i], tempC, powerDC_Watts_one_inv[i], power_ratio, tempLoss[i]);
        }
    }
    // Power quantities go in and come out in units of W
    double powerAC_Watts = 0;
    if (m_inverterType == SANDIA_INVERTER || m_inverterType == DATASHEET_INVERTER || m_inverterType == COEFFICIENT_GENERATOR)
        m_sandiaInverter->acpower(powerDC_Watts_one_inv, DCStringVoltage, &powerAC_Watts, &P_par, &P_lr, &efficiencyAC, &powerClipLoss_kW, &powerConsumptionLoss_kW, &powerNightLoss_kW);
    else if (m_inverterType == PARTLOAD_INVERTER)
        m_partloadInverter->acpower(powerDC_Watts_one_inv, &powerAC_Watts, &P_lr, &P_par, &efficiencyAC, &powerClipLoss_kW, &powerNightLoss_kW);



    // Scale to total system size
    // Do not need to scale back up by m_numInverters because scaling them down was a separate vector, powerDC_Watts_one_inv
    powerDC_kW = 0;
    double tempLoss_avg = 0;
    for (size_t i = 0; i < powerDC_Watts_one_inv.size(); i++) {
        powerDC_kW += powerDC_Watts_one_inv[i] * util::watt_to_kilowatt * m_numInverters;
        tempLoss_avg += tempLoss[i];
    }
    tempLoss_avg /= tempLoss.size();
    //Convert units to kW and scale to total array for all other outputs
    convertOutputsToKWandScale(tempLoss_avg, powerAC_Watts);
}

void SharedInverter::solve_kwdc_for_kwac(const double* x, double* f) {
    calculateACPower(x[0], StringV, Tdry_C);
    f[0] = powerAC_kW - solver_AC;
}

using namespace std::placeholders;
double SharedInverter::calculateRequiredDCPower(const double kwAC, const double DCStringV, double tempC) {

    SharedInverter clone = SharedInverter(*this);

    // set up solver values
    clone.StringV = DCStringV;
    clone.Tdry_C = tempC;
    clone.solver_AC = kwAC;

    std::function<void(const double*, double*)> f;
    f = std::bind(&SharedInverter::solve_kwdc_for_kwac, &clone, _1, _2);

    double x[1], resid[1];
    x[0] = kwAC * 1.04;
    bool check = false;

    newton<double, std::function<void(const double*, double*)>, 1>(x, resid, check, f,
        100, 1e-6, 1e-6, 0.7);

    // if efficiency is too low, the required DC power becomes infinite
    if (!isfinite(x[0]))
        x[0] = kwAC;

    return x[0];
}

double SharedInverter::getInverterDCNominalVoltage()
{
    if (m_inverterType == SANDIA_INVERTER || m_inverterType == DATASHEET_INVERTER || m_inverterType == COEFFICIENT_GENERATOR)
        return m_sandiaInverter->Vdco;
    else if (m_inverterType == PARTLOAD_INVERTER)
        return m_partloadInverter->Vdco;
    else
        return 0.;
}

void SharedInverter::convertOutputsToKWandScale(double tempLoss, double powerAC_watts)
{
    powerAC_kW = powerAC_watts * m_numInverters * util::watt_to_kilowatt;
    powerClipLoss_kW *= m_numInverters * util::watt_to_kilowatt;
    powerConsumptionLoss_kW *= m_numInverters * util::watt_to_kilowatt;
    powerNightLoss_kW *= m_numInverters * util::watt_to_kilowatt;
    powerTempLoss_kW = tempLoss * m_numInverters * util::watt_to_kilowatt;
    powerLossTotal_kW = powerDC_kW - powerAC_kW;
    efficiencyAC *= 100;
    dcWiringLoss_ond_kW *= m_numInverters * util::watt_to_kilowatt;
    acWiringLoss_ond_kW *= m_numInverters * util::watt_to_kilowatt;
}

double SharedInverter::getMaxPowerEfficiency()
{
    if (m_inverterType == SANDIA_INVERTER || m_inverterType == DATASHEET_INVERTER || m_inverterType == COEFFICIENT_GENERATOR)
        calculateACPower(m_sandiaInverter->Paco * util::watt_to_kilowatt, m_sandiaInverter->Vdco, 0.0);
    else if (m_inverterType == PARTLOAD_INVERTER)
        calculateACPower(m_partloadInverter->Paco * util::watt_to_kilowatt, m_partloadInverter->Vdco, 0.0);

    return efficiencyAC;
}

double SharedInverter::getACNameplateCapacitykW()
{
    return m_nameplateAC_kW;
}
