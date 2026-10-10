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

#ifndef ENERGYPLUS_GEOMETRY_VECTOR2D_INCLUDED
#define ENERGYPLUS_GEOMETRY_VECTOR2D_INCLUDED

// C++ Headers
#include <cassert>
#include <cmath>
#include <cstddef>
#include <ostream>

namespace EnergyPlus {

class Vector2D
{
public:
    /// @name Data Elements
    //@{
    double x = 0.0;
    double y = 0.0;
    //@}

    /// @name Creation
    //@{
    /// Default Constructor: Zero-Initializes All Elements
    constexpr Vector2D() = default;

    /// Uniform Value Constructor
    explicit constexpr Vector2D(double t) : x(t), y(t)
    {
    }

    /// Value Constructor
    constexpr Vector2D(double x_, double y_) : x(x_), y(y_)
    {
    }
    //@}

    /// @name Scalar multiplication and division
    //@{
    // Scalar Compound Assignment
    /// *= Scalar
    constexpr Vector2D &operator*=(double t)
    {
        x *= t;
        y *= t;
        return *this;
    }

    /// /= Scalar
    constexpr Vector2D &operator/=(double const u)
    {
        assert(u != 0.0);
        double const inv_u = 1.0 / u;
        x *= inv_u;
        y *= inv_u;
        return *this;
    }

    // Scalar Binary Operators
    friend constexpr Vector2D operator*(Vector2D const &v, double t)
    {
        return {v.x * t, v.y * t};
    }

    friend constexpr Vector2D operator*(double t, Vector2D const &v)
    {
        return {t * v.x, t * v.y};
    }

    friend constexpr Vector2D operator/(Vector2D const &v, double u)
    {
        assert(u != 0.0);
        double const inv_u = 1.0 / u;
        return {v.x * inv_u, v.y * inv_u};
    }
    //@}

    /// @name Vector2D Addition and Subtraction
    //@{
    // Compound Assignment and Unary negation
    constexpr Vector2D &operator+=(Vector2D const &v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    constexpr Vector2D &operator-=(Vector2D const &v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    constexpr Vector2D operator-() const
    {
        return {-x, -y};
    }

    // Addition and Subtraction: Binary Operators
    friend constexpr Vector2D operator+(Vector2D const &a, Vector2D const &b)
    {
        return {a.x + b.x, a.y + b.y};
    }

    friend constexpr Vector2D operator-(Vector2D const &a, Vector2D const &b)
    {
        return {a.x - b.x, a.y - b.y};
    }
    //@}

    /// @name Comparison
    //@{
    /// Exact comparison of the components (also provides !=)
    friend constexpr bool operator==(Vector2D const &, Vector2D const &) = default;
    //@}

    /// @name Subscript
    //@{
    /// Vector2D[ i ] const: 0-Based Index
    constexpr double operator[](std::size_t i) const
    {
        assert(i <= 1);
        return (i == 0 ? x : y);
    }

    /// Vector2D[ i ]: 0-Based Index
    constexpr double &operator[](std::size_t i)
    {
        assert(i <= 1);
        return (i == 0 ? x : y);
    }
    //@}

    /// @name Queries
    //@{
    /// Length (L2 norm)
    double length() const
    {
        return std::sqrt(length_squared());
    }

    /// Length Squared
    constexpr double length_squared() const
    {
        return (x * x) + (y * y);
    }

    /// Distance to a Vector2D
    double distance(Vector2D const &v) const
    {
        return (v - *this).length();
    }

    /// Distance Squared to a Vector2D
    constexpr double distance_squared(Vector2D const &v) const
    {
        return (v - *this).length_squared();
    }

    /// Dot Product with a Vector2D
    constexpr double dot(Vector2D const &v) const
    {
        return (x * v.x) + (y * v.y);
    }

    /// Cross Product with a Vector2D
    constexpr double cross(Vector2D const &v) const
    {
        return (x * v.y) - (y * v.x);
    }
    //@}

    /// @name Normalization
    //@{
    /// Normalize to a Length (in-place)
    Vector2D &normalize(double tar_length = 1.0)
    {
        double const cur_length = length();
        assert(cur_length != 0.0);
        *this *= tar_length / cur_length;
        return *this;
    }

    /// Normalized to a Length (return a new vector)
    Vector2D normalized(double tar_length = 1.0) const
    {
        double const cur_length = length();
        assert(cur_length != 0.0);
        return *this * (tar_length / cur_length);
    }
    //@}

}; // Vector2D

/// Stream << Vector2D output operator
inline std::ostream &operator<<(std::ostream &os, Vector2D const &v)
{
    os << "[" << v.x << ", " << v.y << "]";
    return os;
}

} // namespace EnergyPlus

#endif // ENERGYPLUS_GEOMETRY_VECTOR2D_INCLUDED
