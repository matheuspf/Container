#ifndef CONTAINER_H
#define CONTAINER_H

#include <type_traits>
#include <tuple>
#include <vector>
#include <array>
#include <algorithm>

namespace cnt
{

namespace impl
{

// Stack allocation must not exceed this limit

constexpr std::size_t maxSize = 1000 * sizeof(char);



// Helper for variadic template

template <typename... Args>
struct Holder { using Type = Holder<Args...>; };



// Picks the Nth type

template <std::size_t N, typename... Args>
struct Pick
{
    static_assert(N < sizeof...(Args), "Position to pick exceeds maximum number of elements");

    using Type = decltype(std::get<N>(std::declval<std::tuple<Args...>>()));
};

template <std::size_t N, typename... Args>
using Pick_t = typename Pick<N, Args...>::type;      // Helper for clearer sintax

//---------------------------------------------------------------------------

// Helper for creating constexpr functions for variadic arguments

#define OPERATION(NAME, BIN_OP) \
\
constexpr inline auto NAME () { return 0; }  \
\
template <typename T, typename U>   \
constexpr inline auto NAME (T&& t, U&& u) { return BIN_OP(std::forward<T>(t), std::forward<U>(u)); }    \
\
template <typename T, typename... Args> \
constexpr inline auto NAME (T&& t, Args&&... args) { return BIN_OP(std::forward<T>(t), NAME(std::forward<Args>(args)...)); }


#define MULT(x, y) (x) * (y)    // Because there is no way to pass a constexpr function as argument, this is necessary

OPERATION(mult, MULT);      // Declares a function called 'mult' with the operation MULT
                            // This could be done too: 'OPERATION(mult, [](auto&& x, auto&& y){ ... })',
                            // but it is no more a constexpr function


//-----------------------------------------------------------------------------------

// Test truthness of variadic set of bool arguments

template <bool...>
struct And;

template <bool B1, bool... Bs>
struct And<B1, Bs...> : public And<Bs...> {};

template <bool... Bs>
struct And<false, Bs...> : public std::false_type {};

template <>
struct And<true> : public std::true_type {};




// Choses Nth variable at compile time

template <std::size_t N, typename... Args>
inline constexpr decltype(auto) choose (Args&&... args)
{
    static_assert(N < sizeof...(Args), "Position required exceeds the number of arguments");

    return std::get<N>(std::tuple<Args...>(std::forward<Args>(args)...));
}




// Helper for testing if the selected base will be std::array or a std::vector

template <int N>
constexpr bool test = (N > 0) ? true : false;
//constexpr bool test = (N > 0) ? (N < maxSize) : false;


}   // namespace impl


//==============================================================================================



// Main class
//
// cnt::Vector inherits from a std::vector if it is not supplied with compile time size
// or if the given compile time size is greater than the value defined at cnt::impl::maxSize.
//
// Otherwise it inherits from std::array

template <typename T, int N = 0>
struct Vector : public std::conditional_t<impl::test<N>, std::array<T, N>, std::vector<T>>
{
    using Base = std::conditional_t<impl::test<N>, std::array<T, N>, std::vector<T>>;

    using Base::Base;   // Inherits all constructors of std::vector, given that std::array has no user defined constructors


    template <class U = Base>
    static constexpr bool isArray = std::is_same<std::array<T, N>, U>::value;

    template <class U = Base>
    static constexpr bool isVector = std::is_same<std::vector<T>, U>::value;


    Vector () : Base() {}


    // Constructor needed for initializing the class as: cnt::Vector<T, N> v = {...}, if inheriting from std::array

    template <typename U = Base, typename = std::enable_if_t<isArray<U>>>
    Vector (std::initializer_list<T> in) { std::copy(in.begin(), in.end(), this->begin()); }


    template <typename U = Base, typename = std::enable_if_t<isArray<U>>>
    Vector (std::size_t M) {}

};


//=========================================================================================================


template <typename T, int Rows = 0, int Cols = 0>
class Matrix : public Vector<T, Rows * Cols>
{
public:

    using Base = Vector<T, Rows * Cols>;

    using Base::Base;


    using ValueType = T;




    template <typename U = Base, typename = std::enable_if_t<U::isArray>>
    Matrix () : Base(), rows_(Rows), cols_(Cols) {}

    template <typename U = Base, typename = std::enable_if_t<U::isVector>>
    Matrix (int rows, int cols) : Base(rows * cols), rows_(rows), cols_(cols) {}

    //template <typename U, typename V = Base, typename = std::enable_if_t<V::isArray>>
    //Matrix (std::initializer_list<T> in) : Base(in), rows_(Rows), cols_(Cols) {}


    inline int rows () const { return rows_; }

    inline int cols () const { return cols_; }



    inline constexpr decltype(auto) operator () (int i, int j)
    {
        return this->operator[](i * cols_ + j);
    }

    inline constexpr decltype(auto) operator () (int i, int j) const
    {
        return this->operator[](i * cols_ + j);
    }



    template <typename U>
    inline auto create (U, int r, int c)
    {
        return impl::choose<std::size_t(Base::isVector)>(Matrix<U, Rows, Cols>(), Matrix<U>(rows_, cols_));
    }



private:

    int rows_;
    int cols_;

};




//=========================================================================================================



template <typename T, int... Is>
class Container : public Vector<T, impl::mult(Is...)>
{
public:

    using Base = Vector<T, impl::mult(Is...)>;

    using Base::Base;




    template <typename U = Base, typename = std::enable_if_t<U::isArray>>
	Container () : Base(), dimensions(sizeof...(Is)), dimSize({Is...}), weights()
	{
	    initWeights();
	}

    template <typename U = Base, typename... Args>
    Container (Args... args) : Base(impl::mult(args...)), dimensions(sizeof...(args)),
                                    dimSize({args...}), weights(dimensions)
    {

        //std::enable_if_t<impl::And<U::isVector, std::is_integral<Args>::value...>::value>();

        static_assert(impl::And<U::isVector, std::is_integral<Args>::value...>::value, "Container already initialized");

        initWeights();
    }


	inline void initWeights ()
    {
    	weights.back() = 1;

        for(int i = dimensions - 2; i >= 0; --i)
            weights[i] = weights[i + 1] * dimSize[i + 1];
    }



    template <typename... Indexes>
    inline constexpr decltype(auto) operator () (Indexes... inds)
    {
        static_assert(impl::And<std::is_integral<Indexes>::value...>::value, "Indexes must be integers");

        const std::array<int, sizeof...(Indexes)> v = { inds... };

        return this->operator[](std::inner_product(v.begin(), v.end(), weights.begin(), 0));
    }

    template <typename... Indexes>
    inline constexpr decltype(auto) operator () (Indexes... inds) const
    {
        static_assert(impl::And<std::is_integral<Indexes>::value...>::value, "Indexes must be integers");

        const std::array<int, sizeof...(Indexes)> v = { inds... };

        return this->operator[](std::inner_product(v.begin(), v.end(), weights.begin(), 0));
    }


    inline constexpr decltype(auto) operator () (int i, int j)
    {
        return this->operator[](i * dimSize[1] + j);
    }

    inline constexpr decltype(auto) operator () (int i, int j) const
    {
        return this->operator[](i * dimSize[1] + j);
    }


    inline constexpr auto size (int p) const { return dimSize[p]; }

    inline constexpr auto size () const { return Base::size(); }



private:

    int dimensions;

    Vector<int, sizeof...(Is)> dimSize;

    Vector<int, sizeof...(Is)> weights;

};


//=========================================================================================================



/*template <typename T, std::size_t... Ints>
struct Container : public std::conditional<Test<Ints...>::val,
                                           array<T, Mult<Ints...>::val>, vector<T>>::type
{
    using Base = typename std::conditional<Test<Ints...>::val,
                                           array<T, Mult<Ints...>::val>, vector<T>>::type;

    using reference = typename Base::reference;


    static constexpr bool vectorBase = std::is_same<Base, vector<T>>::value;
    static constexpr bool arrayBase  = !vectorBase;


    static constexpr std::size_t staticSize =  (arrayBase || (Mult<Ints...>::val > MAX)) ? Mult<Ints...>::val : 1;

    static std::size_t dimensions;



    template <class Type, typename U>
	using _if_vector = typename enable_if<is_same<Type, vector<T>>::value, U>::type;

	template <class Type, typename U>
	using _if_array = typename enable_if<is_same<Type, array<T, staticSize>>::value, U>::type;



    template <typename U, typename... Args>
    struct _if_integral_;

    template <typename R, typename U, typename... Args>
    struct _if_integral_<R, U, Args...>
    {
        using type = _if_<And<std::is_same<std::decay_t<U>, std::decay_t<Args>>...,
                         std::is_integral<Args>...>::value, R>;
    };

    template <typename R, typename... Args>
    using _if_integral = typename _if_integral_<R, Args...>::type;


    using indexType = typename std::conditional<vectorBase, std::vector<std::size_t>, std::array<std::size_t, sizeof...(Ints)>>::type;

    indexType sizeDim;
    indexType weights;


	Container ()
	{
	    Init<Base>();

        sizeDim = { Ints... };

		InitWeights();
	}

    template <typename U, typename... Args>
    Container (U u, Args... args)
    {
        static_assert(sizeof...(Ints) == 0, "Already initialized");

        _if_vector<Base, void>();
        _if_integral<void, U, Args...>();

        dimensions = sizeof...(args) + 1;

        sizeDim = std::vector<std::size_t>{ std::size_t(u), std::size_t(args)... };

        weights = vector<std::size_t>(dimensions);

        std::size_t vectorSize = 1;

        for(auto x : sizeDim)
            vectorSize *= x;

        this->resize(vectorSize);

        InitWeights();
    }



    template <class U>
	_if_vector<U, void> Init ()
	{
		if(staticSize > 1)
		    this->resize(staticSize);
	}

	template <class U>
	_if_array<U, void> Init ()
	{
		this->fill(T());
	}


	inline void InitWeights ()
    {
    	weights.back() = 1;

        for(int i = dimensions - 2; i >= 0; --i)
            weights[i] = weights[i + 1] * sizeDim[i + 1];
    }




    template <typename... Args>
    _if_integral<reference, Args...> operator () (Args... args)
    {
        //static_assert(dimensions == (sizeof...(Args)), "Wrong number of dimensions");

        const std::array<Pick_t<0, Args...>, sizeof...(Args)> v = { args... };

        return this->operator[](std::inner_product(v.begin(), v.end(), weights.begin(), 0));
    }


    inline typename Base::reference operator () (std::size_t i, std::size_t j)
    {
        //static_assert(dimensions == 2, "Wrong number of dimensions");

        return this->operator[](i * sizeDim[1] + j);
    }


    inline constexpr size (int p)
    {
        return sizeDim[p];
    }

    inline constexpr size ()
    {
        return Base::size();
    }
};

template <typename T, std::size_t... Ints>
std::size_t Container<T, Ints...>::dimensions = sizeof...(Ints) ? sizeof...(Ints) : 1;*/


} // namespace cnt


#endif  // CONTAINER_H
