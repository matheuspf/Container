#include <bits/stdc++.h>
#define DB(...) std::cout << __VA_ARGS__ << "\n" << std::flush

//#include "Container.h"
//#include "Matrix.h"
#include "include/Container/Container.h"

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
    // cnt::Container<int> v(2, 3, 4);

    // iota(v.begin(), v.end(), 0);

    // const vector<int>& u = v;

    // for(auto x : u)
    // 	DB(x);

    //cnt::Container<double> ks(7, 3, 6, 2);
    cnt::Container<double, 7, 3, 6, 2> ks;

    auto slc = ks.slice();

    slc(5, 2, 4, 1) = 23;

    cout << ks[213] << "\n";



    return 0;
}
