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

    cnt::Container<double> ks(vector<int>{2, 3}, list<double>{4, 5});

    ks(1, 2, 3, 4) = -25.3;

    DB(ks.size());
    DB(ks.size(0));
    DB(ks.size(1));
    DB(ks.size(2));
    DB(ks.size(3));
    




    return 0;
}
