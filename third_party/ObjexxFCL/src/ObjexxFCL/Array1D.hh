#ifndef ObjexxFCL_Array1D_hh_INCLUDED
#define ObjexxFCL_Array1D_hh_INCLUDED

// Array1D: 1D Array
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
#include <ObjexxFCL/Array1D.fwd.hh>
#include <ObjexxFCL/Array1.hh>

// C++ Headers
#include <functional>

namespace ObjexxFCL {

// Array1D: 1D Array
template< typename T >
class Array1D : public Array1< T >
{

private: // Types

	typedef  Array1< T >  Super;
	typedef  internal::InitializerSentinel  InitializerSentinel;

private: // Friend

	template< typename > friend class Array1D;
	friend class Array1A< T >;

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

	typedef  std::function< void( Array1D< T > & ) >  InitializerFunction;

	using Super::conformable;
	using Super::contains;
	using Super::index;
	using Super::isize1;
	using Super::l;
	using Super::operator ();
	using Super::operator [];
	using Super::size1;
	using Super::u;

protected: // Types

	using Super::assign;
	using Super::clear_move;
	using Super::initialize;
	using Super::move_if;
	using Super::move_or_copy;
	using Super::move_or_copy_backward;
	using Super::resize;
	using Super::shift_set;
	using Super::shift_only_set;
	using Super::size_of;
	using Super::swap1;

	using Super::capacity_;
	using Super::data_;
	using Super::I_;
	using Super::sdata_;
	using Super::shift_;
	using Super::size_;

public: // Creation

	// Default Constructor
	Array1D()
	{
		shift_ = 1; // For std::vector-like API
	}

	// Copy Constructor
	Array1D( Array1D const & a ) :
	 Super( a )
	{}

	// Move Constructor
	Array1D( Array1D && a ) noexcept :
	 Super( std::move( a ) )
	{
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array1D( Array1D< U > const & a ) :
	 Super( a )
	{}

	// Super Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array1D( Array1< U > const & a ) :
	 Super( a )
	{}

	// Slice Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array1D( Array1S< U > const & a ) :
	 Super( a )
	{
		setup_real();
		size_type l( 0u );
		for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
			initialize( l, a( i ) );
		}
	}

	// MArray Constructor Template
	template< class A, typename M >
	explicit
	Array1D( MArray1< A, M > const & a ) :
	 Super( a )
	{
		setup_real();
		size_type l( 0u );
		for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
			initialize( l, a( i ) );
		}
	}

	// IndexRange Constructor
	explicit
	Array1D( IR const & I ) :
	 Super( I )
	{
		setup_real();
	}

	// IndexRange + Initializer Value Constructor
	Array1D( IR const & I, T const & t ) :
	 Super( I, InitializerSentinel{} )
	{
		setup_real();
		initialize( t );
	}

	// IndexRange + Initializer Function Constructor
	Array1D( IR const & I, InitializerFunction const & fxn ) :
	 Super( I, InitializerSentinel{} )
	{
		setup_real();
		initialize( fxn );
	}

	// IndexRange + Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( IR const & I, std::initializer_list< U > const l ) :
	 Super( I, l )
	{
		setup_real();
	}

	// IndexRange + Super Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( IR const & I, Array1< U > const & a ) :
	 Super( I, InitializerSentinel{} )
	{
		assert( conformable( a ) );
		setup_real();
		initialize( a );
	}


	// IndexRange + Slice Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( IR const & I, Array1S< U > const & a ) :
	 Super( I, InitializerSentinel{} )
	{
		assert( conformable( a ) );
		setup_real();
		size_type l( 0u );
		for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
			initialize( l, a( i ) );
		}
	}

	// Super + IndexRange Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( Array1< U > const & a, IR const & I ) :
	 Super( I, InitializerSentinel{} )
	{
		assert( conformable( a ) );
		setup_real();
		initialize( a );
	}

	// IndexRange + Base Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( IR const & I, Array< U > const & a ) :
	 Super( I, InitializerSentinel{} )
	{
		assert( size_ == a.size() );
		setup_real();
		initialize( a );
	}

	// Base + IndexRange Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( Array< U > const & a, IR const & I ) :
	 Super( I, InitializerSentinel{} )
	{
		assert( size_ == a.size() );
		setup_real();
		initialize( a );
	}

	// Initializer List Index Range Constructor Template
	template< typename U, class = typename std::enable_if< ! std::is_constructible< T, U >::value >::type >
	explicit
	Array1D( std::initializer_list< U > const l ) :
	 Super( IR( l ) )
	{
		assert( l.size() == 2 );
		setup_real();
	}

	// Initializer List of Values Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value && ! ( std::is_same< U, int >::value || std::is_same< U, Index >::value ) >::type, typename = void >
	Array1D( std::initializer_list< U > const l ) :
	 Super( l )
	{
		setup_real();
	}

	// Initializer List of Values or Index Range Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value && ( std::is_same< U, int >::value || std::is_same< U, Index >::value ) >::type, typename = void, typename = void >
	Array1D( std::initializer_list< U > const l ) :
	 Super( l )
	{ // Note: 2 item lists of index-like elements treated as index range: Others treated as values: See ObjexxFCL.Users.Array.html
#ifdef OBJEXXFCL_DISALLOW_AMBIGUOUS_INITIALIZER_LIST_CONSTRUCTORS
		assert( l.size() != 2 ); // Avoid ambiguity with IndexRange {l,u} usage
#endif
		if ( l.size() == 2 ) { // Treat as an IndexRange
			I_ = l;
			Base::reconstruct_by_size( size_of( I_ ) );
		}
		setup_real();
	}

	// Initializer List Index Range + Initializer Value Constructor Template
	template< typename U >
	Array1D( std::initializer_list< U > const l, T const & t ) :
	 Super( IR( l ), InitializerSentinel{} )
	{
		assert( l.size() == 2 );
		setup_real();
		initialize( t );
	}

	// Initializer List Index Range + Initializer Function Constructor Template
	template< typename U >
	Array1D( std::initializer_list< U > const l, InitializerFunction const & fxn ) :
	 Super( IR( l ), InitializerSentinel{} )
	{
		assert( l.size() == 2 );
		setup_real();
		initialize( fxn );
	}

	// Initializer List Index Range + Super Constructor Template
	template< typename U, typename V, class = typename std::enable_if< std::is_constructible< T, V >::value >::type >
	Array1D( std::initializer_list< U > const l, Array1< V > const & a ) :
	 Super( IR( l ), InitializerSentinel{} )
	{
		assert( l.size() == 2 );
		assert( conformable( a ) );
		setup_real();
		initialize( a );
	}

	// Initializer List Index Range + Base Constructor Template
	template< typename U, typename V, class = typename std::enable_if< std::is_constructible< T, V >::value >::type >
	Array1D( std::initializer_list< U > const l, Array< V > const & a ) :
	 Super( IR( l ), InitializerSentinel{} )
	{
		assert( l.size() == 2 );
		assert( size_ == a.size() );
		setup_real();
		initialize( a );
	}

	// std::array Constructor Template
	template< typename U, Size s >
	Array1D( std::array< U, s > const & a ) :
	 Super( a )
	{
		setup_real();
	}

	// std::vector Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( std::vector< U > const & v ) :
	 Super( v )
	{
		setup_real();
	}

	// Vector2 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( Vector2< U > const & v ) :
	 Super( v )
	{
		setup_real();
	}

	// Vector3 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( Vector3< U > const & v ) :
	 Super( v )
	{
		setup_real();
	}

	// Vector4 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1D( Vector4< U > const & v ) :
	 Super( v )
	{
		setup_real();
	}

	// Iterator Range Constructor Template
	template< class Iterator, typename = decltype( *std::declval< Iterator & >(), void(), ++std::declval< Iterator & >(), void() ) >
	Array1D( Iterator const beg, Iterator const end ) :
	 Super( beg, end )
	{
		setup_real();
	}

	// Array Range Named Constructor Template
	template< typename U >
	static
	Array1D
	range( Array1< U > const & a )
	{
		return Array1D( a.I() );
	}

	// Array Range + Initializer Value Named Constructor Template
	template< typename U >
	static
	Array1D
	range( Array1< U > const & a, T const & t )
	{
		return Array1D( a.I(), t );
	}

	// Array Shape Named Constructor Template
	template< typename U >
	static
	Array1D
	shape( Array1< U > const & a )
	{
		return Array1D( a.isize() );
	}

	// Array Shape + Initializer Value Named Constructor Template
	template< typename U >
	static
	Array1D
	shape( Array1< U > const & a, T const & t )
	{
		return Array1D( a.isize(), t );
	}

	// Slice Shape Named Constructor Template
	template< typename U >
	static
	Array1D
	shape( Array1S< U > const & a )
	{
		return Array1D( a.isize() );
	}

	// Slice Shape + Initializer Value Named Constructor Template
	template< typename U >
	static
	Array1D
	shape( Array1S< U > const & a, T const & t )
	{
		return Array1D( a.isize(), t );
	}

	// One-Based Copy Named Constructor Template
	template< typename U >
	static
	Array1D
	one_based( Array1< U > const & a )
	{
		return Array1D( a.isize(), a );
	}

	// One-Based Slice Named Constructor Template
	template< typename U >
	static
	Array1D
	one_based( Array1S< U > const & a )
	{
		return Array1D( a.isize(), a );
	}

	// Initializer List One-Based Named Constructor Template
	template< typename U >
	static
	Array1D
	one_based( std::initializer_list< U > const l )
	{
		return Array1D( static_cast< int >( l.size() ), l );
	}

	// Destructor
	virtual
	~Array1D() = default;

private: // Creation

	// IndexRange Raw Constructor
	explicit
	Array1D( IR const & I, InitializerSentinel initialized ) :
	 Super( I, initialized )
	{
		setup_real();
	}


public: // Assignment: Array

	// Copy Assignment
	Array1D &
	operator =( Array1D const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! size_real( a.I_ ) ) ) {
				Base::operator =( a );
			} else {
				Base::initialize( a );
			}
		}
		return *this;
	}

	// Move Assignment
	Array1D &
	operator =( Array1D && a ) noexcept
	{
		if ( conformable( a ) ) {
			Base::conformable_move( a );
		} else {
			Base::operator =( std::move( a ) );
			I_ = a.I_;
		}
		a.clear_move();
		return *this;
	}

	// Super Assignment
	Array1D &
	operator =( Super const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! size_real( a.I_ ) ) ) {
				Base::operator =( a );
			} else {
				Base::initialize( a );
			}
		}
		return *this;
	}

	// Super Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1D &
	operator =( Array1< U > const & a )
	{
		if ( ( conformable( a ) ) || ( ! size_real( a.I_ ) ) ) {
			Base::operator =( a );
		} else {
			Base::initialize( a );
		}
		return *this;
	}

	// Slice Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1D &
	operator =( Array1S< U > const & a )
	{
		Super::operator =( a );
		return *this;
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1D &
	operator =( std::initializer_list< U > const l )
	{
		Base::operator =( l );
		return *this;
	}

	// std::array Assignment Template
	template< typename U, Size s, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1D &
	operator =( std::array< U, s > const & a )
	{
		Base::operator =( a );
		return *this;
	}

	// std::vector Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1D &
	operator =( std::vector< U > const & v )
	{
		Base::operator =( v );
		return *this;
	}

	// Vector2 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1D &
	operator =( Vector2< U > const & v )
	{
		Base::operator =( v );
		return *this;
	}

	// Vector3 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1D &
	operator =( Vector3< U > const & v )
	{
		Base::operator =( v );
		return *this;
	}

	// Vector4 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1D &
	operator =( Vector4< U > const & v )
	{
		Base::operator =( v );
		return *this;
	}

public: // Assignment: Value

	// = Value
	Array1D &
	operator =( T const & t )
	{
		Base::operator =( t );
		return *this;
	}

public: // Modifier

	// Clear
	Array1D &
	clear()
	{
		Super::clear();
		return *this;
	}

	// Dimension by IndexRange
	Array1D &
	allocate( IR const & I )
	{
		dimension_real( I );
		return *this;
	}

	// Dimension by Array Template
	template< typename U >
	Array1D &
	allocate( Array1< U > const & a )
	{
		dimension_real( a.I_ );
		return *this;
	}

	// Deallocate
	Array1D &
	deallocate()
	{
		Super::clear();
		return *this;
	}

	// Dimension by IndexRange
	Array1D &
	dimension( IR const & I )
	{
		dimension_real( I );
		return *this;
	}

	// Dimension by IndexRange + Initializer Value
	Array1D &
	dimension( IR const & I, T const & t )
	{
		dimension_real( I, t );
		return *this;
	}

	// Dimension by IndexRange + Initializer Function
	Array1D &
	dimension( IR const & I, InitializerFunction const & fxn )
	{
		dimension_real( I, fxn );
		return *this;
	}

	// Dimension by Array Template
	template< typename U >
	Array1D &
	dimension( Array1< U > const & a )
	{
		dimension_real( a.I_ );
		return *this;
	}

	// Dimension by Array + Initializer Value Template
	template< typename U >
	Array1D &
	dimension( Array1< U > const & a, T const & t )
	{
		dimension_real( a.I_, t );
		return *this;
	}

	// Dimension by Array + Initializer Function Template
	template< typename U >
	Array1D &
	dimension( Array1< U > const & a, InitializerFunction const & fxn )
	{
		dimension_real( a.I_, fxn );
		return *this;
	}

	// Data-Preserving Redimension by IndexRange
	Array1D &
	redimension( IR const & I )
	{
		if ( size_ == 0u ) { // No data
			return dimension( I );
		} else if ( I.size() <= capacity_ ) { // Use existing capacity
			size_type const new_size( I.size() );
			if ( new_size > size_ ) { // Initialize new tail elements
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
				T const fill( Traits::initial_array_value() );
#endif
				for ( size_type i = size_; i < new_size; ++i ) {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
					new ( data_ + i ) T( fill );
#else
					new ( data_ + i ) T;
#endif
				}
			}
			std::ptrdiff_t const off( I_.l() - I.l() );
			if ( off > 0 ) { // Move elements up
				size_type const offu( off );
				size_type const stop( offu < new_size ? std::min( size_, new_size - offu ) : 0u );
				move_or_copy_backward( data_, data_ + stop, data_ + stop + off );
			} else if ( off < 0 ) { // Move elements down
				size_type const offu( -off );
				size_type const stop( offu < size_ ? std::min( size_, new_size + offu ) : 0u );
				if ( offu < stop ) move_or_copy( data_ + offu, data_ + stop, data_ );
			}
			if ( new_size < size_ ) { // Destruct removed tail elements
				for ( size_type i = new_size; i < size_; ++i ) {
					data_[ i ].~T();
				}
			}
			I_ = I;
			shift_set( I_.l() );
			size_ = new_size;
			return *this;
		} else { // Allocate new space
			Array1D o( I, InitializerSentinel{} );
			auto const l_( l() );
			auto const I_l_( I.l() );
			auto const l_max_( std::max( l_, I_l_ ) );
			auto const u_( u() );
			auto const I_u_( I.u() );
			auto const u_min_( std::min( u_, I_u_ ) );
			if ( I_l_ < l_ ) { // Initialize new lower elements
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
				T const fill( Traits::initial_array_value() );
#endif
				for ( int i = I_l_, e = std::min( l_ - 1, I_u_ ); i <= e; ++i ) {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
					new ( &o( i ) ) T( fill );
#else
					new ( &o( i ) ) T;
#endif
				}
			}
			if ( l_max_ <= u_min_ ) { // Ranges overlap
				for ( int i = l_max_; i <= u_min_; ++i ) {
					new ( &o( i ) ) T( move_if( operator ()( i ) ) );
				}
			}
			if ( u_ < I_u_ ) { // Initialize new upper elements
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
				T const fill( Traits::initial_array_value() );
#endif
				for ( int i = std::max( u_ + 1, I_l_ ); i <= I_u_; ++i ) {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
					new ( &o( i ) ) T( fill );
#else
					new ( &o( i ) ) T;
#endif
				}
			}
			swap1( o );
			return *this;
		}
	}

	// Data-Preserving Redimension by IndexRange + Fill Value
	Array1D &
	redimension( IR const & I, T const & t )
	{
		if ( size_ == 0u ) { // No data
			return dimension( I, t );
		} else if ( I.size() <= capacity_ ) { // Use existing capacity
			size_type const new_size( I.size() );
			if ( new_size > size_ ) { // Initialize new tail elements
				for ( size_type i = size_; i < new_size; ++i ) {
					new ( data_ + i ) T( t );
				}
			}
			std::ptrdiff_t const off( I_.l() - I.l() );
			if ( off > 0 ) { // Move elements up
				size_type const offu( off );
				size_type const stop( offu < new_size ? std::min( size_, new_size - offu ) : 0u );
				move_or_copy_backward( data_, data_ + stop, data_ + stop + off );
				std::fill_n( data_, std::min( offu, std::min( size_, new_size ) ), t );
			} else if ( off < 0 ) { // Move elements down
				size_type const offu( -off );
				size_type const stop( offu < size_ ? std::min( size_, new_size + offu ) : 0u );
				if ( offu < stop ) {
					move_or_copy( data_ + offu, data_ + stop, data_ );
					std::fill_n( data_ + ( stop - offu ), std::min( size_, new_size ) - ( stop - offu ), t );
				} else {
					std::fill_n( data_, std::min( size_, new_size ), t );
				}
			}
			if ( new_size < size_ ) { // Destruct removed tail elements
				for ( size_type i = new_size; i < size_; ++i ) {
					data_[ i ].~T();
				}
			}
			I_ = I;
			shift_set( I_.l() );
			size_ = new_size;
			return *this;
		} else { // Allocate new space
			Array1D o( I, InitializerSentinel{} );
			auto const l_( l() );
			auto const I_l_( I.l() );
			auto const l_max_( std::max( l_, I_l_ ) );
			auto const u_( u() );
			auto const I_u_( I.u() );
			auto const u_min_( std::min( u_, I_u_ ) );
			if ( I_l_ < l_ ) { // Fill new lower elements
				for ( int i = I_l_, e = std::min( l_ - 1, I_u_ ); i <= e; ++i ) {
					new ( &o( i ) ) T( t );
				}
			}
			if ( l_max_ <= u_min_ ) { // Ranges overlap
				for ( int i = l_max_; i <= u_min_; ++i ) {
					new ( &o( i ) ) T( move_if( operator ()( i ) ) );
				}
			}
			if ( u_ < I_u_ ) { // Fill new upper elements
				for ( int i = std::max( u_ + 1, I_l_ ); i <= I_u_; ++i ) {
					new ( &o( i ) ) T( t );
				}
			}
			swap1( o );
			return *this;
		}
	}

	// Data-Preserving Redimension by Array Template
	template< typename U >
	Array1D &
	redimension( Array1< U > const & a )
	{
		return redimension( a.I_ );
	}

	// Data-Preserving Redimension by Array + Fill Value Template
	template< typename U >
	Array1D &
	redimension( Array1< U > const & a, T const & t )
	{
		return redimension( a.I_, t );
	}

	// Append Value: Grow by 1
	Array1D &
	append( T const & t )
	{
		if ( capacity_ == size_ ) { // Grow by 1
			Array1D o( IndexRange( l(), u() + 1 ), InitializerSentinel{} );
			for ( int i = l(), e = u(); i <= e; ++i ) {
				new ( &o( i ) ) T( move_if( operator ()( i ) ) );
			}
			swap1( o );
			new ( data_ + size_ - 1 ) T( t );
		} else {
			I_.grow();
			++size_;
			operator ()( u() ) = t;
		}
		return *this;
	}

	// Append Value: Grow by 1
	template< typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	Array1D &
	append( T && t )
	{
		if ( capacity_ == size_ ) { // Grow by 1
			Array1D o( IndexRange( l(), u() + 1 ), InitializerSentinel{} );
			for ( int i = l(), e = u(); i <= e; ++i ) {
				new ( &o( i ) ) T( std::move( operator ()( i ) ) );
			}
			swap1( o );
			new ( data_ + size_ - 1 ) T( std::move( t ) );
		} else {
			I_.grow();
			++size_;
			operator ()( u() ) = std::move( t );
		}
		return *this;
	}


	// Swap
	Array1D &
	swap( Array1D & v )
	{
		using std::swap;
		swap1( v );
		return *this;
	}

public: // std::vector-like API

	// First Value
	T const &
	front() const
	{
		assert( size_ > 0u );
		return operator []( 0u );
	}

	// First Value
	T &
	front()
	{
		assert( size_ > 0u );
		return operator []( 0u );
	}

	// Last Value
	T const &
	back() const
	{
		assert( size_ > 0u );
		return operator []( size_ - 1 );
	}

	// Last Value
	T &
	back()
	{
		assert( size_ > 0u );
		return operator []( size_ - 1 );
	}

	// Append Value by Copy
	Array1D &
	push_back( T const & t )
	{
		I_.grow();
		Base::do_push_back_copy( t );
		return *this;
	}

	// Append Value by Move
	template< typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	Array1D &
	push_back( T && t )
	{
		I_.grow();
		Base::do_push_back_move( std::move( t ) );
		return *this;
	}

	// Remove Last Value
	Array1D &
	pop_back()
	{
		if ( size_ > 0u ) {
			I_.shrink();
			Base::do_pop_back();
		}
		return *this;
	}

	// Insert Value by Copy
	iterator
	insert( const_iterator pos, T const & t )
	{
		I_.grow();
		return Base::do_insert_copy( pos, t );
	}

	// Insert Value by Move
	template< typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	iterator
	insert( const_iterator pos, T && t )
	{
		I_.grow();
		return Base::do_insert_move( pos, std::move( t ) );
	}

	// Insert Multiple Copies of a Value
	iterator
	insert( const_iterator pos, size_type n, T const & t )
	{
		I_.grow( static_cast< int >( n ) );
		return Base::do_insert_n( pos, n, t );
	}

	// Insert Iterator Range
	template< typename Iterator, class = typename std::enable_if<
	 std::is_same< typename std::iterator_traits< Iterator >::iterator_category, std::input_iterator_tag >::value ||
	 std::is_same< typename std::iterator_traits< Iterator >::iterator_category, std::forward_iterator_tag >::value ||
	 std::is_same< typename std::iterator_traits< Iterator >::iterator_category, std::bidirectional_iterator_tag >::value ||
	 std::is_same< typename std::iterator_traits< Iterator >::iterator_category, std::random_access_iterator_tag >::value
	 >::type >
	iterator
	insert( const_iterator pos, Iterator first, Iterator last )
	{
		I_.grow( static_cast< int >( std::distance( first, last ) ) );
		return Base::do_insert_iterator( pos, first, last );
	}

	// Insert Initializer List
	iterator
	insert( const_iterator pos, std::initializer_list< T > il )
	{
		I_.grow( static_cast< int >( il.size() ) );
		return Base::do_insert_initializer_list( pos, il );
	}

	// Insert Value Constructed in Place
	template< typename... Args >
	iterator
	emplace( const_iterator pos, Args &&... args )
	{
		I_.grow();
		return Base::do_emplace( pos, std::forward< Args >( args )... );
	}

	// Append Value Constructed in Place
	template< typename... Args >
	Array1D &
	emplace_back( Args &&... args )
	{
		I_.grow();
		Base::do_emplace_back( std::forward< Args >( args )... );
		return *this;
	}

	// Erase Iterator
	iterator
	erase( const_iterator pos )
	{
		I_.shrink();
		return Base::do_erase( pos );
	}

	// Erase Iterator Range
	iterator
	erase( const_iterator first, const_iterator last )
	{
		I_.shrink( static_cast< int >( std::distance( first, last ) ) );
		return Base::do_erase_iterator( first, last );
	}

	// Reserve Capacity
	Array1D &
	reserve( size_type const n )
	{
		Base::reserve_capacity( n );
		return *this;
	}

protected: // Functions

	// Dimension by IndexRange
	bool
	dimension_assign( IR const & I )
	{
		return size_real( I );
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
		shift_set( I_.l() );
	}

	// Size by IndexRange
	bool
	size_real( IR const & I )
	{
		I_.assign( I );
		shift_only_set( I_.l() );
		return resize( size_of( I_ ) );
	}

	// Dimension by IndexRange
	void
	dimension_real( IR const & I )
	{
		if ( size_real( I ) ) {
			initialize();
		} else {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
			assign();
#endif
		}
	}

	// Dimension by IndexRange + Initializer Value
	void
	dimension_real( IR const & I, T const & t )
	{
		if ( size_real( I ) ) {
			initialize( t );
		} else {
			assign( t );
		}
	}

	// Dimension by IndexRange + Initializer Function
	void
	dimension_real( IR const & I, InitializerFunction const & fxn )
	{
		if ( size_real( I ) ) initialize();
		fxn( *this );
	}

}; // Array1D

// Swap
template< typename T >
inline
void
swap( Array1D< T > & a, Array1D< T > & b )
{
	a.swap( b );
}

// Comparison: Elemental

// Array == Array
template< typename T >
inline
Array1D< bool >
operator ==( Array1< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	eq_elemental( a, b, r );
	return r;
}

// Array != Array
template< typename T >
inline
Array1D< bool >
operator !=( Array1< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	ne_elemental( a, b, r );
	return r;
}

// Array < Array
template< typename T >
inline
Array1D< bool >
operator <( Array1< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	lt_elemental( a, b, r );
	return r;
}

// Array <= Array
template< typename T >
inline
Array1D< bool >
operator <=( Array1< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	le_elemental( a, b, r );
	return r;
}

// Array > Array
template< typename T >
inline
Array1D< bool >
operator >( Array1< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	gt_elemental( a, b, r );
	return r;
}

// Array >= Array
template< typename T >
inline
Array1D< bool >
operator >=( Array1< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	ge_elemental( a, b, r );
	return r;
}

// Array == Value
template< typename T >
inline
Array1D< bool >
operator ==( Array1< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	eq_elemental( a, t, r );
	return r;
}

// Array != Value
template< typename T >
inline
Array1D< bool >
operator !=( Array1< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	ne_elemental( a, t, r );
	return r;
}

// Array < Value
template< typename T >
inline
Array1D< bool >
operator <( Array1< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	lt_elemental( a, t, r );
	return r;
}

// Array <= Value
template< typename T >
inline
Array1D< bool >
operator <=( Array1< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	le_elemental( a, t, r );
	return r;
}

// Array > Value
template< typename T >
inline
Array1D< bool >
operator >( Array1< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	gt_elemental( a, t, r );
	return r;
}

// Array >= Value
template< typename T >
inline
Array1D< bool >
operator >=( Array1< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	ge_elemental( a, t, r );
	return r;
}

// Value == Array
template< typename T >
inline
Array1D< bool >
operator ==( T const & t, Array1< T > const & b )
{
	return ( b == t );
}

// Value != Array
template< typename T >
inline
Array1D< bool >
operator !=( T const & t, Array1< T > const & b )
{
	return ( b != t );
}

// Value < Array
template< typename T >
inline
Array1D< bool >
operator <( T const & t, Array1< T > const & b )
{
	return ( b > t );
}

// Value <= Array
template< typename T >
inline
Array1D< bool >
operator <=( T const & t, Array1< T > const & b )
{
	return ( b >= t );
}

// Value > Array
template< typename T >
inline
Array1D< bool >
operator >( T const & t, Array1< T > const & b )
{
	return ( b < t );
}

// Value >= Array
template< typename T >
inline
Array1D< bool >
operator >=( T const & t, Array1< T > const & b )
{
	return ( b <= t );
}

// Comparison: Elemental: Slice

// Slice == Slice
template< typename T >
inline
Array1D< bool >
operator ==( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) == b( i ) );
	}
	return r;
}

// Slice != Slice
template< typename T >
inline
Array1D< bool >
operator !=( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) != b( i ) );
	}
	return r;
}

// Slice < Slice
template< typename T >
inline
Array1D< bool >
operator <( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) < b( i ) );
	}
	return r;
}

// Slice <= Slice
template< typename T >
inline
Array1D< bool >
operator <=( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) <= b( i ) );
	}
	return r;
}

// Slice > Slice
template< typename T >
inline
Array1D< bool >
operator >( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) > b( i ) );
	}
	return r;
}

// Slice >= Slice
template< typename T >
inline
Array1D< bool >
operator >=( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) >= b( i ) );
	}
	return r;
}

// Slice == Array
template< typename T >
inline
Array1D< bool >
operator ==( Array1S< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	BArray::size_type l( 0u );
	for ( int i = 1, e = r.u(); i <= e; ++i, ++l ) {
		r( i ) = ( a( i ) == b[ l ] );
	}
	return r;
}

// Slice != Array
template< typename T >
inline
Array1D< bool >
operator !=( Array1S< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	BArray::size_type l( 0u );
	for ( int i = 1, e = r.u(); i <= e; ++i, ++l ) {
		r( i ) = ( a( i ) != b[ l ] );
	}
	return r;
}

// Slice < Array
template< typename T >
inline
Array1D< bool >
operator <( Array1S< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	BArray::size_type l( 0u );
	for ( int i = 1, e = r.u(); i <= e; ++i, ++l ) {
		r( i ) = ( a( i ) < b[ l ] );
	}
	return r;
}

// Slice <= Array
template< typename T >
inline
Array1D< bool >
operator <=( Array1S< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	BArray::size_type l( 0u );
	for ( int i = 1, e = r.u(); i <= e; ++i, ++l ) {
		r( i ) = ( a( i ) <= b[ l ] );
	}
	return r;
}

// Slice > Array
template< typename T >
inline
Array1D< bool >
operator >( Array1S< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	BArray::size_type l( 0u );
	for ( int i = 1, e = r.u(); i <= e; ++i, ++l ) {
		r( i ) = ( a( i ) > b[ l ] );
	}
	return r;
}

// Slice >= Array
template< typename T >
inline
Array1D< bool >
operator >=( Array1S< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	BArray::size_type l( 0u );
	for ( int i = 1, e = r.u(); i <= e; ++i, ++l ) {
		r( i ) = ( a( i ) >= b[ l ] );
	}
	return r;
}

// Array == Slice
template< typename T >
inline
Array1D< bool >
operator ==( Array1< T > const & a, Array1S< T > const & b )
{
	return ( b == a );
}

// Array != Slice
template< typename T >
inline
Array1D< bool >
operator !=( Array1< T > const & a, Array1S< T > const & b )
{
	return ( b != a );
}

// Array < Slice
template< typename T >
inline
Array1D< bool >
operator <( Array1< T > const & a, Array1S< T > const & b )
{
	return ( b > a );
}

// Array <= Slice
template< typename T >
inline
Array1D< bool >
operator <=( Array1< T > const & a, Array1S< T > const & b )
{
	return ( b >= a );
}

// Array > Slice
template< typename T >
inline
Array1D< bool >
operator >( Array1< T > const & a, Array1S< T > const & b )
{
	return ( b < a );
}

// Array >= Slice
template< typename T >
inline
Array1D< bool >
operator >=( Array1< T > const & a, Array1S< T > const & b )
{
	return ( b <= a );
}

// Slice == Value
template< typename T >
inline
Array1D< bool >
operator ==( Array1S< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) == t );
	}
	return r;
}

// Slice != Value
template< typename T >
inline
Array1D< bool >
operator !=( Array1S< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) != t );
	}
	return r;
}

// Slice < Value
template< typename T >
inline
Array1D< bool >
operator <( Array1S< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) < t );
	}
	return r;
}

// Slice <= Value
template< typename T >
inline
Array1D< bool >
operator <=( Array1S< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) <= t );
	}
	return r;
}

// Slice > Value
template< typename T >
inline
Array1D< bool >
operator >( Array1S< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) > t );
	}
	return r;
}

// Slice >= Value
template< typename T >
inline
Array1D< bool >
operator >=( Array1S< T > const & a, T const & t )
{
	Array1D< bool > r( Array1D< bool >::shape( a ) );
	for ( int i = 1, e = r.u(); i <= e; ++i ) {
		r( i ) = ( a( i ) >= t );
	}
	return r;
}

// Value == Slice
template< typename T >
inline
Array1D< bool >
operator ==( T const & t, Array1S< T > const & b )
{
	return ( b == t );
}

// Value != Slice
template< typename T >
inline
Array1D< bool >
operator !=( T const & t, Array1S< T > const & b )
{
	return ( b != t );
}

// Value < Slice
template< typename T >
inline
Array1D< bool >
operator <( T const & t, Array1S< T > const & b )
{
	return ( b > t );
}

// Value <= Slice
template< typename T >
inline
Array1D< bool >
operator <=( T const & t, Array1S< T > const & b )
{
	return ( b >= t );
}

// Value > Slice
template< typename T >
inline
Array1D< bool >
operator >( T const & t, Array1S< T > const & b )
{
	return ( b < t );
}

// Value >= Slice
template< typename T >
inline
Array1D< bool >
operator >=( T const & t, Array1S< T > const & b )
{
	return ( b <= t );
}

// Generator

// -Array
template< typename T >
inline
Array1D< T >
operator -( Array1< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= T( -1 );
	return r;
}

// Array + Array
template< typename T >
inline
Array1D< T >
operator +( Array1< T > const & a, Array1< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r += b;
	return r;
}

// Array - Array
template< typename T >
inline
Array1D< T >
operator -( Array1< T > const & a, Array1< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r -= b;
	return r;
}

// Array * Array
template< typename T >
inline
Array1D< T >
operator *( Array1< T > const & a, Array1< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= b;
	return r;
}

// Array / Array
template< typename T >
inline
Array1D< T >
operator /( Array1< T > const & a, Array1< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r /= b;
	return r;
}

// Array + Value
template< typename T >
inline
Array1D< T >
operator +( Array1< T > const & a, T const & t )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r += t;
	return r;
}

// Value + Array
template< typename T >
inline
Array1D< T >
operator +( T const & t, Array1< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r += t;
	return r;
}

// Array - Value
template< typename T >
inline
Array1D< T >
operator -( Array1< T > const & a, T const & t )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r -= t;
	return r;
}

// Value - Array
template< typename T >
inline
Array1D< T >
operator -( T const & t, Array1< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= T( -1 );
	r += t;
	return r;
}

// Array * Value
template< typename T >
inline
Array1D< T >
operator *( Array1< T > const & a, T const & t )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= t;
	return r;
}

// Value * Array
template< typename T >
inline
Array1D< T >
operator *( T const & t, Array1< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= t;
	return r;
}

// Array / Value
template< typename T >
inline
Array1D< T >
operator /( Array1< T > const & a, T const & t )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r /= t;
	return r;
}

// Value / Array
template< typename T >
inline
Array1D< T >
operator /( T const & t, Array1< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r.invert();
	r *= t;
	return r;
}


// Generator: Slice

// -Slice
template< typename T >
inline
Array1D< T >
operator -( Array1S< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= T( -1 );
	return r;
}

// Slice + Slice
template< typename T >
inline
Array1D< T >
operator +( Array1S< T > const & a, Array1S< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r += b;
	return r;
}

// Slice - Slice
template< typename T >
inline
Array1D< T >
operator -( Array1S< T > const & a, Array1S< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r -= b;
	return r;
}

// Slice * Slice
template< typename T >
inline
Array1D< T >
operator *( Array1S< T > const & a, Array1S< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= b;
	return r;
}

// Slice / Slice
template< typename T >
inline
Array1D< T >
operator /( Array1S< T > const & a, Array1S< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r /= b;
	return r;
}

// Slice + Array
template< typename T >
inline
Array1D< T >
operator +( Array1S< T > const & a, Array1< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r += b;
	return r;
}

// Slice - Array
template< typename T >
inline
Array1D< T >
operator -( Array1S< T > const & a, Array1< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r -= b;
	return r;
}

// Slice * Array
template< typename T >
inline
Array1D< T >
operator *( Array1S< T > const & a, Array1< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= b;
	return r;
}

// Slice / Array
template< typename T >
inline
Array1D< T >
operator /( Array1S< T > const & a, Array1< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r /= b;
	return r;
}

// Array + Slice
template< typename T >
inline
Array1D< T >
operator +( Array1< T > const & a, Array1S< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r += b;
	return r;
}

// Array - Slice
template< typename T >
inline
Array1D< T >
operator -( Array1< T > const & a, Array1S< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r -= b;
	return r;
}

// Array * Slice
template< typename T >
inline
Array1D< T >
operator *( Array1< T > const & a, Array1S< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= b;
	return r;
}

// Array / Slice
template< typename T >
inline
Array1D< T >
operator /( Array1< T > const & a, Array1S< T > const & b )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r /= b;
	return r;
}

// Slice + Value
template< typename T >
inline
Array1D< T >
operator +( Array1S< T > const & a, T const & t )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r += t;
	return r;
}

// Value + Slice
template< typename T >
inline
Array1D< T >
operator +( T const & t, Array1S< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r += t;
	return r;
}

// Slice - Value
template< typename T >
inline
Array1D< T >
operator -( Array1S< T > const & a, T const & t )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r -= t;
	return r;
}

// Value - Slice
template< typename T >
inline
Array1D< T >
operator -( T const & t, Array1S< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= T( -1 );
	r += t;
	return r;
}

// Slice * Value
template< typename T >
inline
Array1D< T >
operator *( Array1S< T > const & a, T const & t )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= t;
	return r;
}

// Value * Slice
template< typename T >
inline
Array1D< T >
operator *( T const & t, Array1S< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r *= t;
	return r;
}

// Slice / Value
template< typename T >
inline
Array1D< T >
operator /( Array1S< T > const & a, T const & t )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r /= t;
	return r;
}

// Value / Slice
template< typename T >
inline
Array1D< T >
operator /( T const & t, Array1S< T > const & a )
{
	Array1D< T > r( Array1D< T >::one_based( a ) );
	r.invert();
	r *= t;
	return r;
}




// Cross Product of 3-Tuples
template< typename T >
inline
Array1D< T >
cross( Array1< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	assert( a.size() == 3u );
	Array1D< T > c( 3 );
	c[ 0 ] = ( a[ 1 ] * b[ 2 ] ) - ( a[ 2 ] * b[ 1 ] );
	c[ 1 ] = ( a[ 2 ] * b[ 0 ] ) - ( a[ 0 ] * b[ 2 ] );
	c[ 2 ] = ( a[ 0 ] * b[ 1 ] ) - ( a[ 1 ] * b[ 0 ] );
	return c;
}

// Cross Product of 3-Tuples
template< typename T >
inline
Array1D< T >
cross( Array1< T > const & a, Array1S< T > const & b )
{
	assert( conformable( a, b ) );
	assert( a.size() == 3u );
	Array1D< T > c( 3 );
	c[ 0 ] = ( a[ 1 ] * b[ 2 ] ) - ( a[ 2 ] * b[ 1 ] );
	c[ 1 ] = ( a[ 2 ] * b[ 0 ] ) - ( a[ 0 ] * b[ 2 ] );
	c[ 2 ] = ( a[ 0 ] * b[ 1 ] ) - ( a[ 1 ] * b[ 0 ] );
	return c;
}

// Cross Product of 3-Tuples
template< typename T >
inline
Array1D< T >
cross( Array1S< T > const & a, Array1< T > const & b )
{
	assert( conformable( a, b ) );
	assert( a.size() == 3u );
	Array1D< T > c( 3 );
	c[ 0 ] = ( a[ 1 ] * b[ 2 ] ) - ( a[ 2 ] * b[ 1 ] );
	c[ 1 ] = ( a[ 2 ] * b[ 0 ] ) - ( a[ 0 ] * b[ 2 ] );
	c[ 2 ] = ( a[ 0 ] * b[ 1 ] ) - ( a[ 1 ] * b[ 0 ] );
	return c;
}

// Cross Product of 3-Tuples
template< typename T >
inline
Array1D< T >
cross( Array1S< T > const & a, Array1S< T > const & b )
{
	assert( conformable( a, b ) );
	assert( a.size() == 3u );
	Array1D< T > c( 3 );
	c[ 0 ] = ( a[ 1 ] * b[ 2 ] ) - ( a[ 2 ] * b[ 1 ] );
	c[ 1 ] = ( a[ 2 ] * b[ 0 ] ) - ( a[ 0 ] * b[ 2 ] );
	c[ 2 ] = ( a[ 0 ] * b[ 1 ] ) - ( a[ 1 ] * b[ 0 ] );
	return c;
}

// Cross Product of 3-Tuples
template< typename T >
inline
Array1D< T >
cross_product( Array1< T > const & a, Array1< T > const & b )
{
	return cross( a, b );
}

// Cross Product of 3-Tuples
template< typename T >
inline
Array1D< T >
cross_product( Array1< T > const & a, Array1S< T > const & b )
{
	return cross( a, b );
}

// Cross Product of 3-Tuples
template< typename T >
inline
Array1D< T >
cross_product( Array1S< T > const & a, Array1< T > const & b )
{
	return cross( a, b );
}

// Cross Product of 3-Tuples
template< typename T >
inline
Array1D< T >
cross_product( Array1S< T > const & a, Array1S< T > const & b )
{
	return cross( a, b );
}

} // ObjexxFCL

#endif // ObjexxFCL_Array1D_hh_INCLUDED
