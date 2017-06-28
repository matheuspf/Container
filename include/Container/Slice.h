#ifndef CNT_SLICE_H
#define CNT_SLICE_H

#include <tuple>
#include <algorithm>
#include <cmath>
#include <numeric>


namespace cnt
{

	template <class Cnt>
	class Slice
	{
	public:

    using Base = std::decay_t<Cnt>;


    using value_type = typename Base::value_type;

    using reference = typename Base::reference;

    using const_reference = typename Base::const_reference;



// --------------------------------- Constructors ---------------------------------------------- //


    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    Slice (Cnt& c, const Args&... args) : c(c), dims(sizeof...(Args) ? sizeof...(Args) : c.dimensions), first(0)
    {
        auto it = c.weights.begin();

        const auto& dummie = { (first += *it++ * args)..., 0 };

        last = first + sizeof...(Args) ? c.weights[dims-1] : c.size();
    }


    template <typename... Args, help::EnableIfIterable< std::remove_reference_t< Args >... > = 0 >
    Slice (Cnt& c, const Args&... args) : c(c), dims(sizeof...(Args)), first(0)
    {
        auto it = c.weights.begin();

        const auto& dummie = { (first += std::inner_product(args.begin(), args.end(), it, 0), 
                                                             it += args.size())... };
        last = first + c.weights[dims-1];
    }


    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    Slice (Cnt& c, const U& begin, const U& end) : c(c), dims(std::distance(begin, end)), first(0)
    {
        first = std::inner_product(begin, end, c.weights.begin(), 0);

        last = first + c.weights[dims-1];
    }




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



    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    const_reference operator () (const U& begin, const V& end) const
    {
        return this->operator[](std::inner_product(begin, end, c.weights.begin() + dims, first));
    }

    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    reference operator () (const U& begin, const V& end)
    {
        return const_cast<reference>(static_cast<const Slice&>(*this)(begin, end));
    }



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