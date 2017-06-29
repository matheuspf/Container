/** \file Container.h
  *
  * An interface to easily access multidimensional data, having total 
  * compatibility with STL algorithms and containers.
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





    /** These functions get either a integral type or a iterable of integrals
      * and multiply each element with the iterator 'iter', given by a position
      * in the variable 'weights'. The iterator is incremented, and the value
      * of the multiplication is returned.
      *
      * \note Here, I used a universal reference for the iterator type 'Iter',
      *					so I can use a dummie in the place of the iterator. The
      *					function then can be called with a single argument, to 
      *					allow SFINAE. See the 'operator()' function bellow.
      *
      * \param[in] u Either a integral type or a iterable of integrals
      * \param[in] iter A reference to a iterator. 
      * \return res Result after multiplication(s).
    */
    //@
    template <typename U, typename Iter = int*, help::EnableIfIntegral<std::decay_t<U>> = 0>
    static std::size_t increment (U u, Iter&& iter = 0)
    {
    	return *iter++ * u;
    }


    template <typename U, typename Iter = int*, help::EnableIfIterable<std::decay_t<U>> = 0>
    static std::size_t increment (const U& u, Iter&& iter = 0)
    {
    	std::size_t res = 0;

    	for(auto x : u)
    		res += *iter++ * x;

    	return res;
    }
    //@}



    /** This access operator lets you pass variadic arguments being either integral
      * types or iterables of integral types. The order of the arguments determines
      * the position in each dimension. For example: 'Container<int> c(4, 1, 3);
      * c(vector<long>{1, 0}, 2);' will give you the positions 1, 0 and 2 in the
      * first, second and third dimension, respectivelly.
      *
      * \note The 'Dummie' template stuff is a trick to only allow the call if
      * the arguments are either integral or iterable of integrals types.
      *
      * \param[in] args Either integral types or a iterables of integrals
    */
    //@{
    template <typename... Args,
    		  typename Dummie = std::index_sequence<decltype(increment(std::declval<Args>())){}...>>
    const_reference operator () (const Args&... args) const
    {
        std::size_t pos = 0;

        auto iter = weights.begin();

        const auto& dummie = { (pos += increment(args, iter), int{})... };

        return this->operator[](pos);
    }

    template <typename... Args,
    		  typename Dummie = std::index_sequence<decltype(increment(std::declval<Args>())){}...>>
    reference operator () (const Args&... args)
    {
    	std::size_t pos = 0;

        auto iter = weights.begin();

        const auto& dummie = { (pos += increment(args, iter), int{})... };

        return this->operator[](pos);
        //return const_cast<reference>(static_cast<const Container&>(*this)(args...));
    }
    //@}


    /** Access operator for an iterator defined by the starting position 'begin'.
      * The dimensions to access are defined by the order of the integral elements
      * of the iterator.
      *
      * \param[in] begin Initial position of the iterator/pointer of integral types
    */
    //@{
    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U > > = 0>
    const_reference operator () (const U& begin) const
    {
        return this->operator[](std::inner_product(weights.begin(), weights.end(), begin, 0));
    }

    template <typename U, typename V, help::EnableIfIntIter< std::decay_t< U > > = 0>
    reference operator () (const U& begin)
    {
        return const_cast<reference>(static_cast<const Container&>(*this)(begin));
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
        return this->operator[](std::inner_product(weights.begin(), weights.end(), il.begin(), 0));
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
