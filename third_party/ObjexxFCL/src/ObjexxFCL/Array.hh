#ifndef ObjexxFCL_Array_hh_INCLUDED
#define ObjexxFCL_Array_hh_INCLUDED

// Array: Array Abstract Base Class
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
#include <ObjexxFCL/Array.fwd.hh>
#include <ObjexxFCL/BArray.hh>
#include <ObjexxFCL/AlignedAllocator.hh>
#include <ObjexxFCL/InitializerSentinel.hh>
#include <ObjexxFCL/ProxySentinel.hh>
#include <ObjexxFCL/TypeTraits.hh>

// C++ Headers
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <istream>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace ObjexxFCL {

// Array: Array Abstract Base Class
//
// Note:
//  Can hold numeric or non-numeric values: Numeric operations on non-numeric values won't compile
//  Any meaningful array index ranges can be specified as in Fortran
//  Zero-sized arrays are supported but have no valid indices
//  Argument/proxy arrays can have unbounded/unknown size
//  For efficiency constructors without initializer function or value do not initialize the array
//  For efficiency bounds checking is only active via asserts in debug builds
template< typename T >
class Array : public BArray
{

private: // Friend

	template< typename > friend class Array;

protected: // Types

	typedef  internal::InitializerSentinel  InitializerSentinel;
	typedef  internal::ProxySentinel  ProxySentinel;

public: // Types

	typedef  Array< T >  Base;
	typedef  AlignedAllocator< T >  Aligned;
	typedef  TypeTraits< T >  Traits;

	// STL style
	typedef  T  value_type;
	typedef  T &  reference;
	typedef  T const &  const_reference;
	typedef  T *  pointer;
	typedef  T const *  const_pointer;
	typedef  T *  iterator;
	typedef  T const *  const_iterator;
	typedef  std::reverse_iterator< T * >  reverse_iterator;
	typedef  std::reverse_iterator< T const * >  const_reverse_iterator;

	// C++ style
	typedef  T  Value;
	typedef  T &  Reference;
	typedef  T const &  ConstReference;
	typedef  T *  Pointer;
	typedef  T const *  ConstPointer;
	typedef  T *  Iterator;
	typedef  T const *  ConstIterator;
	typedef  std::reverse_iterator< T * >  ReverseIterator;
	typedef  std::reverse_iterator< T const * >  ConstReverseIterator;

protected: // Creation

	// Default Constructor
	Array() :
	 owner_( true ),
	 capacity_( 0u ),
	 size_( 0u ),
	 mem_( nullptr ),
	 data_( nullptr ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Copy Constructor
	Array( Array const & a ) :
	 BArray( a ),
	 owner_( true ),
	 capacity_( size_of( a.size_ ) ),
	 size_( capacity_ ),
	 mem_( a.data_ != nullptr ? Aligned::allocate_zero( capacity_ ) : nullptr ),
	 data_( a.data_ != nullptr ? Aligned::data( mem_ ) : nullptr ),
	 shift_( a.shift_ ),
	 sdata_( data_ - shift_ )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( a.data_[ i ] );
		}
	}

	// Move Constructor
	Array( Array && a ) noexcept :
	 BArray( std::move( a ) ),
	 owner_( a.owner_ ),
	 capacity_( a.capacity_ ),
	 size_( a.size_ ),
	 mem_( a.mem_ ),
	 data_( a.data_ ),
	 shift_( a.shift_ ),
	 sdata_( a.sdata_ )
	{
		a.capacity_ = a.size_ = 0u;
		a.mem_ = a.data_ = a.sdata_ = nullptr;
		a.shift_ = 0;
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array( Array< U > const & a ) :
	 owner_( true ),
	 capacity_( size_of( a.size() ) ),
	 size_( capacity_ ),
	 mem_( a.data_ != nullptr ? Aligned::allocate_zero( capacity_ ) : nullptr ),
	 data_( a.data_ != nullptr ? Aligned::data( mem_ ) : nullptr ),
	 shift_( a.shift_ ),
	 sdata_( data_ - shift_ )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( a.data_[ i ] );
		}
	}

	// Size Constructor
	explicit
	Array( size_type const size ) :
	 owner_( true ),
	 capacity_( size_of( size ) ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
		T const fill( Traits::initial_array_value() );
#endif
		for ( size_type i = 0; i < size_; ++i ) {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
			new ( data_ + i ) T( fill );
#else
			new ( data_ + i ) T;
#endif
		}
	}

	// Size + InitializerSentinel Constructor
	Array( size_type const size, InitializerSentinel ) :
	 owner_( true ),
	 capacity_( size_of( size ) ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array( std::initializer_list< U > const l ) :
	 owner_( true ),
	 capacity_( l.size() ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		auto il( l.begin() );
		for ( size_type i = 0; i < size_; ++i, ++il ) {
			new ( data_ + i ) T( *il );
		}
	}

	// std::array Constructor Template
	template< typename U, Size s, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array( std::array< U, s > const & a ) :
	 owner_( true ),
	 capacity_( s ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		auto ia( a.begin() );
		for ( size_type i = 0; i < size_; ++i, ++ia ) {
			new ( data_ + i ) T( *ia );
		}
	}

	// Default Proxy Constructor
	Array( ProxySentinel ) :
	 owner_( false ),
	 capacity_( 0u ),
	 size_( 0u ),
	 mem_( nullptr ),
	 data_( nullptr ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Array Proxy Constructor
	Array( Array const & a, ProxySentinel ) :
	 owner_( false ),
	 capacity_( a.capacity_ ),
	 size_( a.size_ ),
	 mem_( nullptr ),
	 data_( a.data_ ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Value Proxy Constructor
	Array( T const & t, ProxySentinel ) :
	 owner_( false ),
	 capacity_( npos ), // Unknown
	 size_( npos ), // Unbounded
	 mem_( nullptr ),
	 data_( const_cast< T * >( &t ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

public: // Creation

	// Destructor
	virtual
	~Array()
	{
		if ( owner_ ) destroy();
	}

protected: // Assignment: Array

	// Copy Assignment
	void
	operator =( Array const & a )
	{
		assert( size_bounded() );
		assert( size_ == a.size_ );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] = a[ i ];
		}
	}

	// Move Assignment
	void
	operator =( Array && a ) noexcept
	{
		assert( this != &a );
		assert( owner_ == a.owner_ );
		if ( owner_ ) destroy();
		capacity_ = a.capacity_;
		size_ = a.size_;
		mem_ = a.mem_;
		data_ = a.data_;
		shift_ = a.shift_;
		sdata_ = a.sdata_;
		a.capacity_ = a.size_ = 0u;
		a.mem_ = a.data_ = a.sdata_ = nullptr;
		a.shift_ = 0;
	}

	// Copy Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( Array< U > const & a )
	{
		assert( size_bounded() );
		assert( size_ == a.size() );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] = a[ i ];
		}
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( std::initializer_list< U > const l )
	{
		assert( size_ == l.size() );
		std::copy( l.begin(), l.end(), data_ );
	}

	// std::array Assignment Template
	template< typename U, Size s, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( std::array< U, s > const & a )
	{
		assert( size_ == s );
		std::copy( a.begin(), a.end(), data_ );
	}

public: // Assignment: Value

	// = Value
	Array &
	operator =( T const & t )
	{
		assert( size_bounded() );
		if ( data_ ) std::fill_n( data_, size_, t );
		return *this;
	}

public: // Subscript

	// array[ i ] const: Linear Subscript
	T const &
	operator []( size_type const i ) const
	{
		assert( ( i < size_ ) || ( size_ == npos ) );
		return data_[ i ];
	}

	// array[ i ]: Linear Subscript
	T &
	operator []( size_type const i )
	{
		assert( ( i < size_ ) || ( size_ == npos ) );
		return data_[ i ];
	}

public: // Predicate

	// Owner?
	bool
	owner() const
	{
		return owner_;
	}

	// Proxy?
	bool
	proxy() const
	{
		return ! owner_;
	}

	// Active?
	bool
	active() const
	{
		return ( data_ != nullptr );
	}

	// Allocated
	bool
	allocated() const
	{
		return ( data_ != nullptr );
	}

	// Active Array Empty?
	bool
	empty() const
	{
		return ( size_ == 0u );
	}

	// Active Array Size Bounded?
	bool
	size_bounded() const
	{
		return ( size_ != npos );
	}

	// Memory Can Overlap a Range?
	bool
	overlap( T const * const b, T const * const e ) const
	{
		if ( ( data_ == nullptr ) || ( b == nullptr ) || ( e == nullptr ) ) { // No active memory range(s)
			return false;
		} else if ( size_ == 0u ) { // No finite memory range
			return false;
		} else if ( size_ == npos ) { // No memory upper bound
			return ( e >= data_ );
		} else { // Bounded ranges
			assert( b <= e );
			T const * const de( data_ + size_ - 1 ); // Data end pointer
			return ( ( data_ >= b ? data_ : b ) <= ( de <= e ? de : e ) );
		}
	}

	// Memory Can Overlap an Array?
	template< template< typename > class A >
	bool
	overlap( A< T > const & a ) const
	{
		if ( data_ == nullptr ) { // No active memory range
			return false;
		} else { // Bounded ranges
			return overlap( a.data_beg(), a.data_end() );
		}
	}

public: // Inspector

	// Rank
	virtual
	int
	rank() const = 0;

	// Data Size
	size_type
	capacity() const
	{
		return capacity_;
	}

	// Active Array Size
	size_type
	size() const
	{
		return size_;
	}

	// Active Array Size
	int
	isize() const
	{
		assert( size_ != npos );
		return static_cast< int >( size_ );
	}

	// IndexRange of a Dimension
	virtual
	IR const &
	I( int const d ) const = 0;

	// Lower Index of a Dimension
	virtual
	int
	l( int const d ) const = 0;

	// Upper Index of a Dimension
	virtual
	int
	u( int const d ) const = 0;

	// Size of a Dimension
	virtual
	size_type
	size( int const d ) const = 0;

	// Size of a Dimension
	virtual
	int
	isize( int const d ) const = 0;

	// Begin Iterator
	const_iterator
	begin() const
	{
		return data_;
	}

	// Begin Iterator
	iterator
	begin()
	{
		return data_;
	}

	// End Iterator
	const_iterator
	end() const
	{
		return ( ( data_ != nullptr ) && ( size_ != npos ) ? data_ + size_ : nullptr );
	}

	// End Iterator
	iterator
	end()
	{
		return ( ( data_ != nullptr ) && ( size_ != npos ) ? data_ + size_ : nullptr );
	}

	// Reverse Begin Iterator
	const_reverse_iterator
	rbegin() const
	{
		return const_reverse_iterator( ( data_ != nullptr ) && ( size_ != npos ) ? data_ + size_ : nullptr );
	}

	// Reverse Begin Iterator
	reverse_iterator
	rbegin()
	{
		return reverse_iterator( ( data_ != nullptr ) && ( size_ != npos ) ? data_ + size_ : nullptr );
	}

	// Reverse End Iterator
	const_reverse_iterator
	rend() const
	{
		return const_reverse_iterator( data_ );
	}

	// Reverse End Iterator
	reverse_iterator
	rend()
	{
		return reverse_iterator( data_ );
	}

	// Data Pointer
	T const *
	data() const
	{
		return data_;
	}

	// Data Pointer
	T *
	data()
	{
		return data_;
	}

	// Data Begin Pointer
	T const *
	data_beg() const
	{
		return data_;
	}

	// Data Begin Pointer
	T *
	data_beg()
	{
		return data_;
	}

	// Data End Pointer
	T const *
	data_end() const
	{
		return ( ( data_ != nullptr ) && ( size_ > 0u ) && ( size_ != npos ) ? data_ + size_ - 1 : nullptr );
	}

	// Data End Pointer
	T *
	data_end()
	{
		return ( ( data_ != nullptr ) && ( size_ > 0u ) && ( size_ != npos ) ? data_ + size_ - 1 : nullptr );
	}

public: // Modifier

	// Clear
	virtual
	Array &
	clear()
	{
		if ( owner_ ) destroy();
		capacity_ = size_ = 0u;
		mem_ = data_ = sdata_ = nullptr;
		shift_ = 0;
		return *this;
	}

public: // Comparison: Predicate

public: // Comparison: Predicate: Any

public: // Comparison: Predicate: All

public: // Comparison: Count

protected: // Comparison: Predicate

protected: // Comparison: Elemental

protected: // Methods

	// Shift Setup
	void
	shift_set( difference_type const shift )
	{
		shift_ = shift;
		sdata_ = data_ - shift_;
	}

	// Shift Setup Without Setting Shifted Data Pointer
	void
	shift_only_set( difference_type const shift )
	{
		shift_ = shift;
	}

	// Active Array Size Setup
	void
	size_set( size_type const size )
	{
		assert( size <= capacity_ );
		size_ = size;
	}

	// Resize a Real Array: Return Whether Reallocation Happened
	bool
	resize( size_type const size )
	{
		assert( owner_ );
		assert( size != npos );
		if ( ( data_ == nullptr ) || ( capacity_ < size ) || ( ( capacity_ == size_ ) && ( size != size_ ) ) ) {
			destroy();
			capacity_ = size_ = size;
			mem_ = Aligned::allocate_zero( capacity_ );
			data_ = Aligned::data( mem_ );
			sdata_ = data_ - shift_;
			return true; // Reallocated: Elements not constructed
		} else {
			size_type i( size_ );
			while ( i > size ) { // Destruct removed elements
				data_[ --i ].~T();
			}
			size_ = size;
			sdata_ = data_ - shift_;
			return false; // Not reallocated
		}
	}

	// Reserve Capacity in a Real Array
	void
	reserve_capacity( size_type const n )
	{
		assert( owner_ );
		assert( n <= max_size );
		if ( capacity_ < n ) {
			void * new_mem = Aligned::allocate_zero( n );
			T * new_data = Aligned::data( new_mem );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			capacity_ = n;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		}
	}

	// Append Value by Copy in a Real Array
	void
	do_push_back_copy( T const & t )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			new ( &new_data[ size_ ] ) T( t );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		} else {
			new ( &data_[ size_ ] ) T( t );
		}
		size_ = new_size;
	}

	// Append Value by Move in a Real Array
	void
	do_push_back_move( T && t )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			new ( &new_data[ size_ ] ) T( std::move( t ) );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		} else {
			new ( &data_[ size_ ] ) T( std::move( t ) );
		}
		size_ = new_size;
	}

	// Insert Value by Copy in a Real Array
	iterator
	do_insert_copy( const_iterator pos, T const & t )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		assert( data_ <= pos );
		assert( pos <= end() );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			iterator const new_pos( new_data + ( pos - data_ ) );
			new ( &*new_pos ) T( t );
			if ( data_ < pos ) uninitialized_move_or_copy( data_, old_pos, new_data );
			if ( pos < old_end ) uninitialized_move_or_copy( old_pos, old_end, new_pos + 1 );
			destroy();
			size_ = new_size;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
			return new_pos;
		} else {
			if ( pos == old_end ) {
				new ( &*old_end ) T( t );
			} else {
				T const tt( t );
				uninitialized_move_or_copy( old_end - 1, old_end, old_end );
				if ( size_ > 1u ) move_or_copy_backward( old_pos, old_end - 1, old_end );
				*old_pos = tt;
			}
			size_ = new_size;
			return old_pos;
		}
	}

	// Append Value Constructed in Place in a Real Array
	template< typename... Args >
	void
	do_emplace_back( Args &&... args )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			new ( &new_data[ size_ ] ) T( std::forward< Args >( args )... );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		} else {
			new ( &data_[ size_ ] ) T( std::forward< Args >( args )... );
		}
		size_ = new_size;
	}

	// Erase Iterator in a Real Array
	iterator
	do_erase( const_iterator pos )
	{
		assert( owner_ );
		assert( data_ <= pos );
		assert( pos < end() );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		move_or_copy( old_pos + 1, old_end, old_pos );
		old_end->~T();
		--size_;
		return old_pos;
	}

	// Swap
	void
	swapB( Array & v )
	{
		assert( owner_ );
		assert( v.owner_ );
		std::swap( capacity_, v.capacity_ );
		std::swap( size_, v.size_ );
		std::swap( mem_, v.mem_ );
		std::swap( data_, v.data_ );
		std::swap( shift_, v.shift_ );
		std::swap( sdata_, v.sdata_ );
	}

	// Initialize by Uniform Value
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	initialize( U const & u )
	{
		assert( owner_ );
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( u );
		}
	}

	// Initialize by Array
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	initialize( Array< U > const & a )
	{
		assert( owner_ );
		assert( size_ == a.size_ );
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( a[ i ] );
		}
	}

	// Initialize an Element
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	initialize( size_type const i, U const & u )
	{
		assert( owner_ );
		new ( data_ + i ) T( u );
	}

	// Uniform Assignment
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	assign( U const & u )
	{
		assert( owner_ );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] = T( u );
		}
	}

	// Element Assignment
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	assign( size_type const i, U const & u )
	{
		data_[ i ] = T( u );
	}

	// Assign Array
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	assign( Array< U > const & a )
	{
		assert( size_bounded() );
		assert( size_ == a.size_ );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] = T( a[ i ] );
		}
	}

	// Switch to Size Construction
	void
	reconstruct_by_size( size_type const size )
	{
		assert( owner_ );
		destroy();
		capacity_ = size_ = size;
		assert( size_bounded() );
		mem_ = Aligned::allocate_zero( capacity_ );
		data_ = Aligned::data( mem_ );
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
		T const fill( Traits::initial_array_value() );
#endif
		for ( size_type i = 0; i < size_; ++i ) {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
			new ( data_ + i ) T( fill );
#else
			new ( data_ + i ) T;
#endif
		}
	}

	// Conformable Move
	void
	conformable_move( Array & a )
	{
		assert( this != &a );
		assert( owner_ == a.owner_ );
		destroy();
		capacity_ = a.capacity_;
		mem_ = a.mem_;
		data_ = a.data_;
		sdata_ = data_ - shift_;
		a.capacity_ = a.size_ = 0u;
		a.mem_ = a.data_ = a.sdata_ = nullptr;
		a.shift_ = 0;
	}

protected: // Static Methods

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1 )
	{
		assert( s1 != npos );
		return s1;
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2 )
	{
		assert( s1 != npos );
		assert( s2 != npos );
		assert( ( s2 == 0 ) || ( s1 <= max_size / s2 ) );
		return s1 * s2;
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2, size_type const s3 )
	{
		return size_of( size_of( s1, s2 ), s3 );
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2, size_type const s3, size_type const s4 )
	{
		return size_of( size_of( s1, s2 ), size_of( s3, s4 ) );
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2, size_type const s3, size_type const s4, size_type const s5 )
	{
		return size_of( size_of( size_of( s1, s2 ), size_of( s3, s4 ) ), s5 );
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2, size_type const s3, size_type const s4, size_type const s5, size_type const s6 )
	{
		return size_of( size_of( size_of( s1, s2 ), size_of( s3, s4 ) ), size_of( s5, s6 ) );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1 )
	{
		return size_of( I1.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2 )
	{
		return size_of( I1.size(), I2.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2, IR const & I3 )
	{
		return size_of( I1.size(), I2.size(), I3.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2, IR const & I3, IR const & I4 )
	{
		return size_of( I1.size(), I2.size(), I3.size(), I4.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2, IR const & I3, IR const & I4, IR const & I5 )
	{
		return size_of( I1.size(), I2.size(), I3.size(), I4.size(), I5.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2, IR const & I3, IR const & I4, IR const & I5, IR const & I6 )
	{
		return size_of( I1.size(), I2.size(), I3.size(), I4.size(), I5.size(), I6.size() );
	}

	// Move if Movable: Movable Overload
	template< typename U, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	static
	U &&
	move_if( U & u )
	{
		return std::move( u );
	}

	// Move if Movable: Non-Movable Overload
	template< typename U, class = typename std::enable_if< ! std::is_move_assignable< U >::value >::type, typename = void >
	static
	U &
	move_if( U & u )
	{
		return u;
	}

	// Move or Copy: Move Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	static
	OutputIterator
	move_or_copy( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::move( beg, end, out );
	}

	// Move or Copy: Copy Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< ! std::is_move_assignable< U >::value >::type, typename = void >
	static
	OutputIterator
	move_or_copy( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::copy( beg, end, out );
	}

	// Move or Copy Backward: Move Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	static
	OutputIterator
	move_or_copy_backward( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::move_backward( beg, end, out );
	}

	// Move or Copy Backward: Copy Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< ! std::is_move_assignable< U >::value >::type, typename = void >
	static
	OutputIterator
	move_or_copy_backward( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::copy_backward( beg, end, out );
	}

	// Move or Copy: Move Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	static
	OutputIterator
	uninitialized_move_or_copy( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::uninitialized_copy( std::make_move_iterator( beg ), std::make_move_iterator( end ), out );
	}

	// Move or Copy: Copy Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< ! std::is_move_assignable< U >::value >::type, typename = void >
	static
	OutputIterator
	uninitialized_move_or_copy( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::uninitialized_copy( beg, end, out );
	}

private: // Methods

	// Destruct Elements and Delete Array Memory (Doesn't Nullify Pointers)
	void
	destroy()
	{
		if ( data_ != nullptr ) {
			size_type i( size_ );
			while ( i ) {
				data_[ --i ].~T();
			}
		}
		::operator delete( mem_ );
	}

protected: // Data

	bool const owner_; // Owner of data array?
	size_type capacity_; // Size of data array
	size_type size_; // Size of active array
	void * mem_; // Pointer to raw memory
	T * data_; // Pointer to data array
	difference_type shift_; // Array shift
	T * sdata_; // Shifted pointer to data array

}; // Array

// Stream >> Array
template< typename T >
inline
std::istream &
operator >>( std::istream & stream, Array< T > & a )
{
	if ( stream && ( a.size() > 0u ) ) {
		for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
			stream >> a[ i ];
			if ( ! stream ) break;
		}
	}
	return stream;
}

// Stream << Array
template< typename T >
inline
std::ostream &
operator <<( std::ostream & stream, Array< T > const & a )
{
	typedef  TypeTraits< T >  Traits;
	if ( stream && ( a.size() > 0u ) ) {
		std::ios_base::fmtflags const old_flags( stream.flags() );
		std::streamsize const old_precision( stream.precision( Traits::precision ) );
		stream << std::right << std::showpoint << std::uppercase;
		int const w( Traits::iwidth );
		for ( BArray::size_type i = 0, e = a.size() - 1; i < e; ++i ) {
			stream << std::setw( w ) << a[ i ] << ' ';
			if ( ! stream ) break;
		} if ( stream ) stream << std::setw( w ) << a[ a.size() - 1 ];
		stream.precision( old_precision );
		stream.flags( old_flags );
	}
	return stream;
}

} // ObjexxFCL

#endif // ObjexxFCL_Array_hh_INCLUDED
