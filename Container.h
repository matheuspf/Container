#ifndef CNT_CONTAINER_H
#define CNT_CONTAINER_H


#include "Vector.h"

#include <tuple>
#include <algorithm>
#include <cmath>
#include <numeric>


#include <iostream>
#define DB(...) std::cout << __VA_ARGS__ << "\n" << std::flush


namespace cnt
{

template <typename T, std::size_t... Is>
struct Container : public Vector<T, help::multiply(Is...)>
{

    using Base = Vector<T, help::multiply(Is...)>;

    //using Base::Base;

    using value_type = typename Base::value_type;

    using reference = typename Base::reference;

    using const_reference = typename Base::const_reference;

    using Base::Size;

    //friend class Slice<Container<T, Is...>>;


    template <typename... Args, std::size_t M = Size, 
              std::enable_if_t<(help::isArray< M > || M >= help::maxSize), int > = 0>
    Container (Args&&... args) : Base{ std::forward<Args>(args)... }, dimensions(sizeof...(Is)), dimSize( int(Is)... )
    {
        initWeights();

        Base::resize(Size);
    }



    template <std::size_t M = Size, help::EnableIfZero< M > = 0>
    Container () {}

    template <typename... Args, std::size_t M = Size, help::EnableIfZero< M > = 0,
              help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    Container (Args&&... args) : dimensions(sizeof...(args)), dimSize{ std::forward<Args>(args)... },
                                 weights(sizeof...(args))
    {
        initWeights();

        Base::resize(weights.front() * dimSize.front());
    }


    template <class... Args, std::size_t M = Size, help::EnableIfZero< M > = 0,
              help::EnableIfIterable< std::remove_reference_t< Args >... > = 0>
    Container (Args&&... args) : dimensions(0)
    {
        auto dummie = { (dimensions += std::forward<Args>(args).size(),
                         dimSize.insert( dimSize.end(), std::begin(std::forward<Args>(args)), 
                                                        std::end(std::forward<Args>(args)) ))... };

        weights.resize(dimensions);

        initWeights();

        Base::resize(weights.front() * dimSize.front());
    }


    template <typename U, typename V, std::size_t M = Size, help::EnableIfZero< M > = 0,
              help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    Container (U&& begin, V&& end) : dimensions( std::distance( std::forward<U>(begin), std::forward<V>(end) ) ), 
                                     dimSize( std::forward<U>(begin), std::forward<V>(end) ),
                                     weights( std::distance( std::forward<U>(begin), std::forward<V>(end) ) )
    {
        initWeights();

        Base::resize(weights.front() * dimSize.front());
    }

    template<std::size_t M = Size, help::EnableIfZero< M > = 0>
    Container (std::initializer_list<T> il) : Base( il ), dimensions(1), dimSize(1, il.size()), weights(1)
    {
        initWeights();
    }


	inline void initWeights ()
    {
    	weights.back() = 1;

        for(int i = dimensions - 2; i >= 0; --i)
            weights[i] = weights[i + 1] * dimSize[i + 1];
    }



    // template <typename... Inds, typename = help::EnableIfIntegral<Inds...>>
    // inline constexpr decltype(auto) slice (Inds... inds)
    // {
    //     return Slice<Container<T, Is...>>(*this, inds...);
    // }

    // template <typename... Inds, typename = help::EnableIfIntegral<Inds...>>
    // inline constexpr decltype(auto) slice (Inds... inds) const
    // {
    //     return Slice<Container<T, Is...>>(*this, inds...);
    // }



    // template <std::size_t N, std::size_t... Js>
    // inline constexpr decltype(auto) slice (const std::array<int, N>& v, std::index_sequence<Js...>)
    // {
    //     return slice(v[Js]...);
    // }

    // template <std::size_t N, std::size_t... Js>
    // inline constexpr decltype(auto) slice (const std::array<int, N>& v, std::index_sequence<Js...>) const
    // {
    //     return slice(v[Js]...);
    // }


    // template <std::size_t N>
    // inline constexpr decltype(auto) slice (const std::array<int, N>& v)
    // {
    //     return slice(v, std::make_index_sequence<N>());
    // }

    // template <std::size_t N>
    // inline constexpr decltype(auto) slice (const std::array<int, N>& v) const
    // {
    //     return slice(v, std::make_index_sequence<N>());
    // }



    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    inline const_reference operator () (Args&&... args) const
    {
        int pos = 0;

        auto it = weights.begin();

        const auto& dummie = { ( pos += *it++ * std::forward<Args>(args) )... };

        return this->operator[]( pos );
    }

    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    inline reference operator () (Args&&... args)
    {
        // return const_cast< std::decay_t< decltype( 
        //        static_cast< const Container& >( *this )( std::forward<Args>(args)... ) ) > & >(
        //        static_cast< const Container& >( *this )( std::forward<Args>(args)... ) );

        return const_cast< reference >( static_cast< const Container& >( *this )( std::forward<Args>(args)...) );
    }



    template <class... Args, help::EnableIfIterable< std::remove_reference_t< Args >... > = 0>
    inline const_reference operator () (Args&&... args) const
    {
        int pos = 0;

        auto it = weights.begin();

        const auto& dummie = { ( pos += std::inner_product( std::forward<Args>(args).begin(), 
                                                            std::forward<Args>(args).end(),
                                                            it, 0), 
                                                            it += std::forward<Args>(args).size() )... };
        return this->operator[]( pos );
    }

    template <typename... Args, help::EnableIfIterable< std::decay_t< Args >... > = 0 >
    inline reference operator () (Args&&... args)
    {
        // return const_cast< std::decay_t< decltype( 
        //        static_cast< const Container& >( *this )( std::forward<Args>(args)... ) ) > & >(
        //        static_cast< const Container& >( *this )( std::forward<Args>(args)... ) );

        return const_cast< reference >( static_cast< const Container& >( *this )( std::forward<Args>(args)...) );
    }



    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    inline const_reference operator () (U&& begin, V&& end) const
    {
        return this->operator[]( std::inner_product(std::forward<U>(begin), std::forward<V>(end),
                                                    weights.begin(), 0) );
    }

    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    inline reference operator () (U&& begin, V&& end)
    {
        // return const_cast< std::decay_t< decltype( 
        //        static_cast< const Container& >( *this )( std::forward<U>(begin), std::forward<V>(end) ) ) > & >(
        //        static_cast< const Container& >( *this )( std::forward<U>(begin), std::forward<V>(end) ) );

        return const_cast< reference >( static_cast< const Container& >( *this )( std::forward<U>(begin), 
                                                                                  std::forward<V>(end) ) );
    }



    template <typename U, help::EnableIfIntegral< std::decay_t< U > > = 0 >
    inline const_reference operator () (std::initializer_list<U> il) const
    {
        return this->operator()( il.begin(), il.end() );
    }

    template <typename U, help::EnableIfIntegral< std::decay_t< U > > = 0 >
    inline reference operator () (std::initializer_list<U> il)
    {
        // return const_cast< std::decay_t< decltype(
        //        static_cast< const Container& >( *this )( il ) ) > & >(
        //        static_cast< const Container& >( *this )( il ) );

        return const_cast< reference >( static_cast< const Container& >( *this )( il ) );
    }



    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    constexpr inline const_reference operator () (const std::tuple<Args...>& tup) const
    {
        return this->operator()( tup, std::make_index_sequence< sizeof...(Args) >() );
    }

    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    constexpr inline reference operator () (const std::tuple<Args...>& tup)
    {
        // return const_cast< std::decay_t< decltype(
        //        static_cast< const Container& >( *this )( tup ) ) > & >(
        //        static_cast< const Container& >( *this )( tup ) );

        return const_cast< reference >( static_cast< const Container& >( *this )( tup ) );
    }

    template <typename... Args, std::size_t... Js>
    constexpr inline const_reference operator () (const std::tuple<Args...>& tup, std::index_sequence<Js...>) const
    {
        return this->operator()( std::get< Js >( tup )... );
    }



    inline std::size_t size (int p) const { return dimSize[p]; }

    inline std::size_t size () const      { return Base::size(); }



private:

    int dimensions;

    Vector<int, bool(Size) * sizeof...(Is)> dimSize;

    Vector<int, bool(Size) * sizeof...(Is)> weights;

};


// //=========================================================================================================



// template <class Cnt>
// struct Slice
// {
//     template <typename... Inds, typename = help::EnableIfIntegral<Inds...>>
//     Slice (Cnt& c, Inds... inds) : c(c), dims(sizeof...(Inds))
//     {
//         first = std::inner_product(c.weights.begin(), c.weights.begin() + dims, help::makeArray(inds...).begin(), 0);

//         last = first + c.weights[dims-1];
//     }



//     inline decltype(auto) operator [] (int p)
//     {
//         return c[first + p];
//     }

//     inline decltype(auto) operator [] (int p) const
//     {
//         return c[first + p];
//     }



//     template <std::size_t N>
//     inline constexpr decltype(auto) operator () (const std::array<int, N>& v)
//     {
//         return c[(std::inner_product(v.begin(), v.end(), c.weights.begin() + dims, first))];
//     }

//     template <std::size_t N>
//     inline constexpr decltype(auto) operator () (const std::array<int, N>& v) const
//     {
//         return c[(std::inner_product(v.begin(), v.end(), c.weights.begin() + dims, first))];
//     }




//     template <typename... Indexes, typename = help::EnableIfIntegral<Indexes...>>
//     inline constexpr decltype(auto) operator () (Indexes... inds)
//     {
//         return this->operator()(std::array<int, sizeof...(Indexes)>{ inds... });
//     }

//     template <typename... Indexes, typename = help::EnableIfIntegral<Indexes...>>
//     inline constexpr decltype(auto) operator () (Indexes... inds) const
//     {
//         return this->operator()(std::array<int, sizeof...(Indexes)>{ inds... });
//     }



//     template <std::size_t N, std::size_t M, std::size_t... Js, std::size_t... Ks>
//     inline constexpr decltype(auto) operator () (const std::array<int, N>& v, const std::array<int, M>& u,
//                                                  std::index_sequence<Js...>, std::index_sequence<Ks...>)
//     {
//         return this->operator()(std::array<int, N + M>{ v[Js]..., u[Ks]... });
//     }

//     template <std::size_t N, std::size_t M, std::size_t... Js, std::size_t... Ks>
//     inline constexpr decltype(auto) operator () (const std::array<int, N>& v, const std::array<int, M>& u,
//                                                  std::index_sequence<Js...>, std::index_sequence<Ks...>) const
//     {
//         return this->operator()(std::array<int, N + M>{ v[Js]..., u[Ks]... });
//     }



//     template <std::size_t N, std::size_t M>
//     inline constexpr decltype(auto) operator () (const std::array<int, N>& v, const std::array<int, M>& u)
//     {
//         return this->operator()(v, u, std::make_index_sequence<N>(), std::make_index_sequence<M>());
//     }

//     template <std::size_t N, std::size_t M>
//     inline constexpr decltype(auto) operator () (const std::array<int, N>& v, const std::array<int, M>& u) const
//     {
//         return this->operator()(v, u, std::make_index_sequence<N>(), std::make_index_sequence<M>());
//     }



//     template <std::size_t N, typename... Indexes, typename = help::EnableIfIntegral<Indexes...>>
//     inline constexpr decltype(auto) operator () (const std::array<int, N>& v, Indexes... inds)
//     {
//         return this->operator()(v, std::array<int, sizeof...(Indexes)>{ inds...}, 
//                                    std::make_index_sequence<N>(), std::make_index_sequence<sizeof...(Indexes)>());
//     }

//     template <std::size_t N, typename... Indexes, typename = help::EnableIfIntegral<Indexes...>>
//     inline constexpr decltype(auto) operator () (const std::array<int, N>& v, Indexes... inds) const
//     {
//         return this->operator()(v, std::array<int, sizeof...(Indexes)>{ inds...}, 
//                                    std::make_index_sequence<N>(), std::make_index_sequence<sizeof...(Indexes)>());
//     }



//     inline constexpr auto size (int p) const { return c.size(dims + p); }

//     inline constexpr auto size () const { return last - first; }



//     inline decltype(auto) begin ()       { return c.begin() + first; }

//     inline decltype(auto) c_begin () const { return c.c_begin() + first; }

//     inline decltype(auto) end ()       { return c.begin() + last; }

//     inline decltype(auto) c_end () const { return c.c_begin() + last; }





//     Cnt& c;

//     std::size_t dims;
//     std::size_t first;
//     std::size_t last;

// };



// } // namespace cnt


}

#endif  // CNT_CONTAINER_H
