#ifndef ENCMATRIX_HPP
#define ENCMATRIX_HPP
#include "fhe/EncryptedArray.h"
#include "fhe/Ctxt.h"
#include "fhe/FHE.h"
#include <vector>
#include "EncVector.hpp"
#include "Matrix.hpp"
namespace MDL {
class EncMatrix : public std::vector<EncVector> {
public:
    EncMatrix(const FHEPubKey& pk)
        : std::vector<EncVector>(0, pk),
        _pk(pk) {}

    EncMatrix& pack(const Matrix<long>  & mat,
                    const EncryptedArray& ea);

    EncMatrix& transpose(const EncryptedArray& ea);

    EncVector dot(const EncVector     & oth,
                  const EncryptedArray& ea) const;

    /// If the EncMatrix was transposed can use this interface for
    /// quicker dot product.
    EncVector column_dot(const EncVector     & oth,
                        const EncryptedArray& ea) const;

    template<typename U>
    void unpack(Matrix<U>           & result,
                const FHESecKey     & sk,
                const EncryptedArray& ea) const;
private:
    const FHEPubKey& _pk;
    #ifdef FHE_THREADS
    static const int WORKER_NR = 4;
    #else // ifdef FHE_THREADS
    static const int WORKER_NR = 1;
    #endif // ifdef FHE_THREADS
};
}
#endif // ENCMATRIX_HPP
