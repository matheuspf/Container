/** \file Slice.h
  *
  * A class to take a 'slice' of a 'Container' class.
*/

#ifndef CNT_SLICE_H
#define CNT_SLICE_H

#include <tuple>
#include <algorithm>
#include <cmath>
#include <numeric>


namespace cnt
{

    /** This class is just a proxy to access chosen dimensions of a container.
      * The only storage is are the positions to access and a reference to
      * the container that created this slice. There are a bunch of examples
      * of how to use it in 'examples/SliceExamples.cpp'.
    */
	template <class Cnt>
	class Slice
	{
	public:


    /** Some type definition
    */
    //@{
    using Base = std::decay_t<Cnt>;


    using value_type = typename Base::value_type;

    using reference = typename Base::reference;

    using const_reference = typename Base::const_reference;
    //@}



// --------------------------------- Constructors ---------------------------------------------- //


    /**
    */
    //@{
    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    Slice (Cnt& c, const Args&... args) : c(c), dims(sizeof...(Args) ? sizeof...(Args) : c.dimensions), first(0)
    {
        auto iter = c.weights.begin();

        const auto& dummie = { (first += *iter++ * args)..., 0 };

        last = first + (sizeof...(Args) ? c.weights[dims-1] : c.size());
    }


    template <typename... Args, help::EnableIfIterable< std::remove_reference_t< Args >... > = 0 >
    Slice (Cnt& c, const Args&... args) : c(c), dims(sizeof...(Args)), first(0)
    {
        auto iter = c.weights.begin();

        const auto& dummie = { (first += std::inner_product(args.begin(), args.end(), iter, 0), 
                                                            iter += args.size())... };
        last = first + c.weights[dims-1];
    }


    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    Slice (Cnt& c, const U& begin, const U& end) : c(c), dims(std::distance(begin, end)), first(0)
    {
        first = std::inner_product(begin, end, c.weights.begin(), 0);

        last = first + c.weights[dims-1];
    }
    //@}




// ------------------------------- Access - operator() --------------------------------------------- //


    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    const_reference operator () (const Args&... args) const
    {
        int pos = first;

        auto it = c.weights.begin() + dims;

        const auto& dummie = { (pos += *it++ * args)... };

        return c[ pos ];
    }

    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    reference operator () (const Args&... args)
    {
        return const_cast<reference>(static_cast<const Slice&>(*this)(args...));
    }



    template <class... Args, help::EnableIfIterable< std::remove_reference_t< Args >... > = 0>
    const_reference operator () (const Args&... args) const
    {
        int pos = first;

        auto it = c.weights.begin() + dims;

        const auto& dummie = { (pos += std::inner_product(args.begin(), args.end(), it, 0), 
                                                          it += args.size())... };
        return c[pos];
    }

    template <typename... Args, help::EnableIfIterable< std::decay_t< Args >... > = 0 >
    reference operator () (const Args&... args)
    {
        return const_cast<reference>(static_cast<const Slice&>(*this)(args...));
    }









    template <typename U, help::EnableIfIntIter< std::decay_t< U >> = 0>
    const_reference operator () (const U& begin) const
    {
        return this->operator[](std::inner_product(c.weights.begin() + dims, c.weights.end(), begin(), first));
    }

    template <typename U, help::EnableIfIntIter< std::decay_t< U > > = 0>
    reference operator () (const U& begin)
    {
        return const_cast<reference>(static_cast<const Slice&>(*this)(begin));
    }





    /** Access for a 'std::initializer_list' of integral type. You can then access a
      * 'Container' as easily as: 'Container<int, 2, 3, 4> c;  c({1, 2, 3}) = 10'.
      *
      * \param[in] il Initializer list defining the position to access
    */
    //@{
    template <typename U, help::EnableIfIntegral< std::decay_t< U > > = 0 >
    const_reference operator () (std::initializer_list<U> il) const
    {
        return this->operator[](std::inner_product(c.weights.begin() + dims, weights.end(), il.begin(), 0));
    }

    template <typename U, help::EnableIfIntegral< std::decay_t< U > > = 0 >
    reference operator () (std::initializer_list<U> il)
    {
        return const_cast<reference>(static_cast<const Container&>(*this)(il));
    }
    //@}




    /** Access for a 'std::tuple' of integral types. It might be useful in some
      * situations. The order is the same of all the rest, just a little messier.
      *
      * \param[in] tup Tuple of integral types to perform access
    */
    //@{
    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    constexpr const_reference operator () (const std::tuple<Args...>& tup) const
    {
        return this->operator()(tup, std::make_index_sequence<sizeof...(Args)>());
    }

    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    constexpr reference operator () (const std::tuple<Args...>& tup)
    {
        return const_cast<reference>(static_cast<const Container&>(*this)(tup));
    }

    template <typename... Args, std::size_t... Js>
    constexpr const_reference operator () (const std::tuple<Args...>& tup, std::index_sequence<Js...>) const
    {
        return this->operator()(std::get<Js>(tup)...);
    }
    //@}






    decltype(auto) operator [] (int p) const
    {
        return c[first + p];
    }

    decltype(auto) operator [] (int p)
    {
        return c[first + p];
    }




    auto size (int p) const { return c.size(dims + p); }

    auto size () const      { return last - first; }



    decltype(auto) begin ()        { return c.begin() + first; }

    decltype(auto) cbegin () const { return c.cbegin() + first; }

    decltype(auto) end ()          { return c.begin() + last; }

    decltype(auto) cend () const   { return c.cbegin() + last; }



private:

    Cnt& c;

    int dims;
    int first;
    int last;

};



} // namespace cnt



#endif // CNT_SLICE_H