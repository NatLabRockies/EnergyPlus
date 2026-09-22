#ifndef PERFORATEDCELLDESCRIPTION_H
#define PERFORATEDCELLDESCRIPTION_H

#include "CellDescription.hpp"

namespace SingleLayerOptics
{
    class CPerforatedCellDescription : public ICellDescription
    {
    public:
        CPerforatedCellDescription(const double t_x, const double t_y, const double t_Thickness);

        double R_dir_dir(const FenestrationCommon::Side t_Side, const CBeamDirection & t_Direction);

    protected:
        double m_x;
        double m_y;
        double m_Thickness;
    };

    class CCircularCellDescription : public CPerforatedCellDescription
    {
    public:
        CCircularCellDescription(const double t_x,
                                 const double t_y,
                                 const double t_Thickness,
                                 const double t_Radius);

        double T_dir_dir(const FenestrationCommon::Side t_Side, const CBeamDirection & t_Direction);

        [[nodiscard]] double xDimension() const;
        [[nodiscard]] double yDimension() const;
        [[nodiscard]] double thickness() const;
        [[nodiscard]] double radius() const;

    private:
        [[nodiscard]] double visibleAhole(const CBeamDirection & t_Direction) const;
        [[nodiscard]] double visibleAcell(const CBeamDirection & t_Direction) const;
        // double m_x;
        // double m_y;
        // double m_Thickness;
        double m_Radius;
    };

    class CRectangularCellDescription : public CPerforatedCellDescription
    {
    public:
        CRectangularCellDescription(const double t_x,
                                    const double t_y,
                                    const double t_Thickness,
                                    const double t_XHole,
                                    const double t_YHole);

        double T_dir_dir(const FenestrationCommon::Side t_Side, const CBeamDirection & t_Direction);

        [[nodiscard]] double xDimension() const;
        [[nodiscard]] double yDimension() const;
        [[nodiscard]] double thickness() const;
        [[nodiscard]] double xHole() const;
        [[nodiscard]] double yHole() const;

    private:
        double TransmittanceV(const CBeamDirection & t_Direction) const;
        double TransmittanceH(const CBeamDirection & t_Direction) const;

        // double m_x;
        // double m_y;
        // double m_Thickness;
        double m_XHole;
        double m_YHole;
    };

}   // namespace SingleLayerOptics

#endif
