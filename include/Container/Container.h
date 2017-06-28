/** \file Container.h
  *
  * A class to easily access multidimensional data, having compatible
  * interface with STL algorithms.
*/

#ifndef CNT_CONTAINER_H
#define CNT_CONTAINER_H

#include <tuple>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "Vector.h"
#include "Slice.h"



// #include <iostream>
// #define DB(...) std::cout << __VA_ARGS__ << "\n" << std::flush


namespace cnt
{

/** Class to easily create and manipulate multidimensional data. Interacts easily
  * with STL algorithms and can be either statically or dinamically allocated.
  *
  * \tparam T The Containers type
  * \tparam Is The compile time size of each dimension. The total size is the 
  *         multiplication of these sizes. See the 'Vector' class.
*/
template <typename T, std::size_t... Is>
class Container : public Vector<T, help::multiply(Is...)>
{
public:


    /** Some type definitions */
    //@{
    using Base = Vector<T, help::multiply(Is...)>;


    using value_type = typename Base::value_type;

    using reference = typename Base::reference;

    using const_reference = typename Base::const_reference;


    using Base::Size;   /// It is actually equal to 'help::multiply(Is...)'
    //@}



    template <class> friend class Slice;     /// Friend definition for the 'Slice' class




// --------------------------------- Constructors ---------------------------------------------- //


    /** Constructor defined when inheriting from 'std::array'. Simulates 'std::array' list 
      * initialization. The number of dimensions is given by 'Is'.
      *
      * \params[in] args Variadic arguments. 'Vector' checks if they are of type 'T'.
    */
    template <typename... Args, std::size_t M = Size, help::EnableIfArray< M > = 0>
    Container (Args&&... args) : Base{ std::forward<Args>(args)... }, 
                                 dimensions(sizeof...(Is)),
                                 dimSize( Is... )
    {
        initWeights();
    }


    /** Same as above, but now for a 'Container' inheriting from 'std::vector' with 'Size'
      * greater than the maximum stack allocation size. In this case, we must resize to
      * 'Size' after intiallizing with 'args'.
      *
      * \params[in] args Variadic arguments. 'Vector' checks if they are of type 'T'.
    */
    template <typename... Args, std::size_t M = Size, std::enable_if_t<( M >= help::maxSize ), int > = 0>
    Container (Args&&... args) : Base{std::forward<Args>(args)...}, 
                                 dimensions(sizeof...(Is)),
                                 dimSize(Is...)
    {
        initWeights();

        Base::resize(Size);
    }


    /// Empty constructor for the case where 'Size' is 0 (no compile time size is given)
    template <std::size_t M = Size, help::EnableIfZero< M > = 0>
    Container () {}


    /** Constructor for the case when 'Size' is 0 (inheriting from 'std::vector').
      * This time the parameters are integral values that define the size of each
      * dimension. So, '3, 4, 7' would gives us a 'Container' with thre dimensions
      * with sizes 3, 4 and 7, respectivelly.
      *
      * \param[in] args Variadic integral types defining the size of each dimension.
                        Only integral types are accepted.
    */
    template <typename... Args, std::size_t M = Size, help::EnableIfZero< M > = 0,
              help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    Container (Args... args) : dimensions(sizeof...(args)), 
                               dimSize{std::size_t(args)...},
                               weights(sizeof...(args))
    {
        initWeights();


        /// Total size is equal to this multiplication. See the 'initWeights' function.
        Base::resize(weights.front() * dimSize.front());
    }



    /** Another constructor defined when 'Size' is 0. Each element is an iterable type
      * containing integral elements, that is, has both 'std::begin' and 'std::end' defined.
      * The number of dimensions is the sum of the sizes of the iterables. For example, if you pass 
      * 'vector<int>{2, 3}, list<long>{4, 5}', a 'Container' with 4 dimensions of sizes 2, 3, 4 and 
      * 5 will be created. Only iterables of integral types are accepted.
      *
      * \param[in] args Variadic iterable types of integrals
    */
    template <class... Args, std::size_t M = Size, help::EnableIfZero< M > = 0,
              help::EnableIfIterable< std::remove_reference_t< Args >... > = 0>
    Container (const Args&... args) : dimensions(0)
    {
    	/** For each iterable we increase the number of dimensions (sum of args.size() for each
    	  * iterable) and insert the dimensions at the end of 'dimSize' 'Vector'.
    	*/
        auto dummie = { (dimensions += args.size(),
                         dimSize.insert(dimSize.end(), std::begin(args), std::end(args)))... };

        weights.resize(dimensions);

        initWeights();

        /// Total size is equal to this multiplication. See the 'initWeights' function.
        Base::resize(weights.front() * dimSize.front());
    }


    /** One more constructor defined when 'Size' is 0. In this case, the argument is the starting
      * and ending positions of a iterator. You can also use pointers. If you have for example
      * int v[3] = {4, 1, 7}, and pass it like: 'Container<double> c(v, v+3)', a 'Container' with
      * dimensions of sizes 4, 1 and 7 will be created.
      *
      * \param[in] begin Initial position of the iterator/pointer of integral types
      * \param[in] end Final position of the iterator/pointer of integral types
    */
    template <typename U, typename V, std::size_t M = Size, help::EnableIfZero< M > = 0,
              help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    Container (const U& begin, const V& end) : dimensions(std::distance(begin, end)), 
                                               dimSize(begin, end),
                                               weights(std::distance(begin, end))
    {
        initWeights();

        /// Total size is equal to this multiplication. See the 'initWeights' function.
        Base::resize(weights.front() * dimSize.front());
    }


    /** A constructor taking a 'std::initializer_list', so you can also construct a
      * container with a single dimension, like that: 'Container<int> c{1, 2, 3}'. The
      * 'Container' in this case will have a single dimension with three elements.
      *
      * \param[in] il Initializer list of type 'T' (same as 'Container')
    */
    template<std::size_t M = Size, help::EnableIfZero< M > = 0>
    Container (std::initializer_list<T> il) : Base( il ), dimensions(1), dimSize(1, il.size()), weights(1)
    {
        initWeights();
    }


    /** This function is called from all constructors. It will initialize the 'weights' to 
      * access a given position in the continuous array created either by 'std::vector' or
      * 'std::array' by performing an inner product, given the size of each dimension.
    */
  	void initWeights ()
    {
	    weights.back() = 1;

	    for(int i = dimensions - 2; i >= 0; --i)
	        weights[i] = weights[i + 1] * dimSize[i + 1];
    }




// ------------------------------- Access - operator() --------------------------------------------- //


    /** Access operator for variadic arguments of integral types. The elements define
      * the position of each dimension in the order they are passed, just as you
      * expect.
      *
      * \param[int] args Variadic integral types defining the position to access in
      					 each dimension.
    */
    //@{
    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    const_reference operator () (const Args&... args) const
    {
        int pos = 0;

        auto it = weights.begin();

        const auto& dummie = { (pos += *it++ * args)... };

        return this->operator[](pos);
    }

    template <typename... Args, help::EnableIfIntegral< std::decay_t< Args >... > = 0 >
    reference operator () (const Args&... args)
    {
        return const_cast<reference>(static_cast<const Container&>(*this)(args...));
    }
    //@}


    /** Access operator for variadic iterable arguments of integral types. The elements
      * in the iterable define the dimension to access in the given order.
      *
      * \param[int] args Variadic iterable arguments defining the position to access in
      					 each dimension.
    */
    //@{
    template <class... Args, help::EnableIfIterable< std::remove_reference_t< Args >... > = 0>
    const_reference operator () (const Args&... args) const
    {
        int pos = 0;

        auto iter = weights.begin();

        const auto& dummie = { (pos += std::inner_product(args.begin(), args.end(), iter, 0), 
                                                          iter += args.size())... };

        return this->operator[](pos);
    }

    template <typename... Args, help::EnableIfIterable< std::decay_t< Args >... > = 0 >
    reference operator () (const Args&... args)
    {
        return const_cast<reference>(static_cast<const Container&>(*this)(args...));
    }
    //@}



    /** Access operator for an iterator defined by the starting position 'begin' and ending
      * position 'end'. The dimensions to access are defined in order by the integral elements
      * of the iterator.
      *
      * \param[in] begin Initial position of the iterator/pointer of integral types
      * \param[in] end Final position of the iterator/pointer of integral types
    */
    //@{
    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    const_reference operator () (const U& begin, const V& end) const
    {
        return this->operator[](std::inner_product(begin, end, weights.begin(), 0));
    }

    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U >, std::decay_t< V > > = 0>
    reference operator () (const U& begin, const V& end)
    {
        return const_cast<reference>(static_cast<const Container&>(*this)(begin, end));
    }
    //@}


    /** Access for a 'std::initializer_list' of integral type. You can then access a
      * 'Container' as easily as: 'Container<int, 2, 3, 4> c;  c({1, 2, 3}) = 10'.
      *
      * \param[in] il Initializer list defining the position to access
    */
    //@{
    template <typename U, help::EnableIfIntegral< std::decay_t< U > > = 0 >
    const_reference operator () (std::initializer_list<U> il) const
    {
        return this->operator()(il.begin(), il.end());
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



    /// Size of each dimension
    constexpr std::size_t size (int p) const { return dimSize[p]; }

    /// Total size
    constexpr std::size_t size ()      const { return Base::size(); }




//---------------------------------- Slice ---------------------------------------------- //
    


    /** As the name says, it takes a 'Slice' of the container. If you use for example:
      * 'Container<int, 2, 3, 4> c;   auto slc = c.slice(1);', the variable 'slc' will
      * a proxy to access the container 'c', having two dimensions and starting from
      * position 1 from the first dimension. For more, see the examples.
      *
      * \param[in] args Variadic integral arguments defining the dimensions to 'take a slice'.
    */
    //@{
    template <typename... Args>
    auto slice (const Args&... args) const
    {
        return Slice<const Container>(*this, args...);
    }

    template <typename... Args>
    auto slice (const Args&... args)
    {
        return Slice<Container>(*this, args...);
    }
    //@}



private:


	/// Number of dimensions
    int dimensions;


    /// The size of each dimension
    Vector<std::size_t, bool(Size) * sizeof...(Is)> dimSize;


    /// The weights to access given the position and sizes of the dimensions
    Vector<std::size_t, bool(Size) * sizeof...(Is)> weights;

};




// } // namespace cnt


}

#endif  // CNT_CONTAINER_H
