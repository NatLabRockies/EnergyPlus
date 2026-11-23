#include <cmath>
#include <WCECommon.hpp>
#include "EffectiveOpenness.hpp"

namespace EffectiveLayers
{
    ShadeOpenness::ShadeOpenness(double ah, double dl, double dr, double dtop, double dbot) :
        Ah(ah),
        Dl(dl),
        Dr(dr),
        Dtop(dtop),
        Dbot(dbot)
    {}

    EffectiveOpenness::EffectiveOpenness(const double ah,
                                         const double al,
                                         const double ar,
                                         const double atop,
                                         const double abot,
                                         const double frontPorosity) :
        Ah(ah),
        Al(al),
        Ar(ar),
        Atop(atop),
        Abot(abot),
        FrontPorosity(frontPorosity)
    {}

    bool EffectiveOpenness::isClosed() const
    {
        return Ah == 0.0 && Al == 0.0 && Ar == 0.0 && Atop == 0.0 && Abot == 0.0
               && FrontPorosity == 0.0;
    }

    EffectiveLayer::EffectiveLayer(double width,
                                   double height,
                                   double thickness,
                                   const ShadeOpenness & openness,
                                   Coefficients t_coefficients) :
        m_Width(width),
        m_Height(height),
        m_Thickness(thickness),
        m_ShadeOpenness(
          openness.Ah * width * height, openness.Dl, openness.Dr, openness.Dtop, openness.Dbot),
        coefficients{t_coefficients}
    {}

    Coefficients::Coefficients(double c1, double c2, double c3, double c4) :
        C1(c1),
        C2(c2),
        C3(c3),
        C4(c4)
    {}

    EffectiveVenetian::EffectiveVenetian(double width,
                                         double height,
                                         double thickness,
                                         const ShadeOpenness & openness,
                                         double slatAngle,
                                         double slatWidth,
                                         Coefficients t_coefficients) :
        EffectiveLayer(width, height, thickness, openness, t_coefficients),
        m_SlatAngleRad(slatAngle * 2.0 * ConstantsData::WCE_PI / 360.0),
        m_SlatWidth(slatWidth)
    {}

    EffectiveOpenness EffectiveVenetian::getEffectiveOpenness()
    {
        const auto area{m_Width * m_Height};
        const auto Ah_eff{area * coefficients.C1
                          * std::pow(m_ShadeOpenness.Ah / area
                                       * std::pow(std::cos(m_SlatAngleRad), coefficients.C2),
                                     coefficients.C3)};

        return {Ah_eff,
                0,
                0,
                m_ShadeOpenness.Dtop * m_Width,
                m_ShadeOpenness.Dbot * m_Width,
                m_ShadeOpenness.Ah};
    }

    double EffectiveVenetian::effectiveThickness()
    {
        return coefficients.C4
               * (m_SlatWidth * std::cos(m_SlatAngleRad) + m_Thickness * std::sin(m_SlatAngleRad));
    }

    EffectiveHorizontalVenetian::EffectiveHorizontalVenetian(double width,
                                                             double height,
                                                             double thickness,
                                                             const ShadeOpenness & openness,
                                                             double slatAngle,
                                                             double slatWidth) :
        EffectiveVenetian(
          width, height, thickness, openness, slatAngle, slatWidth, {0.016, -0.63, 0.53, 0.043})
    {}

    EffectiveVerticalVenentian::EffectiveVerticalVenentian(double width,
                                                           double height,
                                                           double thickness,
                                                           const ShadeOpenness & openness,
                                                           double slatAngle,
                                                           double slatWidth) :
        EffectiveVenetian(
          width, height, thickness, openness, slatAngle, slatWidth, {0.041, 0.0, 0.27, 0.012})
    {}

    EffectiveLayerType1::EffectiveLayerType1(double width,
                                             double height,
                                             double thickness,
                                             const ShadeOpenness & openness) :
        EffectiveLayer(width, height, thickness, openness, {0.078, 1.2, 1.0, 1.0})
    {}

    EffectiveOpenness EffectiveLayerType1::getEffectiveOpenness()
    {
        const auto area{m_Width * m_Height};
        const auto Ah_eff{area * coefficients.C1
                          * (std::pow(m_ShadeOpenness.Ah / area, coefficients.C2))};
        const auto Al_eff{m_ShadeOpenness.Dl * m_Height * coefficients.C3};
        const auto Ar_eff{m_ShadeOpenness.Dr * m_Height * coefficients.C3};
        const auto Atop_eff{m_ShadeOpenness.Dtop * m_Width * coefficients.C4};
        const auto Abop_eff{m_ShadeOpenness.Dbot * m_Width * coefficients.C4};
        return {Ah_eff, Al_eff, Ar_eff, Atop_eff, Abop_eff, m_ShadeOpenness.Ah};
    }

    double EffectiveLayerType1::effectiveThickness()
    {
        return m_Thickness;
    }

    EffectiveLayerPerforated::EffectiveLayerPerforated(double width,
                                                       double height,
                                                       double thickness,
                                                       const ShadeOpenness & openness) :
        EffectiveLayerType1(width, height, thickness, openness)
    {}

    EffectiveLayerDiffuse::EffectiveLayerDiffuse(double width,
                                                 double height,
                                                 double thickness,
                                                 const ShadeOpenness & openness) :
        EffectiveLayerType1(width, height, thickness, openness)
    {}

    EffectiveLayerWoven::EffectiveLayerWoven(double width,
                                             double height,
                                             double thickness,
                                             const ShadeOpenness & openness) :
        EffectiveLayerType1(width, height, thickness, openness)
    {}

    EffectiveLayerBSDF::EffectiveLayerBSDF(double width,
                                           double height,
                                           double thickness,
                                           const ShadeOpenness & openness) :
        EffectiveLayerType1(width, height, thickness, openness)
    {}

    EffectiveLayerOther::EffectiveLayerOther(double width,
                                             double height,
                                             double thickness,
                                             const ShadeOpenness & openness) :
        EffectiveLayer(width, height, thickness, openness)
    {}
    EffectiveOpenness EffectiveLayerOther::getEffectiveOpenness()
    {
        return {m_ShadeOpenness.Ah,
                m_ShadeOpenness.Dl * m_Height,
                m_ShadeOpenness.Dr * m_Height,
                m_ShadeOpenness.Dtop * m_Width,
                m_ShadeOpenness.Dbot * m_Width,
                m_ShadeOpenness.Ah};
    }

    double EffectiveLayerOther::effectiveThickness()
    {
        return m_Thickness;
    }
}   // namespace EffectiveLayers
