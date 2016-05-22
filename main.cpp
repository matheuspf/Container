#include <iostream>
#include "Container.h"

using namespace std;


template <class> struct Print;



template <int... Is>
void Foo (const cnt::Container<int, Is...>& c)
{
    for(int i = 0; i < c.size(0); ++i)
        for(int j = 0; j < c.size(1); ++j)
            for(int k = 0; k < c.size(2); ++k)
                cout << c(i, j, k) << endl;
}


template <size_t N = 0>
auto Goo (int n = 0)
{
    cnt::Vector<int, N> v(n);

    return v;
}



int main ()
{
    /*cnt::Container<int, 7, 5, 3> v;     // Creates a std::array
    cnt::Container<int> u(2, 4, 6);     // Creates a std::vector

    std::generate(v.begin(), v.end(), []{ static int x = 0; return x++; });
    std::generate(u.begin(), u.end(), []{ static int x = 0; return x++; });

    Foo(v);
    Foo(u);*/


    cout << Goo().size() << endl;

    //Print<typename decay_t<decltype(Goo(5))>::Base>();


    return 0;
}
