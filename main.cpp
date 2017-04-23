#include <bits/stdc++.h>
#define DB(...) std::cout << __VA_ARGS__ << "\n" << std::flush

//#include "Container.h"
//#include "Matrix.h"
#include "Container.h"

using namespace std;



template <class> struct Prt;


template <typename T, size_t... Is>
ostream& operator << (ostream& out, const cnt::Container<T, Is...>& v)
{
    for(const auto& x : v)
        out << x << " ";

    return out << "\n";
}



int main ()
{
    cnt::Container<int> v(10, 20, 30);

    v(5, 18, 23) = 15;

    int ar[] = {5, 18, 23};

    DB(v(5, 18, 23));



    return 0;
}
