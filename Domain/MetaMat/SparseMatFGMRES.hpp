/*******************************************************************************
 * Copyright (C) 2017-2022 Theodore Chang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
/**
 * @class SparseMatFGMRES
 * @brief A SparseMatFGMRES class that holds matrices.
 *
 * @author tlc
 * @date 20/01/2021
 * @version 0.1.0
 * @file SparseMatFGMRES.hpp
 * @addtogroup MetaMat
 * @{
 */

// ReSharper disable CppCStyleCast
#ifndef SPARSEMATBASEFGMRES_HPP
#define SPARSEMATBASEFGMRES_HPP

#ifdef SUANPAN_MKL

#include <mkl_rci.h>
#include <mkl_spblas.h>
#include "csr_form.hpp"
#include "SparseMat.hpp"

template<sp_d T> class SparseMatBaseFGMRES : public SparseMat<T> {
    static double tolerance;

    const matrix_descr descr;

    podarray<int> ipar;
    podarray<double> dpar;
    podarray<double> work;

    friend void set_fgmres_tolerance(double);

public:
    SparseMatBaseFGMRES(uword, uword, uword, bool);
    SparseMatBaseFGMRES(const SparseMatBaseFGMRES&) = default;
    SparseMatBaseFGMRES(SparseMatBaseFGMRES&&) noexcept = delete;
    SparseMatBaseFGMRES& operator=(const SparseMatBaseFGMRES&) = delete;
    SparseMatBaseFGMRES& operator=(SparseMatBaseFGMRES&&) noexcept = delete;
    ~SparseMatBaseFGMRES() override;

    int solve(Mat<T>&, const Mat<T>&) override;
};

template<sp_d T> double SparseMatBaseFGMRES<T>::tolerance = 1E-4;

inline void set_fgmres_tolerance(const double T) { SparseMatBaseFGMRES<double>::tolerance = T; }

template<sp_d T> SparseMatBaseFGMRES<T>::SparseMatBaseFGMRES(const uword in_row, const uword in_col, const uword in_elem, const bool in_sym)
    : SparseMat<T>(in_row, in_col, in_elem)
    , descr{in_sym ? SPARSE_MATRIX_TYPE_SYMMETRIC : SPARSE_MATRIX_TYPE_GENERAL, SPARSE_FILL_MODE_FULL, SPARSE_DIAG_NON_UNIT}
    , ipar(128)
    , dpar(128) {}

template<sp_d T> SparseMatBaseFGMRES<T>::~SparseMatBaseFGMRES() {
    mkl_free_buffers();
}

template<sp_d T> int SparseMatBaseFGMRES<T>::solve(Mat<T>& X, const Mat<T>& B) {
    const auto N = static_cast<int>(B.n_rows);

    work.zeros((5 * N * N + 11 * N + 2) / 2);

    X = B;

    csr_form<T, int> csr_mat(this->triplet_mat);

    int request;

    for(auto I = 0llu; I < B.n_cols; ++I) {
        dfgmres_init(&N, (double*)X.colptr(I), (double*)B.colptr(I), &request, ipar.memptr(), dpar.memptr(), work.memptr());
        if(request != 0) return request;

        ipar[8] = 1;
        ipar[9] = 0;
        ipar[11] = 1;
        dpar[0] = tolerance;

        dfgmres_check(&N, (double*)X.colptr(I), (double*)B.colptr(I), &request, ipar.memptr(), dpar.memptr(), work.memptr());
        if(request == -1100) return request;

        while(true) {
            dfgmres(&N, (double*)X.colptr(I), (double*)B.colptr(I), &request, ipar.memptr(), dpar.memptr(), work.memptr());
            if(request == 0) {
                int counter;
                dfgmres_get(&N, (double*)X.colptr(I), (double*)B.colptr(I), &request, ipar.memptr(), dpar.memptr(), work.memptr(), &counter);
                if(request != 0) return request;
                suanpan_debug("FGMRES uses %d iterations.\n", counter);
                break;
            }
            if(request != 1) return request;
            const vec xn(&work[ipar[21] - 1llu], X.n_rows, false, true);
            // ReSharper disable once CppInitializedValueIsAlwaysRewritten
            // ReSharper disable once CppEntityAssignedButNoRead
            vec yn(&work[ipar[22] - 1llu], X.n_rows, false, true);
            yn = csr_mat * xn;
        }
    }

    return 0;
}

template<sp_d T> class SparseMatFGMRES : public SparseMatBaseFGMRES<T> {
public:
    SparseMatFGMRES(uword, uword, uword = 0);

    unique_ptr<MetaMat<T>> make_copy() override;
};

template<sp_d T> SparseMatFGMRES<T>::SparseMatFGMRES(const uword in_row, const uword in_col, const uword in_elem)
    : SparseMatBaseFGMRES<T>(in_row, in_col, in_elem, false) {}

template<sp_d T> unique_ptr<MetaMat<T>> SparseMatFGMRES<T>::make_copy() { return std::make_unique<SparseMatFGMRES<T>>(*this); }

template<sp_d T> class SparseSymmMatFGMRES : public SparseMatBaseFGMRES<T> {
public:
    SparseSymmMatFGMRES(uword, uword, uword = 0);

    unique_ptr<MetaMat<T>> make_copy() override;
};

template<sp_d T> SparseSymmMatFGMRES<T>::SparseSymmMatFGMRES(const uword in_row, const uword in_col, const uword in_elem)
    : SparseMatBaseFGMRES<T>(in_row, in_col, in_elem, true) {}

template<sp_d T> unique_ptr<MetaMat<T>> SparseSymmMatFGMRES<T>::make_copy() { return std::make_unique<SparseSymmMatFGMRES<T>>(*this); }

#endif

#endif

//! @}
