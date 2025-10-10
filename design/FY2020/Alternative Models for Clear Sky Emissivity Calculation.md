Alternative Models for Clear Sky Emissivity Calculation
================

**Xuan Luo, Tianzhen Hong, Yukai Tomsovic**

**Lawrence Berkeley National Laboratory**

 - Original Date: October 15, 2019

## Justification for New Feature ##

Sky emissivity, which represents the complex combination of the spectrally-dependent emissivity of atmospheric constituents, is fundamental to the calculation of downwelling longwave atmospheric radiation [1]. In EnergyPlus, the clear sky emissivity is used to calculated the clear sky temperature. Over the last century, researchers have proposed empirical sky emissivity models as a function of meteorological variables, including ambient temperature, water vapor pressure, or dew point temperature. The differences in these models result in large differences in estimations of thermal energy exchange that can be amplified by climate or application, such as radiative cooling [2].

There are two major algorithms adopted in BPS programs, one based on the work of Clark & Allen (1978) [3], the other Martin & Berdahl (1984) [4]. Currently, EnergyPlus uses the Clark-Allen model based on a logarithmic relationship to dew point temperature, which was formulated using measurements limited to data collected over one year in San Antonio, Texas. While the original authors reported a low root mean square error (RMSE) of 10 W/m<sup>2</sup> while used to caculate long wave radiations, studies by the International Energy Agency and Dai and Fang demonstrated that, out of the empirical models examined, Clark-Allen had among the highest high errors when tested against MODTRAN predictions and observed data, respectively [5-6]. Further illustrating the limitations of Clark-Allen, a more recent assessment by Zhang et al shows that application of this model to calculate downwelling radiation in all-sky conditions tends to result in larger errors for low altitude and humid climates [6]. Literature indicates that this model is of insufficient accuracy. However, because the accuracy of emissivity models is limited by available measured data and strongly correlated to local conditions, it is difficult to establish a clear consensus among literature of the “best” model. For example, the most accurate model presented is often the model that was developed or fitted to the same region or climate as the dataset being studied. Despite this challenge, there is convincing evidence that more appropriate models than Clark-Allen for wide application to the contiguous United States exist.

We propose to add the calibrated version [7] of the Berdahl and Martin [4], Brunt [8] and Idso [9] models, identified as three models with higher accuracy compared to Clark-Allen and other existing popular models. The three models are calibrated using the radiation and meteorological measurements from the SURFRAD (Surface Radiation Budget Network) and ASOS (Automated Surface Observing System) operated by NOAA (National Oceanic and Atmospheric Administration). Currently seven SURFRAD stations are operating
in climatologically diverse regions over the contiguous United States including Bondville (in Illinois), Boulder (in Colorado), Desert Rock (in Nevada), Fort Peck (in Montana), Goodwin Creek (in Mississippi), Penn State University (in Pennsylvania) and Sioux Falls (in South Dakota) represent the climatological diversities.

This feature update would allow users to select from these three models as alternatives to Clark-Allen to calculate the sky emissivity and temperature.

## E-mail and  Conference Call Conclusions ##

N/A

## Overview ##

Berdahl and Martin (1984) proposed the following relationship between dew point temperature (T<sub>d</sub>, C) and clear sky emissivity:

ε = 0.711 + 0.56・(T<sub>d</sub>/100) + 0.73・(T<sub>d</sub>/100)^2  &nbsp;&nbsp;    Eq. (4)    Eq. (1)

Brunt (1932) proposed the following relationship between partial water vapor pressure (P<sub>w</sub>, hPa) and clear sky emissivity:

ε = 0.52 + 0.065・(P<sub>w</sub>)1/2  &nbsp;&nbsp;    Eq. (2)

Idso (1981) uses both partial water vapor pressure (Pw, hPa) and ambient temperature (T<sub>a</sub>, K) in this empirical model:

ε = 0.70 + 5.95・10E-5・(P<sub>w</sub> ∙ exp(1500/T<sub>a</sub>))  &nbsp;&nbsp;    Eq. (3)

These three models are among the most widely accepted models, and all show improved performance over Clark-Allen in comparisons for locations across the United States [5-6]. When analyzed against observed longwave irradiance data, irradiation calculated with these models had relative RMSEs as low as 4% [10-11]. However, in order to address the challenge of location dependence, Li et al performed a grid-search recalibration using data from seven SURFRAD stations across the United States. Their study indicates that, after calibration, the models not only improved significantly in accuracy but also can be grouped into a few families yielding the same longwave irradiance values. This suggests that one of the most important factors in the accuracy of the proposed model is the fitting dataset, for local climate and geography cause large variations in emissivity. Original formulations of these models using small datasets in one location to fit the relationship between the desired meteorological variable and sky emissivity are likely to produce large errors when applied to data outside that region [12].

Newly calibrated forms of the three models proposed by Li et al are chosen for implementation in EnergyPlus. Li et al’s extensive dataset of over 30,000 data points and subsequent analysis provides conclusive results that both of these models will accurately estimate sky emissivity. The calibrated forms of Martin & Berdahl, Brunt, and Idso from Li et alare listed:

Martin & Berdahl Calibrated: ε = 0.758 + 0.521・(T<sub>d</sub>/100) + 0.625・(T<sub>d</sub>/100)^2  &nbsp;&nbsp;    Eq. (4)

Brunt Calibrated: ε = 0.618 + 0.056・(P<sub>w</sub>)^1/2 	&nbsp;&nbsp;    Eq. (5)

Idso Calibrated: ε = 0.685 + 3.2・10E-5・P<sub>w</sub>・exp(1699/T<sub>a</sub>)  &nbsp;&nbsp;    Eq. (6)

Table 1 summarizes studies that examined the performance of the original and calibrated forms in calculating longwave radiation (LW, W/m<sup>2</sup>), which is a function of sky emissivity and ambient temperature:

LW = ε ∙ 5.6697 ∙ 10E-8 ∙ T<sub>a</sub><sup>4</sup>  &nbsp;&nbsp;    Eq (7)

**Table 1. Summary of reported clear sky atmospheric longwave radiation errors compared to observed data**

| Reference                        | Original model  RMSE (W/m2)     | Calibrated model  RMSE (W/m2)   |
|----------------------------------|---------------------------------|---------------------------------|
| Brunt(1931)                      | 32.24                           | 13.24                           |
| Idso(1981)                       | 14.03                           | 13.18                           |
| Berdahl & Martine (1984)         | 22.42                           | 13.24                           |

This improved sky emissivity feature would allow the user to select one of the three models, the calibrated Brunt (1932), the calibrated Idso (1981), and Clark-Allen (1978) using common meteorological inputs of ambient temperature (all), dewpoint temperature (Clark-Allen), and water vapor pressure (Idso and Brunt). The user can expect errors within 4-5% or around 14 W/m<sup>2</sup> when using these models as reported by Li et al.

## Approach ##

We propose to add a new field to the existing `WeatherProperty:SkyTemperature` object to indicate the model to calculate the clear sky emissivity for calculating sky temperature.

```
WeatherProperty:SkyTemperature,
       \memo This object is used to define alternative sky temperature calculation or schedule import methods.
  A1,  \field Name
       \note blank in this field will apply to all run periods (that is, all objects=
       \note SizingPeriod:WeatherFileDays, SizingPeriod:WeatherFileConditionType or RunPeriod
       \note otherwise, this name must match one of the environment object names.
       \type object-list
       \object-list RunPeriodsAndDesignDays
  A2,  \field Calculation Type
       \required-field
       \note The field indicates that the sky temperature will be imported from external schedules or calculated by alternative methods other than default.
       \type choice
       \key ScheduleValue
       \key DifferenceScheduleDryBulbValue
       \key DifferenceScheduleDewPointValue
       \key UseBruntModel
       \key UseIdsoModel
       \key UseBerdahlMartinModel
  A3;  \field Schedule Name
       \note if name matches a SizingPeriod:DesignDay, put in a day schedule of this name
       \note if name is for a SizingPeriod:WeatherFileDays, SizingPeriod:WeatherFileConditionType or
       \note RunPeriod, put in a full year schedule that covers the appropriate days.
       \note Required if Calculation Type is ScheduleValue, DifferenceScheduleDryBulbValue or DifferenceScheduleDewPointValue.
       \type object-list
       \object-list DayScheduleNames
       \object-list ScheduleNames
```

## Testing/Validation/Data Source(s) ##

With the example file - DOE reference small office model, we will compare the sky emissivity and temperature calculation results between models.

## Input Output Reference Documentation ##

To be developed.

## Input Description ##

The `Calculation Type` field under the _WeatherProperty:SkyTemperature_ object will be modified to take two new keys as the choice:

- **UseBerdahlMartinModel**
- **UseBruntModel**
- **UseIdsoModel**

## Outputs Description ##

N/A

## Engineering Reference ##

To be developed.

## Example Files and Transition Changes ##

The existing DOE reference small office model will be modified to use alternative clear sky emissivity calculation methos.

No transition change is required.

## References ##

[1] M. Cucumo, A. De Rosa, and V. Marinelli, “Experimental testing of correlations to calculate the atmospheric ‘transparency window’ emissivity coefficient,” Sol. Energy, vol. 80, no. 8, pp. 1031–1038, Aug. 2006.

[2] L. Evangelisti, C. Guattari, and F. Asdrubali, “On the sky temperature models and their influence on buildings energy performance: A critical review,” Energy Build., vol. 183, pp. 607–625, 2019.

[3] G. Clark, C. Allen, "The Estimation of Atmospheric Radiation for Clear and Cloudy Skies," In Proceedings of the 2nd National Passive Solar Conference, pp. 675–678, 1978.

[4] P. Berdahl, M. Martin, "Emissivity of ClearSkies," Solar Energy, 32(5), pp.663–664, 1984.

[5] Q. Dai and X. Fang, “A new model for atmospheric radiation under clear sky condition at various altitudes,” Adv. Sp. Res., vol. 54, no. 6, pp. 1044–1048, Sep. 2014.

[6] K. Zhang, T. P. Mcdowell, and M. Kummert, “Sky Temperature Estimation and Measurement for Longwave Radiation Calculation,” no. 3, pp. 769–778, 2017.

[7] M. Li, Y. Jiang, and C. F. M. Coimbra, “On the determination of atmospheric longwave irradiance under all-sky conditions,” Sol. Energy, vol. 144, pp. 40–48, 2017.

[8] D. Brunt, “Notes on Radiation in the Atmosphere,” 1932.

[9] S. B. Idso, “A set of equations for full spectrum and 8-14 um thermal radiation from cloudless skies,” Water Resour. Res., vol. 17, no. 2, pp. 295–304, 1981.

[10]  M. Choi, J. M. Jacobs, and W. P. Kustas, “Assessment of clear and cloudy sky parameterizations for daily downwelling longwave radiation over different land surfaces in Florida, USA,” Geophys. Res. Lett., vol. 35, no. 20, 2008.

[11]  F. Carmona, R. Rivas, and V. Caselles, “Estimation of daytime downward longwave radiation under clear and cloudy skies conditions over a sub-humid region,” Theor. Appl. Climatol., vol. 115, no. 1–2, pp. 281–295, 2014.

[12] M. G. G. Iziomon, H. Mayer, and A. Matzarakis, “Downward atmospheric longwave irradiance under clear and cloudy skies: Measurement and parameterization,” J. Atmos. Solar-Terrestrial Phys., vol. 65, no. 10, pp. 1107–1116, Jul. 2003.



