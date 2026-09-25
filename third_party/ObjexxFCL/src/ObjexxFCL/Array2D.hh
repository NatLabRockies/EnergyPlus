#ifndef ObjexxFCL_Array2D_hh_INCLUDED
#define ObjexxFCL_Array2D_hh_INCLUDED

// Array2D: Row-Major 2D Array
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
#include <ObjexxFCL/Array2D.fwd.hh>
#include <ObjexxFCL/Array2.hh>

// C++ Headers
#include <functional>

namespace ObjexxFCL {

// Array2D: Row-Major 2D Array
template< typename T >
class Array2D : public Array2< T >
{

private: // Types

	typedef  Array2< T >  Super;
	typedef  internal::InitializerSentinel  InitializerSentinel;

private: // Friend

	template< typename > friend class Array2D;
	friend class Array2A< T >;

public: // Types

	typedef  typename Super::Base  Base;
	typedef  typename Super::Traits  Traits;
	typedef  typename Super::IR  IR;

	// STL Style
	typedef  typename Super::value_type  value_type;
	typedef  typename Super::reference  reference;
	typedef  typename Super::const_reference  const_reference;
	typedef  typename Super::pointer  pointer;
	typedef  typename Super::const_pointer  const_pointer;
	typedef  typename Super::iterator  iterator;
	typedef  typename Super::const_iterator  const_iterator;
	typedef  typename Super::reverse_iterator  reverse_iterator;
	typedef  typename Super::const_reverse_iterator  const_reverse_iterator;
	typedef  typename Super::size_type  size_type;
	typedef  typename Super::difference_type  difference_type;

	// C++ Style
	typedef  typename Super::Value  Value;
	typedef  typename Super::Reference  Reference;
	typedef  typename Super::ConstReference  ConstReference;
	typedef  typename Super::Pointer  Pointer;
	typedef  typename Super::ConstPointer  ConstPointer;
	typedef  typename Super::Iterator  Iterator;
	typedef  typename Super::ConstIterator  ConstIterator;
	typedef  typename Super::ReverseIterator  ReverseIterator;
	typedef  typename Super::ConstReverseIterator  ConstReverseIterator;
	typedef  typename Super::Size  Size;
	typedef  typename Super::Difference  Difference;

	typedef  std::function< void( Array2D< T > & ) >  InitializerFunction;

	using Super::conformable;
	using Super::contains;
	using Super::index;
	using Super::isize1;
	using Super::isize2;
	using Super::l1;
	using Super::l2;
	using Super::operator ();
	using Super::operator [];
	using Super::size1;
	using Super::size2;
	using Super::u1;
	using Super::u2;

protected: // Types

	using Super::assign;
	using Super::clear_move;
	using Super::initialize;
	using Super::move_if;
	using Super::resize;
	using Super::shift_set;
	using Super::shift_only_set;
	using Super::size_of;
	using Super::swap2;

	using Super::data_;
	using Super::I1_;
	using Super::I2_;
	using Super::sdata_;
	using Super::shift_;
	using Super::size_;
	using Super::z1_;
	using Super::z2_;

public: // Creation

	// Default Constructor
	Array2D()
	{}

	// Copy Constructor
	Array2D( Array2D const & a ) :
	 Super( a )
	{}

	// Move Constructor
	Array2D( Array2D && a ) noexcept :
	 Super( std::move( a ) )
	{
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array2D( Array2D< U > const & a ) :
	 Super( a )
	{}

	// Super Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array2D( Array2< U > const & a ) :
	 Super( a )
	{}

	// Slice Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array2D( Array2S< U > const & a ) :
	 Super( a )
	{
		setup_real();
		size_type l( 0u );
		for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
			for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
				initialize( l, a( i1, i2 ) );
			}
		}
	}

	// IndexRange Constructor
	Array2D( IR const & I1, IR const & I2 ) :
	 Super( I1, I2 )
	{
		setup_real();
	}

	// IndexRange + Initializer Value Constructor
	Array2D( IR const & I1, IR const & I2, T const & t ) :
	 Super( I1, I2, InitializerSentinel{} )
	{
		setup_real();
		initialize( t );
	}


	// IndexRange + Initializer Function Constructor
	Array2D( IR const & I1, IR const & I2, InitializerFunction const & fxn ) :
	 Super( I1, I2, InitializerSentinel{} )
	{
		setup_real();
		initialize( fxn );
	}


	// IndexRange + Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array2D( IR const & I1, IR const & I2, std::initializer_list< U > const l ) :
	 Super( I1, I2, l )
	{
		setup_real();
	}

	// IndexRange + Super Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array2D( IR const & I1, IR const & I2, Array2< U > const & a ) :
	 Super( I1, I2, InitializerSentinel{} )
	{
		assert( conformable( a ) );
		setup_real();
		initialize( a );
	}

	// IndexRange + Slice Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array2D( IR const & I1, IR const & I2, Array2S< U > const & a ) :
	 Super( I1, I2, InitializerSentinel{} )
	{
		assert( conformable( a ) );
		setup_real();
		size_type l( 0u );
		for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
			for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
				initialize( l, a( i1, i2 ) );
			}
		}
	}

	// Super + IndexRange Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array2D( Array2< U > const & a, IR const & I1, IR const & I2 ) :
	 Super( I1, I2, InitializerSentinel{} )
	{
		assert( conformable( a ) );
		setup_real();
		initialize( a );
	}

	// IndexRange + Base Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array2D( IR const & I1, IR const & I2, Array< U > const & a ) :
	 Super( I1, I2, InitializerSentinel{} )
	{
		assert( size_ == a.size() );
		setup_real();
		initialize( a );
	}

	// Base + IndexRange Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array2D( Array< U > const & a, IR const & I1, IR const & I2 ) :
	 Super( I1, I2, InitializerSentinel{} )
	{
		assert( size_ == a.size() );
		setup_real();
		initialize( a );
	}

	// Array Range Named Constructor Template
	template< typename U >
	static
	Array2D
	range( Array2< U > const & a )
	{
		return Array2D( a.I1_, a.I2_ );
	}

	// Array Range + Initializer Value Named Constructor Template
	template< typename U >
	static
	Array2D
	range( Array2< U > const & a, T const & t )
	{
		return Array2D( a.I1_, a.I2_, t );
	}


	// One-Based Copy Named Constructor Template
	template< typename U >
	static
	Array2D
	one_based( Array2< U > const & a )
	{
		return Array2D( a, a.isize1(), a.isize2() );
	}

	// One-Based Slice Named Constructor Template
	template< typename U >
	static
	Array2D
	one_based( Array2S< U > const & a )
	{
		return Array2D( a.isize1(), a.isize2(), a );
	}

	// Diagonal Matrix Named Constructor
	static
	Array2D
	diag( IR const & I, T const & d )
	{
		Array2D D( I, I );
		D.to_diag( d );
		return D;
	}

	// Identity Matrix Named Constructor
	static
	Array2D
	identity( IR const & I )
	{
		Array2D D( I, I );
		D.to_diag( T( 1 ) );
		return D;
	}

	// Destructor
	virtual
	~Array2D() = default;

private: // Creation

	// IndexRange Raw Constructor
	explicit
	Array2D( IR const & I1, IR const & I2, InitializerSentinel initialized ) :
	 Super( I1, I2, initialized )
	{
		setup_real();
	}

public: // Assignment: Array

	// Copy Assignment
	Array2D &
	operator =( Array2D const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! size_real( a.I1_, a.I2_ ) ) ) {
				Base::operator =( a );
			} else {
				Base::initialize( a );
			}
		}
		return *this;
	}

	// Move Assignment
	Array2D &
	operator =( Array2D && a ) noexcept
	{
		if ( conformable( a ) ) {
			Base::conformable_move( a );
		} else {
			Base::operator =( std::move( a ) );
			I1_ = a.I1_;
			I2_ = a.I2_;
			z1_ = a.z1_;
			z2_ = a.z2_;
		}
		a.clear_move();
		return *this;
	}

	// Super Assignment
	Array2D &
	operator =( Super const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! size_real( a.I1_, a.I2_ ) ) ) {
				Base::operator =( a );
			} else {
				Base::initialize( a );
			}
		}
		return *this;
	}

	// Super Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array2D &
	operator =( Array2< U > const & a )
	{
		if ( ( conformable( a ) ) || ( ! size_real( a.I1_, a.I2_ ) ) ) {
			Base::operator =( a );
		} else {
			Base::initialize( a );
		}
		return *this;
	}

	// Slice Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array2D &
	operator =( Array2S< U > const & a )
	{
		Super::operator =( a );
		return *this;
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array2D &
	operator =( std::initializer_list< U > const l )
	{
		Base::operator =( l );
		return *this;
	}

public: // Assignment: Value

	// = Value
	Array2D &
	operator =( T const & t )
	{
		Base::operator =( t );
		return *this;
	}

public: // Modifier

	// Clear
	Array2D &
	clear()
	{
		Super::clear();
		return *this;
	}

	// Dimension by IndexRange
	Array2D &
	allocate( IR const & I1, IR const & I2 )
	{
		dimension_real( I1, I2 );
		return *this;
	}

	// Dimension by Array Template
	template< typename U >
	Array2D &
	allocate( Array2< U > const & a )
	{
		dimension_real( a.I1_, a.I2_ );
		return *this;
	}

	// Deallocate
	Array2D &
	deallocate()
	{
		Super::clear();
		return *this;
	}

	// Dimension by IndexRange
	Array2D &
	dimension( IR const & I1, IR const & I2 )
	{
		dimension_real( I1, I2 );
		return *this;
	}

	// Dimension by IndexRange + Initializer Value
	Array2D &
	dimension( IR const & I1, IR const & I2, T const & t )
	{
		dimension_real( I1, I2, t );
		return *this;
	}

	// Dimension by IndexRange + Initializer Function
	Array2D &
	dimension( IR const & I1, IR const & I2, InitializerFunction const & fxn )
	{
		dimension_real( I1, I2, fxn );
		return *this;
	}

	// Dimension by Array Template
	template< typename U >
	Array2D &
	dimension( Array2< U > const & a )
	{
		dimension_real( a.I1_, a.I2_ );
		return *this;
	}

	// Dimension by Array + Initializer Value Template
	template< typename U >
	Array2D &
	dimension( Array2< U > const & a, T const & t )
	{
		dimension_real( a.I1_, a.I2_, t );
		return *this;
	}

	// Dimension by Array + Initializer Function Template
	template< typename U >
	Array2D &
	dimension( Array2< U > const & a, InitializerFunction const & fxn )
	{
		dimension_real( a.I1_, a.I2_, fxn );
		return *this;
	}


	// Swap
	Array2D &
	swap( Array2D & v )
	{
		using std::swap;
		swap2( v );
		return *this;
	}

protected: // Functions

	// Dimension by IndexRange
	bool
	dimension_assign( IR const & I1, IR const & I2 )
	{
		return size_real( I1, I2 );
	}

	// Initialize to Default State
	void
	initialize()
	{
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
		std::uninitialized_fill_n( data_, size_, Traits::initial_array_value() );
#else
		for ( size_type i = 0; i < size_; ++i ) {
				new ( data_ + i ) T;
			}
#endif
	}

	// Initialize by Function
	void
	initialize( InitializerFunction const & fxn )
	{
		initialize();
		fxn( *this );
	}

	// Assignment to Default State
	void
	assign()
	{
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
		std::fill_n( data_, size_, Traits::initial_array_value() );
#endif
	}

private: // Functions

	// Set Up for IndexRange Constructor
	void
	setup_real()
	{
		shift_set( ( I1_.l() * z2_ ) + I2_.l() );
	}

	// Size by IndexRange
	bool
	size_real( IR const & I1, IR const & I2 )
	{
		I1_.assign( I1 );
		I2_.assign( I2 );
		z1_ = I1_.size();
		z2_ = I2_.size();
		shift_only_set( ( I1_.l() * z2_ ) + I2_.l() );
		return resize( size_of( z1_, z2_ ) );
	}

	// Dimension by IndexRange
	void
	dimension_real( IR const & I1, IR const & I2 )
	{
		if ( size_real( I1, I2 ) ) {
			initialize();
		} else {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
			assign();
#endif
		}
	}

	// Dimension by IndexRange + Initializer Value
	void
	dimension_real( IR const & I1, IR const & I2, T const & t )
	{
		if ( size_real( I1, I2 ) ) {
			initialize( t );
		} else {
			assign( t );
		}
	}

	// Dimension by IndexRange + Initializer Function
	void
	dimension_real( IR const & I1, IR const & I2, InitializerFunction const & fxn )
	{
		if ( size_real( I1, I2 ) ) initialize();
		fxn( *this );
	}

}; // Array2D

// Swap
template< typename T >
inline
void
swap( Array2D< T > & a, Array2D< T > & b )
{
	a.swap( b );
}

// Comparison: Elemental

// Value == Array
template< typename T >
inline
Array2D< bool >
operator ==( T const & t, Array2< T > const & b )
{
	return ( b == t );
}

// Value != Array
template< typename T >
inline
Array2D< bool >
operator !=( T const & t, Array2< T > const & b )
{
	return ( b != t );
}

// Value < Array
template< typename T >
inline
Array2D< bool >
operator <( T const & t, Array2< T > const & b )
{
	return ( b > t );
}

// Value <= Array
template< typename T >
inline
Array2D< bool >
operator <=( T const & t, Array2< T > const & b )
{
	return ( b >= t );
}

// Value > Array
template< typename T >
inline
Array2D< bool >
operator >( T const & t, Array2< T > const & b )
{
	return ( b < t );
}

// Value >= Array
template< typename T >
inline
Array2D< bool >
operator >=( T const & t, Array2< T > const & b )
{
	return ( b <= t );
}

// Comparison: Elemental: Slice

// Array == Slice
template< typename T >
inline
Array2D< bool >
operator ==( Array2< T > const & a, Array2S< T > const & b )
{
	return ( b == a );
}

// Array != Slice
template< typename T >
inline
Array2D< bool >
operator !=( Array2< T > const & a, Array2S< T > const & b )
{
	return ( b != a );
}

// Array < Slice
template< typename T >
inline
Array2D< bool >
operator <( Array2< T > const & a, Array2S< T > const & b )
{
	return ( b > a );
}

// Array <= Slice
template< typename T >
inline
Array2D< bool >
operator <=( Array2< T > const & a, Array2S< T > const & b )
{
	return ( b >= a );
}

// Array > Slice
template< typename T >
inline
Array2D< bool >
operator >( Array2< T > const & a, Array2S< T > const & b )
{
	return ( b < a );
}

// Array >= Slice
template< typename T >
inline
Array2D< bool >
operator >=( Array2< T > const & a, Array2S< T > const & b )
{
	return ( b <= a );
}

// Value == Slice
template< typename T >
inline
Array2D< bool >
operator ==( T const & t, Array2S< T > const & b )
{
	return ( b == t );
}

// Value != Slice
template< typename T >
inline
Array2D< bool >
operator !=( T const & t, Array2S< T > const & b )
{
	return ( b != t );
}

// Value < Slice
template< typename T >
inline
Array2D< bool >
operator <( T const & t, Array2S< T > const & b )
{
	return ( b > t );
}

// Value <= Slice
template< typename T >
inline
Array2D< bool >
operator <=( T const & t, Array2S< T > const & b )
{
	return ( b >= t );
}

// Value > Slice
template< typename T >
inline
Array2D< bool >
operator >( T const & t, Array2S< T > const & b )
{
	return ( b < t );
}

// Value >= Slice
template< typename T >
inline
Array2D< bool >
operator >=( T const & t, Array2S< T > const & b )
{
	return ( b <= t );
}

// Generator

// -Array
template< typename T >
inline
Array2D< T >
operator -( Array2< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= T( -1 );
	return r;
}

// Array + Array
template< typename T >
inline
Array2D< T >
operator +( Array2< T > const & a, Array2< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r += b;
	return r;
}

// Array - Array
template< typename T >
inline
Array2D< T >
operator -( Array2< T > const & a, Array2< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r -= b;
	return r;
}

// Array * Array
template< typename T >
inline
Array2D< T >
operator *( Array2< T > const & a, Array2< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= b;
	return r;
}

// Array / Array
template< typename T >
inline
Array2D< T >
operator /( Array2< T > const & a, Array2< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r /= b;
	return r;
}

// Array + Value
template< typename T >
inline
Array2D< T >
operator +( Array2< T > const & a, T const & t )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r += t;
	return r;
}

// Value + Array
template< typename T >
inline
Array2D< T >
operator +( T const & t, Array2< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r += t;
	return r;
}

// Array - Value
template< typename T >
inline
Array2D< T >
operator -( Array2< T > const & a, T const & t )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r -= t;
	return r;
}

// Value - Array
template< typename T >
inline
Array2D< T >
operator -( T const & t, Array2< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= T( -1 );
	r += t;
	return r;
}

// Array * Value
template< typename T >
inline
Array2D< T >
operator *( Array2< T > const & a, T const & t )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= t;
	return r;
}

// Value * Array
template< typename T >
inline
Array2D< T >
operator *( T const & t, Array2< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= t;
	return r;
}

// Array / Value
template< typename T >
inline
Array2D< T >
operator /( Array2< T > const & a, T const & t )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r /= t;
	return r;
}

// Value / Array
template< typename T >
inline
Array2D< T >
operator /( T const & t, Array2< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r.invert();
	r *= t;
	return r;
}

// Array Transpose: Fortran-Compatible 1-Based Indexing
template< typename T >
inline
Array2D< T >
transpose( Array2< T > const & a )
{
	typedef  BArray::size_type  size_type;
	size_type const as1( a.size1() );
	size_type const as2( a.size2() );
	Array2D< T > aT( static_cast< int >( as2 ), static_cast< int >( as1 ) );
	for ( size_type i1 = 0, l = 0; i1 < as1; ++i1 ) {
		for ( size_type i2 = 0, lT = i1; i2 < as2; ++i2, ++l, lT += as1 ) {
			aT[ lT ] = a[ l ];
		}
	}
	return aT;
}

// Array Transposed: Preserved Indexing
template< typename T >
inline
Array2D< T >
transposed( Array2< T > const & a )
{
	typedef  BArray::size_type  size_type;
	size_type const as1( a.size1() );
	size_type const as2( a.size2() );
	Array2D< T > aT( a.I2(), a.I1() );
	for ( size_type i1 = 0, l = 0; i1 < as1; ++i1 ) {
		for ( size_type i2 = 0, lT = i1; i2 < as2; ++i2, ++l, lT += as1 ) {
			aT[ lT ] = a[ l ];
		}
	}
	return aT;
}

// Generator: Slice

// -Slice
template< typename T >
inline
Array2D< T >
operator -( Array2S< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= T( -1 );
	return r;
}

// Slice + Slice
template< typename T >
inline
Array2D< T >
operator +( Array2S< T > const & a, Array2S< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r += b;
	return r;
}

// Slice - Slice
template< typename T >
inline
Array2D< T >
operator -( Array2S< T > const & a, Array2S< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r -= b;
	return r;
}

// Slice * Slice
template< typename T >
inline
Array2D< T >
operator *( Array2S< T > const & a, Array2S< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= b;
	return r;
}

// Slice / Slice
template< typename T >
inline
Array2D< T >
operator /( Array2S< T > const & a, Array2S< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r /= b;
	return r;
}

// Slice + Array
template< typename T >
inline
Array2D< T >
operator +( Array2S< T > const & a, Array2< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r += b;
	return r;
}

// Slice - Array
template< typename T >
inline
Array2D< T >
operator -( Array2S< T > const & a, Array2< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r -= b;
	return r;
}

// Slice * Array
template< typename T >
inline
Array2D< T >
operator *( Array2S< T > const & a, Array2< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= b;
	return r;
}

// Slice / Array
template< typename T >
inline
Array2D< T >
operator /( Array2S< T > const & a, Array2< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r /= b;
	return r;
}

// Array + Slice
template< typename T >
inline
Array2D< T >
operator +( Array2< T > const & a, Array2S< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r += b;
	return r;
}

// Array - Slice
template< typename T >
inline
Array2D< T >
operator -( Array2< T > const & a, Array2S< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r -= b;
	return r;
}

// Array * Slice
template< typename T >
inline
Array2D< T >
operator *( Array2< T > const & a, Array2S< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= b;
	return r;
}

// Array / Slice
template< typename T >
inline
Array2D< T >
operator /( Array2< T > const & a, Array2S< T > const & b )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r /= b;
	return r;
}

// Slice + Value
template< typename T >
inline
Array2D< T >
operator +( Array2S< T > const & a, T const & t )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r += t;
	return r;
}

// Value + Slice
template< typename T >
inline
Array2D< T >
operator +( T const & t, Array2S< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r += t;
	return r;
}

// Slice - Value
template< typename T >
inline
Array2D< T >
operator -( Array2S< T > const & a, T const & t )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r -= t;
	return r;
}

// Value - Slice
template< typename T >
inline
Array2D< T >
operator -( T const & t, Array2S< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= T( -1 );
	r += t;
	return r;
}

// Slice * Value
template< typename T >
inline
Array2D< T >
operator *( Array2S< T > const & a, T const & t )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= t;
	return r;
}

// Value * Slice
template< typename T >
inline
Array2D< T >
operator *( T const & t, Array2S< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r *= t;
	return r;
}

// Slice / Value
template< typename T >
inline
Array2D< T >
operator /( Array2S< T > const & a, T const & t )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r /= t;
	return r;
}

// Value / Slice
template< typename T >
inline
Array2D< T >
operator /( T const & t, Array2S< T > const & a )
{
	Array2D< T > r( Array2D< T >::one_based( a ) );
	r.invert();
	r *= t;
	return r;
}

// Slice Transpose: Fortran-Compatible 1-Based Indexing
template< typename T >
inline
Array2D< T >
transpose( Array2S< T > const & a )
{
	typedef  BArray::size_type  size_type;
	int const as1( a.isize1() );
	int const as2( a.isize2() );
	Array2D< T > aT( as2, as1 );
	size_type l( 0u );
	for ( int i1 = 1; i1 <= as2; ++i1 ) {
		for ( int i2 = 1; i2 <= as1; ++i2, ++l ) {
			aT[ l ] = a( i2, i1 );
		}
	}
	return aT;
}

// Slice Transposed: Preserved Indexing
template< typename T >
inline
Array2D< T >
transposed( Array2S< T > const & a )
{
	return transpose( a ); // Slice indexing is 1-based
}

} // ObjexxFCL

#endif // ObjexxFCL_Array2D_hh_INCLUDED
