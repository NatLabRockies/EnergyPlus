#ifndef ObjexxFCL_Array1S_hh_INCLUDED
#define ObjexxFCL_Array1S_hh_INCLUDED

// Array1S: 1D Slice Array Proxy
//
// Project: Objexx Fortran-C++ Library (ObjexxFCL)
//
// Version: 4.2.0
//
// Language: C++
//
// Copyright (c) 2000-2017 Objexx Engineering, Inc. All Rights Reserved.
// Use of this source code or any derivative of it is restricted by license.
// Licensing is available from Objexx Engineering, Inc.:  http://objexx.com

// ObjexxFCL Headers
#include <ObjexxFCL/Array1S.fwd.hh>
#include <ObjexxFCL/ArrayRS.hh>
#include <ObjexxFCL/CArray.hh>
#include <ObjexxFCL/Vector2.hh>
#include <ObjexxFCL/Vector3.hh>
#include <ObjexxFCL/Vector4.hh>

// C++ Headers
#include <array>
#include <vector>

namespace ObjexxFCL {

// Array1S: 1D Slice Array Proxy
template< typename T >
class Array1S : public ArrayRS< T, 1 >
{

private: // Types

	typedef  ArrayRS< T, 1 >  Super;

private: // Friend

	template< typename > friend class Array1S;

public: // Types

	typedef  typename Super::Base  Base;
	typedef  typename Super::Traits  Traits;
	typedef  typename Super::IR  IR;
	typedef  typename Super::IS  IS;
	typedef  typename Super::DS  DS;

	// STL Style
	typedef  typename Super::value_type  value_type;
	typedef  typename Super::reference  reference;
	typedef  typename Super::const_reference  const_reference;
	typedef  typename Super::pointer  pointer;
	typedef  typename Super::const_pointer  const_pointer;
	typedef  typename Super::size_type  size_type;
	typedef  typename Super::difference_type  difference_type;

	// C++ Style
	typedef  typename Super::Value  Value;
	typedef  typename Super::Reference  Reference;
	typedef  typename Super::ConstReference  ConstReference;
	typedef  typename Super::Pointer  Pointer;
	typedef  typename Super::ConstPointer  ConstPointer;
	typedef  typename Super::Size  Size;
	typedef  typename Super::Difference  Difference;

	using Super::isize;
	using Super::overlap;
	using Super::size;

protected: // Types

	using Super::in_range;
	using Super::slice_k;

	using Super::contiguous_;
	using Super::data_;
	using Super::data_beg_;
	using Super::data_end_;
	using Super::size_;

public: // Creation

	// Default Constructor
	Array1S() :
	 m_( 1 ),
	 k_( 0 ),
	 u_( 0 )
	{}

	// Copy Constructor
	Array1S( Array1S const & a ) :
	 Super( a ),
	 m_( a.m_ ),
	 k_( a.k_ ),
	 u_( a.u_ )
	{
		data_set();
	}

	// Data Constructor
	Array1S( T const * data, std::int64_t const k, DS const & d ) :
	 Super( data, d.z() ),
	 m_( d.m() ),
	 k_( k + d.k() ),
	 u_( d.u() )
	{
		contiguous_ = computed_contiguous();
		data_set();
	}

	// Non-Const Data Constructor
	Array1S( T * data, std::int64_t const k, DS const & d ) :
	 Super( data, d.z() ),
	 m_( d.m() ),
	 k_( k + d.k() ),
	 u_( d.u() )
	{
		contiguous_ = computed_contiguous();
		data_set();
	}

	// Array Constructor
	template< template< typename > class A >
	Array1S( A< T > const & a ) :
	 Super( a.data(), a.size() ),
	 m_( 1 ),
	 k_( -m_ ),
	 u_( a.isize() )
	{
		contiguous_ = true;
		data_set();
	}

	// Destructor
	virtual
	~Array1S() = default;

public: // Assignment: Array

	// Copy Assignment
	Array1S &
	operator =( Array1S const & a )
	{
		if ( this != &a ) {
			assert( conformable( a ) );
			if ( overlap( a ) ) { // Overlap-safe
				CArray< T > c( size_ );
				for ( int i = 1; i <= u_; ++i ) {
					c( i ) = a( i );
				}
				for ( int i = 1; i <= u_; ++i ) {
					operator ()( i ) = c( i );
				}
			} else { // Not overlap-safe
				for ( int i = 1; i <= u_; ++i ) {
					operator ()( i ) = a( i );
				}
			}
		}
		return *this;
	}

	// += Array Template
	template< template< typename > class A >
	Array1S &
	operator +=( A< T > const & a )
	{
		assert( conformable( a ) );
		if ( overlap( a ) ) { // Overlap-safe
			CArray< T > c( size_ );
			for ( int i = 1, j = a.l(); i <= u_; ++i, ++j ) {
				c( i ) = a( j );
			}
			for ( int i = 1; i <= u_; ++i ) {
				operator ()( i ) += c( i );
			}
		} else { // Not overlap-safe
			for ( int i = 1, j = a.l(); i <= u_; ++i, ++j ) {
				operator ()( i ) += a( j );
			}
		}
		return *this;
	}

	// *= std::array Template
	template< typename U, Size s, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1S &
	operator *=( std::array< U, s > const & a )
	{
		assert( size_ == s );
		auto r( a.begin() );
		for ( int i = 1; i <= u_; ++i, ++r ) {
			operator ()( i ) *= *r;
		}
		return *this;
	}

	// += std::array Template
	template< typename U, Size s, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1S &
	operator +=( std::array< U, s > const & a )
	{
		assert( size_ == s );
		auto r( a.begin() );
		for ( int i = 1; i <= u_; ++i, ++r ) {
			operator ()( i ) += *r;
		}
		return *this;
	}

public: // Assignment: Value

	template< typename U, Size s, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1S &
	operator =( std::array< U, s > const & a )
	{
		assert( size_ == s );
		auto r( a.begin() );
		for ( int i = 1; i <= u_; ++i, ++r ) {
			operator ()( i ) = *r;
		}
		return *this;
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1S &
	operator =( std::initializer_list< U > const l )
	{
		assert( size_ == l.size() );
		auto r( l.begin() );
		for ( int i = 1; i <= u_; ++i, ++r ) {
			operator ()( i ) = *r;
		}
		return *this;
	}

	// = Value
	Array1S &operator=(T const &t) {
		for (int i = 1; i <= u_; ++i) {
			operator()(i) = t;
		}
		return *this;
	}

       	// *= Value
	Array1S &
	operator *=( T const & t )
	{
		for ( int i = 1; i <= u_; ++i ) {
			operator ()( i ) *= t;
		}
		return *this;
	}

	// /= Value
	template< typename U, class = typename std::enable_if< std::is_floating_point< U >::value && std::is_assignable< T&, U >::value >::type >
	Array1S &
	operator /=( U const & u )
	{
		assert( u != U( 0 ) );
		U const inv_u( U( 1 ) / u );
		for ( int i = 1; i <= u_; ++i ) {
			operator ()( i ) *= inv_u;
		}
		return *this;
	}

	// /= Value
	template< typename U, class = typename std::enable_if< ! std::is_floating_point< U >::value && std::is_assignable< T&, U >::value >::type, typename = void >
	Array1S &
	operator /=( U const & u )
	{
		assert( u != U( 0 ) );
		for ( int i = 1; i <= u_; ++i ) {
			operator ()( i ) /= u;
		}
		return *this;
	}
public: // Subscript

	// array( i ) const
	T const &
	operator ()( int const i ) const
	{
		assert( contains( i ) );
		return data_[ k_ + ( m_ * i ) ];
	}

	// array( i )
	T &
	operator ()( int const i )
	{
		assert( contains( i ) );
		return data_[ k_ + ( m_ * i ) ];
	}

	// Linear Index
	size_type
	index( int const i ) const
	{
		return k_ + ( m_ * i );
	}

	// array[ i ] const: 0-Based Subscript
	T const &
	operator []( size_type const i ) const
	{
		assert( i < std::numeric_limits< size_type >::max() );
		assert( contains( static_cast< int >( i + 1 ) ) );
		return data_[ k_ + ( m_ * ( i + 1 ) ) ];
	}

	// array[ i ]: 0-Based Subscript
	T &
	operator []( size_type const i )
	{
		assert( i < std::numeric_limits< int >::max() );
		assert( contains( static_cast< int >( i + 1 ) ) );
		return data_[ k_ + ( m_ * ( i + 1 ) ) ];
	}

public: // Slice Proxy Generators

	// array( s ) const
	Array1S
	operator ()( IS const & s ) const
	{
		DS const d( u_, s, m_ );
		return Array1S( data_, k_, d );
	}

	// array( s )
	Array1S
	operator ()( IS const & s )
	{
		DS const d( u_, s, m_ );
		return Array1S( data_, k_, d );
	}

	// array( {s} ) const
	template< typename U, class = typename std::enable_if< std::is_constructible< int, U >::value >::type >
	Array1S
	operator ()( std::initializer_list< U > const l ) const
	{
		IS const s( l );
		DS const d( u_, s, m_ );
		return Array1S( data_, k_, d );
	}

	// array( {s} )
	template< typename U, class = typename std::enable_if< std::is_constructible< int, U >::value >::type >
	Array1S
	operator ()( std::initializer_list< U > const l )
	{
		IS const s( l );
		DS const d( u_, s, m_ );
		return Array1S( data_, k_, d );
	}

public: // Predicate

	// Contains Indexed Element?
	bool
	contains( int const i ) const
	{
		return in_range( u_, i );
	}

	// Conformable?
	template< typename U >
	bool
	conformable( Array1S< U > const & a ) const
	{
		return ( u_ == a.u_ );
	}

	// Conformable?
	template< class A >
	bool
	conformable( A const & a ) const
	{
		return ( ( a.rank() == 1 ) && ( size_ == a.size() ) );
	}

	// Equal Dimensions?
	template< typename U >
	bool
	equal_dimensions( Array1S< U > const & a ) const
	{
		return conformable( a );
	}

	// Equal Dimensions?
	template< class A >
	bool
	equal_dimensions( A const & a ) const
	{
		return conformable( a );
	}

public: // Inspector

	// IndexRange of a Dimension
	IR
	I( int const d ) const
	{
		switch ( d ) {
		case 1:
			return I1();
		default:
			assert( false );
			return I1();
		}
	}

	// Upper Index of a Dimension
	int
	u( int const d ) const
	{
		switch ( d ) {
		case 1:
			return u_;
		default:
			assert( false );
			return u_;
		}
	}

	// Size of a Dimension
	size_type
	size( int const d ) const
	{
		switch ( d ) {
		case 1:
			return u_;
		default:
			assert( false );
			return u_;
		}
	}

	// Size of a Dimension
	int
	isize( int const d ) const
	{
		switch ( d ) {
		case 1:
			return u_;
		default:
			assert( false );
			return u_;
		}
	}

	// IndexRange
	IR
	I() const
	{
		return IR( 1, u_ );
	}

	// Lower Index
	int
	l() const
	{
		return 1;
	}

	// Upper Index
	int
	u() const
	{
		return u_;
	}

	// IndexRange of Dimension 1
	IR
	I1() const
	{
		return IR( 1, u_ );
	}

	// Lower Index of Dimension 1
	int
	l1() const
	{
		return 1;
	}

	// Upper Index of Dimension 1
	int
	u1() const
	{
		return u_;
	}

	// Size of Dimension 1
	size_type
	size1() const
	{
		return u_;
	}

	// Size of Dimension 1
	int
	isize1() const
	{
		return u_;
	}

	// Shift for Proxy
	std::ptrdiff_t
	shift() const
	{
		return 1;
	}

public:

	// Slice == Value
	friend
	bool
	eq( Array1S const & a, T const & t )
	{
		if ( a.empty() ) return true;
		for ( int i = 1, e = a.u(); i <= e; ++i ) {
			if ( ! ( a( i ) == t ) ) return false;
		}
		return true;
	}

	// Any Slice != Value
	friend
	bool
	any_ne( Array1S const & a, T const & t )
	{
		return ! eq( a, t );
	}

	// Any Slice < Value
	friend
	bool
	any_lt( Array1S const & a, T const & t )
	{
		if ( a.empty() ) return false;
		for ( int i = 1, e = a.u(); i <= e; ++i ) {
			if ( a( i ) < t ) return true;
		}
		return false;
	}

	// Any Slice > Value
	friend
	bool
	any_gt( Array1S const & a, T const & t )
	{
		return any_lt( t, a );
	}

	// Any Value < Slice
	friend
	bool
	any_lt( T const & t, Array1S const & a )
	{
		if ( a.empty() ) return false;
		for ( int i = 1, e = a.u(); i <= e; ++i ) {
			if ( t < a( i ) ) return true;
		}
		return false;
	}


private: // Methods

	// Contiguous?
	bool
	computed_contiguous() const
	{
		return m_ == 1;
	}

	// Memory Range Set
	void
	data_set()
	{
		if ( size_ > 0u ) { // Non-empty slice
			data_beg_ = data_end_ = data_ + k_;
			data_beg_ += m_ * ( m_ >= 0 ? 1 : u_ );
			data_end_ += m_ * ( m_ <= 0 ? 1 : u_ );
		} else {
			data_ = data_beg_ = data_end_ = nullptr;
		}
	}

private: // Data

	std::int64_t const m_; // Multiplier
	std::int64_t const k_; // Constant
	int const u_; // Upper index

}; // Array1S

// Conformable?
template< typename U, typename V >
inline
bool
conformable( Array1S< U > const & a, Array1S< V > const & b )
{
	return a.conformable( b );
}

// Magnitude
template< typename T >
inline
T
magnitude( Array1S< T > const & a )
{
	T mag_sq( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		T const mag_i( a( i ) );
		mag_sq += mag_i * mag_i;
	}
	return std::sqrt( mag_sq );
}

// Magnitude Squared
template< typename T >
inline
T
magnitude_squared( Array1S< T > const & a )
{
	T mag_sq( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		T const mag_i( a( i ) );
		mag_sq += mag_i * mag_i;
	}
	return mag_sq;
}

// Distance
template< typename T >
inline
T
distance( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( a.size() == b.size() );
	T distance_sq( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		T const distance_i( a( i ) - b( i ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Array1S< T > const & a, Vector2< T > const & b )
{
	assert( a.size() == 2u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Vector2< T > const & a, Array1S< T > const & b )
{
	return distance( b, a );
}

// Distance
template< typename T >
inline
T
distance( Array1S< T > const & a, Vector3< T > const & b )
{
	assert( a.size() == 3u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Vector3< T > const & a, Array1S< T > const & b )
{
	return distance( b, a );
}

// Distance
template< typename T >
inline
T
distance( Array1S< T > const & a, Vector4< T > const & b )
{
	assert( a.size() == 4u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Vector4< T > const & a, Array1S< T > const & b )
{
	return distance( b, a );
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( a.size() == b.size() );
	T distance_sq( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		T const distance_i( a( i ) - b( i ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1S< T > const & a, Vector2< T > const & b )
{
	assert( a.size() == 2u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Vector2< T > const & a, Array1S< T > const & b )
{
	return distance_squared( b, a );
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1S< T > const & a, Vector3< T > const & b )
{
	assert( a.size() == 3u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Vector3< T > const & a, Array1S< T > const & b )
{
	return distance_squared( b, a );
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1S< T > const & a, Vector4< T > const & b )
{
	assert( a.size() == 4u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Vector4< T > const & a, Array1S< T > const & b )
{
	return distance_squared( b, a );
}

// Dot Product
template< typename T >
inline
T
dot( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( a.size() == b.size() );
	T result( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		result += a( i ) * b( i );
	}
	return result;
}

// Dot Product of Boolean Arrays
inline
bool
dot( Array1S< bool > const & a, Array1S< bool > const & b )
{
	assert( a.size() == b.size() );
	bool result( false );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		if ( a( i ) && b( i ) ) {
			result = true;
			break;
		}
	}
	return result;
}

// Dot Product with Vector2
template< typename T >
inline
T
dot( Array1S< T > const & a, Vector2< T > const & b )
{
	assert( a.size() == 2u );
	T result( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		result += a( i ) * b( i );
	}
	return result;
}

// Dot Product with Vector2
template< typename T >
inline
T
dot( Vector2< T > const & a, Array1S< T > const & b )
{
	return dot( b, a );
}

// Dot Product with Vector3
template< typename T >
inline
T
dot( Array1S< T > const & a, Vector3< T > const & b )
{
	assert( a.size() == 3u );
	T result( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		result += a( i ) * b( i );
	}
	return result;
}

// Dot Product with Vector3
template< typename T >
inline
T
dot( Vector3< T > const & a, Array1S< T > const & b )
{
	return dot( b, a );
}

// Dot Product with Vector4
template< typename T >
inline
T
dot( Array1S< T > const & a, Vector4< T > const & b )
{
	assert( a.size() == 4u );
	T result( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		result += a( i ) * b( i );
	}
	return result;
}

// Dot Product with Vector4
template< typename T >
inline
T
dot( Vector4< T > const & a, Array1S< T > const & b )
{
	return dot( b, a );
}

// Dot Product (Fortran Intrinsic Name)
template< typename T >
inline
T
dot_product( Array1S< T > const & a, Array1S< T > const & b )
{
	return dot( a, b );
}

// Dot Product of Boolean Arrays (Fortran Intrinsic Name)
inline
bool
dot_product( Array1S< bool > const & a, Array1S< bool > const & b )
{
	return dot( a, b );
}

// Cross Product of 2-Tuples
template< typename T >
inline
T
cross2( Vector2< T > const & a, Array1S< T > const & b )
{
	return cross2( b, a );
}

// Stream >> Array1S
template< typename T >
inline
std::istream &
operator >>( std::istream & stream, Array1S< T > & a )
{
	if ( stream && ( a.size() > 0u ) ) {
		for ( int i = 1, e = a.u(); i <= e; ++i ) {
			stream >> a( i );
			if ( ! stream ) break;
		}
	}
	return stream;
}

// Stream << Array1S
template< typename T >
inline
std::ostream &
operator <<( std::ostream & stream, Array1S< T > const & a )
{
	typedef  TypeTraits< T >  Traits;
	if ( stream && ( a.size() > 0u ) ) {
		std::ios_base::fmtflags const old_flags( stream.flags() );
		std::streamsize const old_precision( stream.precision( Traits::precision ) );
		stream << std::right << std::showpoint << std::uppercase;
		int const w( Traits::iwidth );
		for ( int i = 1, e = a.u(); i <= e; ++i ) {
			stream << std::setw( w ) << a( i ) << ' ';
			if ( ! stream ) break;
		}
		stream.precision( old_precision );
		stream.flags( old_flags );
	}
	return stream;
}

namespace fmt {

// List-Directed Format: Array1S
template< typename T >
inline
std::string
LD( Array1S< T > const & a )
{
	std::string s;
	std::size_t const n( a.size() );
	if ( n > 0u ) {
		s.reserve( n * TypeTraits< T >::width );
		for ( int i = 1, e = a.u(); i <= e; ++i ) {
			s.append( fmt::LD( a( i ) ) );
		}
	}
	return s;
}

} // fmt

} // ObjexxFCL

#endif // ObjexxFCL_Array1S_hh_INCLUDED
