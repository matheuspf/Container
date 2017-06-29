/** \file Helpers.h
  *
  * Some metaprogramming helpers
*/

#ifndef CNT_VECTOR_HELPERS_H
#define CNT_VECTOR_HELPERS_H

#include <type_traits>
#include <vector>
#include <array>



namespace std
{
	/** These are not available in C++14 */
	//@{
	template <typename T, typename U>
	constexpr bool is_same_v = std::is_same< T, U >::value;

	template <typename T>
	constexpr bool is_integral_v = std::is_integral<T>::value;
	//@}
}




namespace cnt
{

namespace help
{

/// A single stack allocation must not exceed this limit
constexpr std::size_t maxSize = 100000 * sizeof(char);


//---------------------------------------------------------------------------




/** Multiply the integers to get the total size at compile time. If no argument
  * is give, the function returns 0
  *
  * \param[in] Is The variadic arguments
  * \return The total size of multiplying Is or 0, if sizeof...(Is) == 0
*/
//@{
template <std::size_t... Is, std::enable_if_t<sizeof...(Is), int> = 0>
constexpr inline std::size_t multiply ()
{
	std::size_t res = 1;

	const auto& dummie = { (res *= Is, int{})... };

	return res;
}

constexpr inline std::size_t multiply ()
{
	return 0;
}
//@}



//-----------------------------------------------------------------------------------


/** Test truthness of variadic set of bool arguments. Only gives a true value
  * if all elements are true.
*/
//@{
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
//@}



/** Helper for creating std::array from variadic arguments
  *
  * \note All arguments must be of the same type
  *
  * \param[in] t First argument, to guarantee at least one element
  * \param[in] args Variadic arguments to create the array.
*/
template <typename T, typename... Args, typename = std::enable_if_t< And_v< std::is_same_v< T, Args >... > > >
inline constexpr auto makeArray (T t, Args... args)
{
    return std::array< T, sizeof...(Args) + 1 >{ t, args... };
}




/** If the compile time size is greater than 0 and less than the defined maxSize, 
  * the Vector is a std::array. Otherwise it is a std::vector
*/
//@{
template <std::size_t N>
constexpr bool isArray = ((N > 0) && (N < maxSize)) ? true : false;

template <std::size_t N>
constexpr bool isVector = !isArray< N >;
//@}


/// Selects either a 'std::array<T, N>' or a 'std::vector<T>' depending on the size 'N'
template <typename T, std::size_t N>
using SelectType = std::conditional_t<help::isArray<N>, std::array<T, N>, std::vector<T>>;



/** Some useful helpers for SFINAE */
//@{

/// Enable if 'N' satisfy 'isArray' trait
template <std::size_t N>
using EnableIfArray = std::enable_if_t< isArray< N >, int >;

/// Enable if 'N' satisfy 'isVector' trait
template <std::size_t N>
using EnableIfVector = std::enable_if_t< isVector< N >, int >;

/// Enable if 'N' is not '0' trait
template <std::size_t N>
using EnableIfZero = std::enable_if_t< (N == 0), int >;


/// Enable if all 'Args' are integrals
template <typename... Args>
using EnableIfIntegral = std::enable_if_t< And_v< std::is_integral_v< Args >... >, int >; 



/// Enable if all 'Args' are iterable types (has an iterator given by std::begin)
template <typename... Args>
using EnableIfIterable = std::enable_if_t < And_v< bool(sizeof...(Args)), !std::is_same_v< std::decay_t< decltype( 
													*std::begin( std::declval< Args >() )) >, void >... > &&
										    And_v<std::is_integral_v<std::decay_t<decltype(
										    		*std::begin( std::declval< Args >() ))> > ... >, int >; 


/// Enable if all 'Args' are iterators or pointers to integral types
template <typename... Args>
using EnableIfIntIter = std::enable_if_t< And_v< (std::is_pointer< Args >::value && 
												    std::is_integral_v< decltype( *std::declval< Args >() ) >)... > ||
										  And_v<  std::is_integral_v< typename std::iterator_traits< Args >::value_type > ... >, int >;
//@}

}   // namespace help

}


#endif // CNT_VECTOR_HELPERS_H
