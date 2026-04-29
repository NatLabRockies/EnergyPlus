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

#ifndef DataVectorTypes_hh_INCLUDED
#define DataVectorTypes_hh_INCLUDED

// C++ Headers
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Vector3.hh>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataVectorTypes {

    // Using/Aliasing

    // Data
    // -only module should be available to other modules and routines.
    // Thus, all variables in this module must be PUBLIC.

    // MODULE PARAMETER DEFINITIONS:
    // na

    // DERIVED TYPE DEFINITIONS

    // the following two derived types are used in triangulation (for DXF outputs)
    //'Points (Vertices)

    //'Created Triangles, vv# are the vertex pointers

    // Types

    // Vector2/3 are integrated with Array and offer additional capabilities such as
    //  subscript lookup and are templates so we are using them as plug replacements
    //  for consistent API and to avoid cost of copying them
    // Note: For vectorization contexts std::array is a better choice
    using Vector = ObjexxFCL::Vector3<Real64>;
    using Vector_2d = ObjexxFCL::Vector2<Real64>;
    using ObjexxFCL::cross;

    struct Vector2dCount : Vector_2d
    {
        int count{};
        Vector2dCount() = default;
    };

    //    struct Vector // This is used to specify a point in 3D space
    //    {
    //        // Members
    //        // Right Handed Coordinate system is used
    //        Real64 x;
    //        Real64 y;
    //        Real64 z;
    //
    //        // Default Constructor
    //        Vector()
    //        {}
    //
    //
    //        // Uniform Real64 Constructor
    //        Vector( Real64 const v ) :
    //            x( v ),
    //            y( v ),
    //            z( v )
    //        {}
    //
    //        // Array Assignment
    //        inline
    //        Vector &
    //        operator =( Array1D< Real64 > const & a )
    //        {
    //            assert( ( a.l() == 1 ) && ( a.u() == 3 ) );
    //            x = a( 1 );
    //            y = a( 2 );
    //            z = a( 3 );
    //            return *this;
    //        }
    //
    //        // Array Assignment
    //        inline
    //        Vector &
    //        operator =( Array1A< Real64 > const a )
    //        {
    //            a.dim( 3 );
    //            x = a( 1 );
    //            y = a( 2 );
    //            z = a( 3 );
    //            return *this;
    //        }
    //
    //        // Array Assignment
    //        inline
    //        Vector &
    //        operator =( Array1S< Real64 > const & a )
    //        {
    //            assert( ( a.l() == 1 ) && ( a.u() == 3 ) );
    //            x = a( 1 );
    //            y = a( 2 );
    //            z = a( 3 );
    //            return *this;
    //        }
    //
    //        // Vector3 Assignment
    //        inline
    //        Vector &
    //        operator =( Vector3< Real64 > const & v )
    //        {
    //            x = v.x;
    //            y = v.y;
    //            z = v.z;
    //            return *this;
    //        }
    //
    //        // Real64 Assignment
    //        inline
    //        Vector &
    //        operator =( Real64 const v )
    //        {
    //            x = v;
    //            y = v;
    //            z = v;
    //            return *this;
    //        }
    //
    //        // += Vector
    //        inline
    //        Vector &
    //        operator +=( Vector const & a )
    //        {
    //            x += a.x;
    //            y += a.y;
    //            z += a.z;
    //            return *this;
    //        }
    //
    //        // -= Vector
    //        inline
    //        Vector &
    //        operator -=( Vector const & a )
    //        {
    //            x -= a.x;
    //            y -= a.y;
    //            z -= a.z;
    //            return *this;
    //        }
    //
    //        // += Real64
    //        inline
    //        Vector &
    //        operator +=( Real64 const v )
    //        {
    //            x += v;
    //            y += v;
    //            z += v;
    //            return *this;
    //        }
    //
    //        // -= Real64
    //        inline
    //        Vector &
    //        operator -=( Real64 const v )
    //        {
    //            x -= v;
    //            y -= v;
    //            z -= v;
    //            return *this;
    //        }
    //
    //        // *= Real64
    //        inline
    //        Vector &
    //        operator *=( Real64 const v )
    //        {
    //            x *= v;
    //            y *= v;
    //            z *= v;
    //            return *this;
    //        }
    //
    //        // /= Real64
    //        inline
    //        Vector &
    //        operator /=( Real64 const v )
    //        {
    //            assert( v != Real64( 0.0 ) );
    //            x /= v;
    //            y /= v;
    //            z /= v;
    //            return *this;
    //        }
    //
    //        // Array Conversion
    //        inline
    //        operator Array1D< Real64 >() const
    //        {
    //            return Array1D< Real64 >( 3, { x, y, z } );
    //        }
    //
    //        // Length
    //        inline
    //        Real64
    //        length() const
    //        {
    //            return std::sqrt( ( x * x ) + ( y * y ) + ( z * z ) );
    //        }
    //
    //        // Length
    //        inline
    //        Real64
    //        length_squared() const
    //        {
    //            return ( x * x ) + ( y * y ) + ( z * z );
    //        }
    //
    //        // Negated
    //        inline
    //        friend
    //        Vector
    //        operator -( Vector const & a )
    //        {
    //            return Vector( -a.x, -a.y, -a.z );
    //        }
    //
    //        // Vector + Vector
    //        inline
    //        friend
    //        Vector
    //        operator +( Vector const & a, Vector const & b )
    //        {
    //            Vector r;
    //            r.x = a.x + b.x;
    //            r.y = a.y + b.y;
    //            r.z = a.z + b.z;
    //            return r;
    //        }
    //
    //        // Vector - Vector
    //        inline
    //        friend
    //        Vector
    //        operator -( Vector const & a, Vector const & b )
    //        {
    //            Vector r;
    //            r.x = a.x - b.x;
    //            r.y = a.y - b.y;
    //            r.z = a.z - b.z;
    //            return r;
    //        }
    //
    //        // Vector * Vector: Cross Product //Autodesk Suggest migrating to cross function to avoid confusion
    //        inline
    //        friend
    //        Vector
    //        operator *( Vector const & a, Vector const & b )
    //        {
    //            Vector c;
    //            c.x = ( a.y * b.z ) - ( a.z * b.y );
    //            c.y = ( a.z * b.x ) - ( a.x * b.z );
    //            c.z = ( a.x * b.y ) - ( a.y * b.x );
    //            return c;
    //        }
    //
    //        // Vector * Real64
    //        inline
    //        friend
    //        Vector
    //        operator *( Vector const & a, Real64 const b )
    //        {
    //            Vector r;
    //            r.x = a.x * b;
    //            r.y = a.y * b;
    //            r.z = a.z * b;
    //            return r;
    //        }
    //
    //        // Real64 * Vector
    //        inline
    //        friend
    //        Vector
    //        operator *( Real64 const b, Vector const & a )
    //        {
    //            Vector r;
    //            r.x = a.x * b;
    //            r.y = a.y * b;
    //            r.z = a.z * b;
    //            return r;
    //        }
    //
    //        // Vector * Integer
    //        inline
    //        friend
    //        Vector
    //        operator *( Vector const & a, int const b )
    //        {
    //            Vector r;
    //            r.x = a.x * b;
    //            r.y = a.y * b;
    //            r.z = a.z * b;
    //            return r;
    //        }
    //
    //        // Integer * Vector
    //        inline
    //        friend
    //        Vector
    //        operator *( int const b, Vector const & a )
    //        {
    //            Vector r;
    //            r.x = a.x * b;
    //            r.y = a.y * b;
    //            r.z = a.z * b;
    //            return r;
    //        }
    //
    //        // Vector / Real64
    //        inline
    //        friend
    //        Vector
    //        operator /( Vector const & a, Real64 const b )
    //        {
    //            assert( b != 0.0 );
    //            Vector r;
    //            r.x = a.x / b;
    //            r.y = a.y / b;
    //            r.z = a.z / b;
    //            return r;
    //        }
    //
    //        // Vector / Integer
    //        inline
    //        friend
    //        Vector
    //        operator /( Vector const & a, int const b )
    //        {
    //            assert( b != 0 );
    //            Vector r;
    //            r.x = a.x / b;
    //            r.y = a.y / b;
    //            r.z = a.z / b;
    //            return r;
    //        }
    //
    //        // Magnitude
    //        inline
    //        friend
    //        Real64
    //        magnitude( Vector const & a )
    //        {
    //            return std::sqrt( square( a.x ) + square( a.y ) + square( a.z ) );
    //        }
    //
    //        // Magnitude Squared
    //        inline
    //        friend
    //        Real64
    //        magnitude_squared( Vector const & a )
    //        {
    //            return square( a.x ) + square( a.y ) + square( a.z );
    //        }
    //
    //        // Distance
    //        inline
    //        friend
    //        Real64
    //        distance( Vector const & a, Vector const & b )
    //        {
    //            return std::sqrt( square( a.x - b.x ) + square( a.y - b.y ) + square( a.z - b.z ) );
    //        }
    //
    //        // Distance Squared
    //        inline
    //        friend
    //        Real64
    //        distance_squared( Vector const & a, Vector const & b )
    //        {
    //            return square( a.x - b.x ) + square( a.y - b.y ) + square( a.z - b.z );
    //        }
    //
    //        // Dot Product
    //        inline
    //        friend
    //        Real64
    //        dot( Vector const & a, Vector const & b )
    //        {
    //            return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
    //        }
    //
    //        // Dot Product
    //        inline
    //        friend
    //        Real64
    //        dot( Vector const & a, Vector3< Real64 > const & b )
    //        {
    //            return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
    //        }
    //
    //        // Dot Product
    //        inline
    //        friend
    //        Real64
    //        dot( Vector3< Real64 > const & a, Vector const & b )
    //        {
    //            return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
    //        }
    //
    //        // Cross Product
    //        inline
    //        friend
    //        Vector
    //        cross( Vector const & a, Vector const & b )
    //        {
    //            Vector c;
    //            c.x = ( a.y * b.z ) - ( a.z * b.y );
    //            c.y = ( a.z * b.x ) - ( a.x * b.z );
    //            c.z = ( a.x * b.y ) - ( a.y * b.x );
    //            return c;
    //        }
    //
    //        // Cross Product
    //        inline
    //        friend
    //        Vector
    //        cross( Vector const & a, Vector3< Real64 > const & b )
    //        {
    //            Vector c;
    //            c.x = ( a.y * b.z ) - ( a.z * b.y );
    //            c.y = ( a.z * b.x ) - ( a.x * b.z );
    //            c.z = ( a.x * b.y ) - ( a.y * b.x );
    //            return c;
    //        }
    //
    //        // Cross Product
    //        inline
    //        friend
    //        Vector
    //        cross( Vector3< Real64 > const & a, Vector const & b )
    //        {
    //            Vector c;
    //            c.x = ( a.y * b.z ) - ( a.z * b.y );
    //            c.y = ( a.z * b.x ) - ( a.x * b.z );
    //            c.z = ( a.x * b.y ) - ( a.y * b.x );
    //            return c;
    //        }
    //
    //        // Array Generator
    //        inline
    //        Array1D< Real64 >
    //        Array() const
    //        {
    //            return Array1D< Real64 >( 3, { x, y, z } );
    //        }
    //
    //        // Vector3 Generator
    //        inline
    //        Vector3< Real64 >
    //        Vec3() const
    //        {
    //            return Vector3< Real64 >( x, y, z );
    //        }
    //
    //        // Assign to an Array
    //        inline
    //        void
    //        assign_to( Array1D< Real64 > & a ) const
    //        {
    //            a.dimension( 3 );
    //            a( 1 ) = x;
    //            a( 2 ) = y;
    //            a( 3 ) = z;
    //        }
    //
    //        // Assign to a Vector3
    //        inline
    //        void
    //        assign_to( Vector3< Real64 > & v ) const
    //        {
    //            v.x = x;
    //            v.y = y;
    //            v.z = z;
    //        }
    //
    //    private: // Static Functions
    //
    //        // Square
    //        inline
    //        static
    //        Real64
    //        square( Real64 const x )
    //        {
    //            return x * x;
    //        }
    //
    //    };

    struct PlaneEq // This is used to specify a plane based on vectors in that plane
    {
        // Members
        Real64 x{};
        Real64 y{};
        Real64 z{};
        Real64 w{};

        // Default Constructor
        PlaneEq() = default;
    };

    struct Face // Used to specify the face of a polyhedron
    {
        // Members
        int NSides{};  // Number of Sides for this Face
        int SurfNum{}; // ALLOCATABLE to actual surface number
        Array1D<Vector> FacePoints;
        Vector NewellAreaVector;

        // Default Constructor
        Face() = default;
    };

    struct Polyhedron // This is used to specify a polyhedron based on the vectors that comprise it (a zone).
    {
        // Members
        int NumSurfaceFaces{};
        Array1D<Face> SurfaceFace;

        // Default Constructor
        Polyhedron() = default;
    };

    //    struct Vector_2d
    //    {
    //        // Members
    //        Real64 x;
    //        Real64 y;
    //
    //        // Default Constructor
    //        Vector_2d()
    //        {}
    //
    //
    //        // Dot Product
    //        inline
    //        friend
    //        Real64
    //        dot( Vector_2d const & a, Vector_2d const & b )
    //        {
    //            return ( a.x * b.x ) + ( a.y * b.y );
    //        }
    //
    //        // Cross Product
    //        inline
    //        friend
    //        Real64
    //        cross( Vector_2d const & a, Vector_2d const & b )
    //        {
    //            return ( a.x * b.y ) - ( a.y * b.x );
    //        }
    //
    //    };

    struct dTriangle
    {
        // Members
        int vv0{};
        int vv1{};
        int vv2{};

        // Default Constructor
        dTriangle() = default;
    };

} // namespace DataVectorTypes

} // namespace EnergyPlus

#endif
