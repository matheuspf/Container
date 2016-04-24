#include <bits/stdc++.h>

using namespace std;


constexpr std::size_t MAX = 1000 * sizeof(char);


template <bool B, class T>
using _if_ = typename enable_if<B, T>::type;


template <typename... Args>
struct Holder
{
    using type = Holder<Args...>;
};



template <std::size_t, class...>
struct Pick;

template <std::size_t N, class T, class... Args>
struct Pick<N, T, Args...> : public Pick<N-1, Args...> {};

template <class T, class... Args>
struct Pick<0, T, Args...>
{
	using type = T;
};

template <std::size_t N, class... Args>
using Pick_t = typename Pick<N, Args...>::type;



template <int...>
struct Mult;

template <int x, int... Ints>
struct Mult<x, Ints...>
{
	static constexpr int val = x > 0 ? x * Mult<Ints...>::val : 0;
};

template <int x>
struct Mult<x>
{
	static constexpr int val = x > 0 ? x : 0;
};

template <>
struct Mult<>
{
	static constexpr int val = 0;
};


template <int... Ints>
struct Test
{
    static constexpr bool val = sizeof...(Ints) > 0 ? bool(Mult<Ints...>::val) : false;
};


template <typename...>
struct And;

template<>
struct And<> : public std::true_type {};

template <typename T>
struct And<T> : public T {};

template <typename T, typename U>
struct And<T, U> : public std::conditional<T::value, U, T>::type {};

template <typename T, typename U, typename V, typename... Args>
struct And<T, U, V, Args...> : public std::conditional<T::value, And<U, V, Args...>, T>::type {};





template <typename T, std::size_t... Ints>
struct Container : public std::conditional<Test<Ints...>::val,        /*((sizeof...(Ints) == 0) || (Mult<Ints...>::val > MAX)),*/
                                           array<T, Mult<Ints...>::val>, vector<T>>::type
{
    using Base = typename std::conditional<Test<Ints...>::val,         /*((sizeof...(Ints) == 0) || (Mult<Ints...>::val > MAX)),*/
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
std::size_t Container<T, Ints...>::dimensions = sizeof...(Ints) ? sizeof...(Ints) : 1;






int main ()
{
    //Container<int, 3, 5, 4> c;
    Container<int> c = Container<int>(3, 5, 4);

    for(int i = 0, l=0; i < c.size(0); ++i)
        for(int j = 0; j < c.size(1); ++j)
            for(int k = 0; k < c.size(2); ++k, ++l)
                c(i, j, k) = l;

    for(auto x : c)
        cout << x << endl;

    return 0;
}
