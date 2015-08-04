#include "MPEncMatrix.hpp"
#include "MPPubKey.hpp"
#include "MPSecKey.hpp"
#include "MPEncArray.hpp"
#include "MPReplicate.h"

MPEncMatrix::MPEncMatrix(const MPPubKey &pk) : _pk(pk) {}

void MPEncMatrix::pack(const MDL::Matrix<long> &mat,
                       const MPEncArray &ea)
{
    auto rows = mat.rows();
    ctxts.resize(rows, _pk);

    for (long r = 0; r < rows; r++) {
        ctxts[r].pack(mat[r], ea);
    }
}

void MPEncMatrix::unpack(MDL::Matrix<NTL::ZZ> &result,
                         const MPSecKey &sk,
                         const MPEncArray &ea,
                         bool negate)
{
    const long rows = rowsNum();
    result.resize(rows);

    for (long r = 0; r < rows; r++) {
        ctxts[r].unpack(result[r], sk, ea, negate);
    }
}

MPEncVector MPEncMatrix::sDot(const MPEncVector &oth,
                              const MPEncArray &ea) const
{
    std::vector<MPEncVector> parts(rowsNum(), _pk);
    for (long c = 0; c < rowsNum(); c++) {
        auto tmp(oth);
        replicate(tmp, ea, c);
        tmp.multiplyBy(ctxts[c]);
        parts[c] = tmp;
    }

    for (long c = 1; c < rowsNum(); c++) {
        parts[0] += parts[c];
    }

    return parts[0];
}

MPEncMatrix& MPEncMatrix::dot(const MPEncMatrix &oth,
                              const MPEncArray &ea) const
{

}
