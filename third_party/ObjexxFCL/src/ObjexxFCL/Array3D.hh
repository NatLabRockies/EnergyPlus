#ifndef ObjexxFCL_Array3D_hh_INCLUDED
#define ObjexxFCL_Array3D_hh_INCLUDED

// Array3D: Row-Major 3D Array
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
#include <ObjexxFCL/Array3D.fwd.hh>
#include <ObjexxFCL/Array3.hh>

// C++ Headers
#include <functional>

namespace ObjexxFCL {

// Array3D: Row-Major 3D Array
template< typename T >
class Array3D : public Array3< T >
{

private: // Types

	typedef  Array3< T >  Super;
	typedef  internal::InitializerSentinel  InitializerSentinel;

private: // Friend

	template< typename > friend class Array3D;

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

	typedef  std::function< void( Array3D< T > & ) >  InitializerFunction;

	using Super::conformable;
	using Super::contains;
	using Super::index;
	using Super::isize1;
	using Super::isize2;
	using Super::isize3;
	using Super::l1;
	using Super::l2;
	using Super::l3;
	using Super::operator ();
	using Super::operator [];
	using Super::size1;
	using Super::size2;
	using Super::size3;
	using Super::u1;
	using Super::u2;
	using Super::u3;

protected: // Types

	using Super::assign;
	using Super::clear_move;
	using Super::initialize;
	using Super::move_if;
	using Super::resize;
	using Super::shift_set;
	using Super::shift_only_set;
	using Super::size_of;
	using Super::swap3;

	using Super::data_;
	using Super::I1_;
	using Super::I2_;
	using Super::I3_;
	using Super::sdata_;
	using Super::shift_;
	using Super::size_;
	using Super::z1_;
	using Super::z2_;
	using Super::z3_;

public: // Creation

	// Default Constructor
	Array3D()
	{}

	// Copy Constructor
	Array3D( Array3D const & a ) :
	 Super( a )
	{}

	// Move Constructor
	Array3D( Array3D && a ) noexcept :
	 Super( std::move( a ) )
	{
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array3D( Array3D< U > const & a ) :
	 Super( a )
	{}

	// Super Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array3D( Array3< U > const & a ) :
	 Super( a )
	{}


	// IndexRange Constructor
	Array3D( IR const & I1, IR const & I2, IR const & I3 ) :
	 Super( I1, I2, I3 )
	{
		setup_real();
	}

	// IndexRange + Initializer Value Constructor
	Array3D( IR const & I1, IR const & I2, IR const & I3, T const & t ) :
	 Super( I1, I2, I3, InitializerSentinel{} )
	{
		setup_real();
		initialize( t );
	}

	// IndexRange + Initializer Function Constructor
	Array3D( IR const & I1, IR const & I2, IR const & I3, InitializerFunction const & fxn ) :
	 Super( I1, I2, I3, InitializerSentinel{} )
	{
		setup_real();
		initialize( fxn );
	}

	// IndexRange + Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array3D( IR const & I1, IR const & I2, IR const & I3, std::initializer_list< U > const l ) :
	 Super( I1, I2, I3, l )
	{
		setup_real();
	}

	// IndexRange + Super Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array3D( IR const & I1, IR const & I2, IR const & I3, Array3< U > const & a ) :
	 Super( I1, I2, I3, InitializerSentinel{} )
	{
		assert( conformable( a ) );
		setup_real();
		initialize( a );
	}


	// Super + IndexRange Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array3D( Array3< U > const & a, IR const & I1, IR const & I2, IR const & I3 ) :
	 Super( I1, I2, I3, InitializerSentinel{} )
	{
		assert( conformable( a ) );
		setup_real();
		initialize( a );
	}



	// Destructor
	virtual
	~Array3D() = default;

private: // Creation

	// IndexRange Raw Constructor
	explicit
	Array3D( IR const & I1, IR const & I2, IR const & I3, InitializerSentinel initialized ) :
	 Super( I1, I2, I3, initialized )
	{
		setup_real();
	}

public: // Assignment: Array

	// Copy Assignment
	Array3D &
	operator =( Array3D const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! size_real( a.I1_, a.I2_, a.I3_ ) ) ) {
				Base::operator =( a );
			} else {
				Base::initialize( a );
			}
		}
		return *this;
	}

	// Move Assignment
	Array3D &
	operator =( Array3D && a ) noexcept
	{
		if ( conformable( a ) ) {
			Base::conformable_move( a );
		} else {
			Base::operator =( std::move( a ) );
			I1_ = a.I1_;
			I2_ = a.I2_;
			I3_ = a.I3_;
			z1_ = a.z1_;
			z2_ = a.z2_;
			z3_ = a.z3_;
		}
		a.clear_move();
		return *this;
	}

	// Super Assignment
	Array3D &
	operator =( Super const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! size_real( a.I1_, a.I2_, a.I3_ ) ) ) {
				Base::operator =( a );
			} else {
				Base::initialize( a );
			}
		}
		return *this;
	}

	// Super Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array3D &
	operator =( Array3< U > const & a )
	{
		if ( ( conformable( a ) ) || ( ! size_real( a.I1_, a.I2_, a.I3_ ) ) ) {
			Base::operator =( a );
		} else {
			Base::initialize( a );
		}
		return *this;
	}


	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array3D &
	operator =( std::initializer_list< U > const l )
	{
		Base::operator =( l );
		return *this;
	}


public: // Assignment: Value

	// = Value
	Array3D &
	operator =( T const & t )
	{
		Base::operator =( t );
		return *this;
	}

public: // Modifier

	// Clear
	Array3D &
	clear()
	{
		Super::clear();
		return *this;
	}

	// Dimension by IndexRange
	Array3D &
	allocate( IR const & I1, IR const & I2, IR const & I3 )
	{
		dimension_real( I1, I2, I3 );
		return *this;
	}

	// Dimension by Array Template
	template< typename U >
	Array3D &
	allocate( Array3< U > const & a )
	{
		dimension_real( a.I1_, a.I2_, a.I3_ );
		return *this;
	}

	// Deallocate
	Array3D &
	deallocate()
	{
		Super::clear();
		return *this;
	}

	// Dimension by IndexRange
	Array3D &
	dimension( IR const & I1, IR const & I2, IR const & I3 )
	{
		dimension_real( I1, I2, I3 );
		return *this;
	}

	// Dimension by IndexRange + Initializer Value
	Array3D &
	dimension( IR const & I1, IR const & I2, IR const & I3, T const & t )
	{
		dimension_real( I1, I2, I3, t );
		return *this;
	}

	// Dimension by IndexRange + Initializer Function
	Array3D &
	dimension( IR const & I1, IR const & I2, IR const & I3, InitializerFunction const & fxn )
	{
		dimension_real( I1, I2, I3, fxn );
		return *this;
	}

	// Dimension by Array Template
	template< typename U >
	Array3D &
	dimension( Array3< U > const & a )
	{
		dimension_real( a.I1_, a.I2_, a.I3_ );
		return *this;
	}

	// Dimension by Array + Initializer Value Template
	template< typename U >
	Array3D &
	dimension( Array3< U > const & a, T const & t )
	{
		dimension_real( a.I1_, a.I2_, a.I3_, t );
		return *this;
	}

	// Dimension by Array + Initializer Function Template
	template< typename U >
	Array3D &
	dimension( Array3< U > const & a, InitializerFunction const & fxn )
	{
		dimension_real( a.I1_, a.I2_, a.I3_, fxn );
		return *this;
	}

	// Swap
	Array3D &
	swap( Array3D & v )
	{
		using std::swap;
		swap3( v );
		return *this;
	}

protected: // Functions

	// Dimension by IndexRange
	bool
	dimension_assign( IR const & I1, IR const & I2, IR const & I3 )
	{
		return size_real( I1, I2, I3 );
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
		shift_set( ( ( ( I1_.l() * z2_ ) + I2_.l() ) * z3_ ) + I3_.l() );
	}

	// Size by IndexRange
	bool
	size_real( IR const & I1, IR const & I2, IR const & I3 )
	{
		I1_.assign( I1 );
		I2_.assign( I2 );
		I3_.assign( I3 );
		z1_ = I1_.size();
		z2_ = I2_.size();
		z3_ = I3_.size();
		shift_only_set( ( ( ( I1_.l() * z2_ ) + I2_.l() ) * z3_ ) + I3_.l() );
		return resize( size_of( z1_, z2_, z3_ ) );
	}

	// Dimension by IndexRange
	void
	dimension_real( IR const & I1, IR const & I2, IR const & I3 )
	{
		if ( size_real( I1, I2, I3 ) ) {
			initialize();
		} else {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
			assign();
#endif
		}
	}

	// Dimension by IndexRange + Initializer Value
	void
	dimension_real( IR const & I1, IR const & I2, IR const & I3, T const & t )
	{
		if ( size_real( I1, I2, I3 ) ) {
			initialize( t );
		} else {
			assign( t );
		}
	}

	// Dimension by IndexRange + Initializer Function
	void
	dimension_real( IR const & I1, IR const & I2, IR const & I3, InitializerFunction const & fxn )
	{
		if ( size_real( I1, I2, I3 ) ) initialize();
		fxn( *this );
	}

}; // Array3D

// Swap
template< typename T >
inline
void
swap( Array3D< T > & a, Array3D< T > & b )
{
	a.swap( b );
}



} // ObjexxFCL

#endif // ObjexxFCL_Array3D_hh_INCLUDED
