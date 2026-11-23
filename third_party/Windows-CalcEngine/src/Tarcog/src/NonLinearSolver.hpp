#ifndef TARNONLINEARSOLVER_H
#define TARNONLINEARSOLVER_H

#include <memory>
#include <vector>

#include "HeatFlowBalance.hpp"
#include "IGU.hpp"

namespace Tarcog
{
    namespace ISO15099
    {
        class CNonLinearSolver
        {
        public:
            explicit CNonLinearSolver(CIGU & t_IGU, size_t numberOfIterations = 0u);

            // sets tolerance for solution
            void setTolerance(double t_Tolerance);

            // returns number of iterations for current solution.
            size_t getNumOfIterations() const;

            void solve();

            double solutionTolerance() const;
            bool isToleranceAchieved() const;

        private:
            double calculateTolerance(const std::vector<double> & t_Solution) const;
            void estimateNewState(const std::vector<double> & t_Solution);

            CIGU & m_IGU;
            CHeatFlowBalance m_QBalance;
            std::vector<double> m_IGUState;
            double m_Tolerance;
            size_t m_Iterations;
            double m_RelaxParam;
            double m_SolutionTolerance;
        };

    }   // namespace ISO15099

}   // namespace Tarcog


#endif
