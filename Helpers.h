#ifndef CNT_VECTOR_HELPERS_H
#define CNT_VECTOR_HELPERS_H

#include <type_traits>
#include <vector>
#include <array>


// Helpers


namespace std
{
	template <typename T, typename U>
	constexpr bool is_same_v = std::is_same< T, U >::value;

	template <typename T>
	constexpr bool is_integral_v = std::is_integral<T>::value;
}

namespace cnt
{

namespace help
{

// Stack allocation must not exceed this limit

constexpr std::size_t maxSize = 100000 * sizeof(char);


//---------------------------------------------------------------------------

// Helper for creating constexpr functions for variadic arguments

#define OPERATION(NAME, BIN_OP, IDENTITY) \
\
constexpr inline auto NAME () { return IDENTITY; }  \
\
template <typename T> \
constexpr inline auto NAME (T&& t){ return t; } \
\
template <typename T, typename U>   \
constexpr inline auto NAME (T&& t, U&& u) { return BIN_OP(std::forward<T>(t), std::forward<U>(u)); }    \
\
template <typename T, typename... Args> \
constexpr inline auto NAME (T&& t, Args&&... args) { return BIN_OP(std::forward<T>(t), NAME(std::forward<Args>(args)...)); }


#define MULT(x, y) (x) * (y)    // Because there is no way to pass a constexpr function in as argument, this is necessary

OPERATION(multiply, MULT, 0);   // Declares a function called 'multiply' with the operation MULT
                               	// This could be done too: 'OPERATION(mult, [](auto&& x, auto&& y){ ... })',
                                // but is no longer a constexpr function


//-----------------------------------------------------------------------------------

// Test truthness of variadic set of bool arguments

template <bool...>
struct And;

template <bool B1, bool... Bs>
struct And< B1, Bs... > : And< Bs... > {};

template <bool... Bs>
struct And< false, Bs... > : std::false_type {};

template <>
struct And<true> : std::true_type {};

template <>
struct And<> : std::true_type {};

template <bool... Bs>
constexpr bool And_v = And< Bs... >::value;




// Helper for creating std::array from variadic arguments

template <typename T, typename... Args, typename = std::enable_if_t< And_v< std::is_same_v< T, Args >... > > >
inline constexpr auto makeArray (T t, Args... args)
{
    return std::array< T, sizeof...(Args) + 1 >{ t, args... };
}




// If the compile time size is greater than 0 and less than the defined maxSize, 
// the Vector is a std::array. Otherwise it is a std::vector

template <std::size_t N>
constexpr bool isArray = ((N > 0) && (N < maxSize)) ? true : false;

template <std::size_t N>
constexpr bool isVector = !isArray< N >;


template <std::size_t N>
using EnableIfArray = std::enable_if_t< isArray< N >, int >;	// Enable if size 'N' makes an array

template <std::size_t N>
using EnableIfVector = std::enable_if_t< isVector< N >, int >;  // Enable if size 'N' makes a vector

template <std::size_t N>
using EnableIfZero = std::enable_if_t< (N == 0), int >;		// Enable if size 'N' is 0



// / Enable if all 'Args' are integrals

template <typename... Args>
using EnableIfIntegral = std::enable_if_t< And_v< std::is_integral_v< Args >... >, int >; 


// Enable if all 'Args' are iterable types (has an iterator given by std::begin)

template <typename... Args>
using EnableIfIterable = std::enable_if_t < And_v< !std::is_same_v< std::decay_t< decltype( 
													*std::begin( std::declval< Args >() )) >, void >... >, int >; 


// Enable if all 'Args' are iterators or pointers to integral types

template <typename... Args>
using EnableIfIntIter = std::enable_if_t< ( And_v< (std::is_pointer< Args >::value && 
												    std::is_integral_v< decltype( *std::declval< Args >() ) >)... > ||
										    And_v<  std::is_integral_v< typename std::iterator_traits< Args >::value_type 
										    > ... >), int >;


}   // namespace help

}


#endif // CNT_VECTOR_HELPERS_H
