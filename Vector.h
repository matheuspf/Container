#ifndef CNT_VECTOR_H
#define CNT_VECTOR_H


#include "Helpers.h"


namespace cnt
{


// Main class
//
// cnt::Vector inherits from a std::vector if it is not supplied with compile time size
// or if the given compile time size is greater than the value defined at cnt::help::maxSize.
// for maximum stack size allocation. Otherwise it inherits from std::array

template <typename T, std::size_t N = 0>
struct Vector : public std::conditional_t<help::isArray<N>, std::array<T, N>, std::vector<T>>
{
    using Base = std::conditional_t<help::isArray<N>, std::array<T, N>, std::vector<T>>;

    using Base::Base;   // Inherits all constructors of std::vector. std::array has no constructor


    static constexpr std::size_t Size = N;


    static constexpr bool isArray  = help::isArray<N>;

    static constexpr bool isVector = help::isVector<N>;


    // If compile time size N is greater than help::maxSize, initialize the std::vector with this size

    template <std::size_t M = Size, help::EnableIfVector<M> = 0>
    Vector () : Base(Size) {}


    // For std::array aggregate initialization

    template <typename... Args, std::size_t M = Size, help::EnableIfArray<M> = 0>
    Vector (Args&&... args) : Base{std::forward<Args>(args)...} {}
};


} // namespace cnt


#endif  // CNT_VECTOR_H