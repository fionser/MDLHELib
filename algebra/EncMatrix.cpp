#include "utils/MatrixAlgebraUtils.hpp"

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
                       const EncryptedArray& ea) const
{
    result.resize(this->size());

    for (size_t r = 0; r < this->size(); r++) {
        this->at(r).unpack(result[r], sk, ea);
    }
}

template<>
void EncMatrix::unpack(Matrix<NTL::ZZX>    & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea) const
{
    result.resize(this->size());

    for (size_t r = 0; r < this->size(); r++) {
        this->at(r).unpack(result[r], sk, ea);
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

EncMatrix& EncMatrix::transpose(const EncryptedArray& ea)
{
    // need to be square matrix
    if (ea.size() != this->size()) return *this;

    auto dim = ea.size();
    EncMatrix tmp(_pk);

    /// first store the first row
    tmp.resize(dim, this->at(0));

    for (size_t col = 0; col < dim; col++) {
        auto mask = make_bit_mask(ea, col);
        tmp[col].multByConstant(mask);
        ea.rotate(tmp[col], -col);

        for (size_t row = 1; row < dim; row++) {
            Ctxt copy(this->at(row));
            copy.multByConstant(mask);
            ea.rotate(copy, row - col);
            tmp[col] += copy;
        }
    }

    this->swap(tmp);
    return *this;
}
} // namespace MDL
