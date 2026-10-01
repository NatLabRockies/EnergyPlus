#ifndef ObjexxFCL_Array1A_hh_INCLUDED
#define ObjexxFCL_Array1A_hh_INCLUDED

// Array1A: 1D Argument Array
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
#include <ObjexxFCL/Array1A.fwd.hh>
#include <ObjexxFCL/Array1.hh>

namespace ObjexxFCL {

// Array1A: 1D Argument Array
template< typename T >
class Array1A : public Array1< T >
{

private: // Types

	typedef  Array1< T >  Super;
	typedef  internal::ProxySentinel  ProxySentinel;

public: // Types

	typedef  typename Super::Base  Base;
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

	using Super::conformable;
	using Super::npos;
	using Super::operator ();

protected: // Types

	using Super::shift_set;
	using Super::size_set;

	using Super::data_;
	using Super::I_;
	using Super::sdata_;
	using Super::shift_;
	using Super::size_;

public: // Creation

	// Default Constructor
	Array1A() :
	 Super( ProxySentinel{} )
	{}

	// Copy Constructor
	Array1A( Array1A const & a ) :
	 Super( a, ProxySentinel{} )
	{
		shift_set( a.shift_ );
	}

	// Super Constructor
	Array1A( Super const & a ) :
	 Super( a, ProxySentinel{} )
	{
		shift_set( a.shift_ );
	}

	// Slice Constructor
	Array1A( Array1S< T > const & a ) :
	 Super( a, ProxySentinel{} )
	{
		shift_set( a.shift() );
	}

	// Base Constructor
	Array1A( Base const & a ) :
	 Super( a, ProxySentinel{} )
	{
		shift_set( 1 );
	}

	// Value Constructor
	Array1A( T const & t ) :
	 Super( t, ProxySentinel{} )
	{
		shift_set( 1 );
	}

	// Copy + IndexRange Constructor
	Array1A( Array1A const & a, IR const & I ) :
	 Super( a, I, ProxySentinel{} )
	{
		dimension_argument();
	}

	// Super + IndexRange Constructor
	Array1A( Super const & a, IR const & I ) :
	 Super( a, I, ProxySentinel{} )
	{
		dimension_argument();
	}

	// Slice + IndexRange Constructor
	Array1A( Array1S< T > const & a, IR const & I ) :
	 Super( a, I, ProxySentinel{} )
	{
		dimension_argument();
	}

	// Base + IndexRange Constructor
	Array1A( Base const & a, IR const & I ) :
	 Super( a, I, ProxySentinel{} )
	{
		dimension_argument();
	}

	// Value + IndexRange Constructor
	Array1A( T const & t, IR const & I ) :
	 Super( t, I, ProxySentinel{} )
	{
		dimension_argument();
	}

	// Destructor
	virtual
	~Array1A() = default;

public: // Assignment: Array

	// Copy Assignment
	Array1A &
	operator =( Array1A const & a )
	{
		if ( this != &a ) {
			if ( ! conformable( a ) ) dimension( a );
			Base::operator =( a );
		}
		return *this;
	}

	// Super Assignment
	Array1A &
	operator =( Super const & a )
	{
		if ( this != &a ) {
			if ( ! conformable( a ) ) dimension( a );
			Base::operator =( a );
		}
		return *this;
	}

	// Super Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1A &
	operator =( Array1< U > const & a )
	{
		if ( ! conformable( a ) ) dimension( a );
		Base::operator =( a );
		return *this;
	}

	// Slice Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1A &
	operator =( Array1S< U > const & a )
	{
		Super::operator =( a );
		return *this;
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1A &
	operator =( std::initializer_list< U > const l )
	{
		Base::operator =( l );
		return *this;
	}

	// std::array Assignment Template
	template< typename U, Size s, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1A &
	operator =( std::array< U, s > const & a )
	{
		Base::operator =( a );
		return *this;
	}

	// std::vector Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1A &
	operator =( std::vector< U > const & v )
	{
		Base::operator =( v );
		return *this;
	}

	// Vector2 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1A &
	operator =( Vector2< U > const & v )
	{
		Base::operator =( v );
		return *this;
	}

	// Vector3 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1A &
	operator =( Vector3< U > const & v )
	{
		Base::operator =( v );
		return *this;
	}

	// Vector4 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1A &
	operator =( Vector4< U > const & v )
	{
		Base::operator =( v );
		return *this;
	}

public: // Assignment: Value

	// = Value
	Array1A &
	operator =( T const & t )
	{
		Base::operator =( t );
		return *this;
	}

public: // Predicate

	// Initializer Active?
	bool
	initializer_active() const
	{
		return false;
	}

public: // Modifier

	// Clear
	Array1A &
	clear()
	{
		Super::clear();
		return *this;
	}

	// Dimension by IndexRange Even if Const
	Array1A const &
	dim( IR const & I ) const
	{
		const_cast< Array1A & >( *this ).dimension( I );
		return *this;
	}

	// Dimension by Array Even if Const
	template< typename U >
	Array1A const &
	dim( Array1< U > const & a ) const
	{
		const_cast< Array1A & >( *this ).dimension( a );
		return *this;
	}

	// Dimension by IndexRange
	Array1A &
	dimension( IR const & I )
	{
		I_.assign( I );
		dimension_argument();
		return *this;
	}

	// Dimension by Array
	template< typename U >
	Array1A &
	dimension( Array1< U > const & a )
	{
		I_.assign( a.I() );
		dimension_argument();
		return *this;
	}

protected: // Functions

	// Dimension by IndexRange
	bool
	dimension_assign( IR const & I )
	{
		I_.assign( I );
		dimension_argument();
		return false;
	}

private: // Functions

	// Dimension by Current IndexRange
	void
	dimension_argument()
	{
		if ( I_.bounded() ) {
			size_set( I_.size() );
		} else if ( size_ == npos ) {
			size_set( npos );
		} else { // Infer size
			I_.u( I_.l() + static_cast< int >( size_ ) - 1 );
			size_set( I_.size() );
		}
		shift_set( I_.l() );
	}

}; // Array1A

} // ObjexxFCL

#endif // ObjexxFCL_Array1A_hh_INCLUDED
