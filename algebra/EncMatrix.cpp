#include "utils/MatrixAlgebraUtils.hpp"
#include "utils/FHEUtils.hpp"
#include "fhe/replicate.h"
#include "EncMatrix.hpp"
#include <thread>
namespace MDL {
EncMatrix& EncMatrix::pack(const Matrix<long>  & mat,
                           const EncryptedArray& ea)
{
    this->resize(mat.rows(), _pk);

    for (size_t r = 0; r < mat.rows(); r++) {
        this->at(r).pack(mat[r], ea);
    }
    return *this;
}

template<>
void EncMatrix::unpack(Matrix<long>        & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea,
                       bool                  negate) const
{
    result.resize(this->size());

    for (size_t r = 0; r < this->size(); r++) {
        this->at(r).unpack(result[r], sk, ea, negate);
    }
}

template<>
void EncMatrix::unpack(Matrix<NTL::ZZX>    & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea,
                       bool                  negate) const
{
    result.resize(this->size());

    for (size_t r = 0; r < this->size(); r++) {
        this->at(r).unpack(result[r], sk, ea, negate);
    }
}

EncVector EncMatrix::dot(const EncVector     & oth,
                         const EncryptedArray& ea) const
{
    std::vector<EncVector> result(this->size(),
                                  oth.getPubKey());
    std::vector<std::thread> workers;
    std::atomic<size_t> counter(0);

    for (int wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([this, &result,
                                                 &counter, &oth, &ea]()
            {
                size_t next;

                while ((next = counter.fetch_add(1)) < ea.size()) {
                    result[next] = this->at(next);
                    result[next].dot(oth, ea);
                    auto one_bit_mask = make_bit_mask(ea, next);
                    result[next].multByConstant(one_bit_mask);
                }
            })));
    }

    for (auto && wr : workers) wr.join();

    for (size_t r = 1; r < result.size(); r++) {
        result[0] += result[r];
    }
    return result[0];
}

EncVector EncMatrix::column_dot(const EncVector     & oth,
                                const EncryptedArray& ea) const
{
    std::vector<EncVector>   parts(this->size(), this->at(0).getPubKey());
    std::atomic<size_t>      counter(0);
    std::vector<std::thread> workers;

    for (int wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&oth, &ea, &parts,
                                                 &counter, this]() {
                size_t c;

                while ((c = counter.fetch_add(1)) < ea.size()) {
                    EncVector vec(oth);
                    replicate(ea, vec, c);
                    vec.multiplyBy(this->at(c));
                    parts[c] = vec;
                }
            })));
    }

    for (auto && wr : workers) {
        wr.join();
    }

    EncVector result(parts[0]);

    for (size_t i = 1; i < this->size(); i++) {
        result += parts[i];
    }
    return result;
}

EncMatrix& EncMatrix::transpose(const EncryptedArray& ea)
{
    // need to be square matrix
    if (ea.size() != this->size()) return *this;

    auto dim = ea.size();
    auto mat = *this;

    for (size_t row = 1; row < dim; row++) {
        ea.rotate(mat[row], row);
    }

    for (size_t col = 0; col < dim; col++) {
        auto new_col = mat[0];
        new_col.multByConstant(make_bit_mask(ea, col));

        for (size_t row = 1; row < dim; row++) { auto tmp(mat[row]);
            tmp.multByConstant(make_bit_mask(ea, (col + row) % dim));
            new_col += tmp;
        }

        ea.rotate(new_col, -col);
        this->at(col) = new_col;
    }
    return *this;
}

EncMatrix& EncMatrix::dot(const EncMatrix &oth,
                          const EncryptedArray &ea,
                          long col_to_process)
{
    /*
       [[1 2]     [[5 6]
       [3 4]]  *  [7 8]]

       [1 1] * [5 6] + [2 2] * [7 8] = [19 22]
       [3 3] * [5 6] + [4 4] * [7 8] = [43 50]
     */
    auto rows_nr = this->size();
    col_to_process = col_to_process == 0 ? ea.size() : col_to_process;
    assert(rows_nr == oth.size());
    assert(col_to_process <= ea.size());
    for (size_t row = 0; row < rows_nr; row++) {
        EncVector oneRow(_pk);
        for (size_t col = 0; col < col_to_process; col++) {
            auto tmp(this->at(row));
            replicate(ea, tmp, col);
            tmp.multiplyBy(oth[col]);
            if (col > 0) oneRow += tmp;
            else oneRow = tmp;
        }
        this->at(row) = oneRow;
    }
    return *this;
}

EncMatrix& EncMatrix::operator-=(const EncMatrix &oth)
{
    assert(this->size() == oth.size());

    for (size_t row = 0; row < oth.size(); row++) {
        this->at(row) -= oth[row];
    }
    return *this;
}

EncMatrix& EncMatrix::multByConstant(const NTL::ZZX &c)
{
    for (auto &row : *this) {
        row.multByConstant(c);
    }
    return *this;
}

EncMatrix& EncMatrix::addConstant(const std::vector<NTL::ZZX> &cons)
{
    assert(cons.size() == this->size());
    for (size_t i = 0; i < cons.size(); i++) {
        this->at(i).addConstant(cons[i]);
    }
    return *this;
}

EncMatrix& EncMatrix::negate()
{
    for (auto &row : *this) {
        row.negate();
    }
}

EncMatrix& EncMatrix::operator+=(const EncMatrix& oth)
{
    if (this->size() != oth.size()) {
        fprintf(stderr, "Warnning! adding two mismatch size matrix!\n");
        return *this;
    }

    for (size_t i = 0; i < this->size(); i++) this->at(i) += oth[i];
    return *this;
}

EncMatrix& EncMatrix::operator-=(const EncMatrix& oth)
{
    if (this->size() != oth.size()) {
        fprintf(stderr, "Warnning! adding two mismatch size matrix!\n");
        return *this;
    }

    for (size_t i = 0; i < this->size(); i++) this->at(i) -= oth[i];
    return *this;
}

EncMatrix& EncMatrix::multByConstant(const NTL::ZZX cons)
{
    for (size_t i = 0; i < this->size(); i++) this->at(i).multByConstant(cons);
    return *this;
}
} // namespace MDL
