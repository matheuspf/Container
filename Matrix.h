#ifndef CNT_MATRIX_H
#define CNT_MATRIX_H


#include "Vector.h"


namespace cnt
{


// Simple Matrix class based on cnt::Vector

template <typename T, int Rows = 0, int Cols = 0>
class Matrix : public Vector<T, Rows * Cols>
{
public:

    using Base = Vector<T, Rows * Cols>;

    using Base::Base;
    using Base::Size;


    template <std::size_t M = Size, help::EnableIfVector<M> = 0>
    Matrix (int rows_ = 0, int cols_ = 0) : 
    Base(Size ? Size : rows_ * cols_), rows_(Rows ? Rows : rows_), cols_(Cols ? Cols : cols_) { }


    template <typename... Args, std::size_t M = Size, help::EnableIfArray<M> = 0>
    Matrix (Args&&... args) : Base{std::forward<Args>(args)...}, rows_(Rows), cols_(Cols) { }


    inline int rows () const { return rows_; }

    inline int cols () const { return cols_; }


    template <typename U>
    inline bool whitinBounds (const std::pair<U, U>& p) { return whitinBounds(p.first, p.second); }

    inline bool whitinBounds (int i, int j) { return whitinRows(i) && whitinCols(j); }


    inline bool whitinRows (int i) { return i >= 0 && i < rows(); }

    inline bool whitinCols (int j) { return j >= 0 && j < cols(); }


    inline decltype(auto) operator () (int i, int j) { return this->operator[](i * cols() + j); }

    inline decltype(auto) operator () (int i, int j) const { return this->operator[](i * cols() + j); }


    template <typename U>
    inline decltype(auto) operator () (const std::pair<U, U>& p) { return this->operator()(p.first, p.second); }

    template <typename U>
    inline decltype(auto) operator () (const std::pair<U, U>& p) const { return this->operator()(p.first, p.second); }


private:

    int rows_;
    int cols_;

};

}

#endif // CNT_MATRIX_H