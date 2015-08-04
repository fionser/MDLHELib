#include "MPEncMatrix.hpp"
#include "MPPubKey.hpp"
#include "MPSecKey.hpp"
#include "MPEncArray.hpp"
#include "MPReplicate.h"
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

MPEncMatrix::MPEncMatrix(const MPPubKey &pk) : _pk(pk) {}

MPEncMatrix::MPEncMatrix(const MPPubKey &pk,
                         const std::vector<MPEncVector> &copy)
    : _pk(pk) { ctxts = copy; }

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
                              const MPEncArray &ea,
                              long columnToProces)
{
    if (columnToProces <= 0) columnToProces = ea.slots();
    auto rows = rowsNum();

    for (long row = 0; row < rows; row++) {
        MPEncVector oneRow(_pk);
        for (long col = 0; col < columnToProces; col++) {
            auto tmp(ctxts[row]);
            replicate(tmp, ea, col);
            tmp *= oth.ctxts[col];
            if (col > 0) oneRow += tmp;
            else oneRow = tmp;
        }
        oneRow.reLinearize();
        ctxts[row] = oneRow;
    }

    return *this;
}

MPEncMatrix& MPEncMatrix::addConstant(const MDL::Matrix<long> &con,
                                      const MPEncArray &ea)
{
    if (rowsNum() != con.rows()) {
        printf("Warnning! MPEncMatrix addConstant!\n");
        return *this;
    }

    auto rows = con.rows();
    for (long row = 0; row < rows; row++) {
        ctxts[row].addConstant(con[row], ea);
    }

    return *this;
}

MPEncMatrix& MPEncMatrix::mulConstant(const MDL::Matrix<long> &con,
                                      const MPEncArray &ea)
{
    if (rowsNum() != con.rows()) {
        printf("Warnning! MPEncMatrix mulConstant!\n");
        return *this;
    }

    auto rows = con.rows();
    for (long row = 0; row < rows; row++) {
        ctxts[row].mulConstant(con[row], ea);
    }

    return *this;
}

MPEncMatrix& MPEncMatrix::negate()
{
    for (auto &row : ctxts) row.negate();
    return *this;
}

MPEncMatrix& MPEncMatrix::operator+=(const MPEncMatrix &oth)
{
    if (rowsNum() != oth.rowsNum()) {
        printf("Warnning! MPEncMatrix operator +=!\n");
        return *this;
    }

    for (long r = 0; r < oth.rowsNum(); r++) {
        ctxts[r] += oth.ctxts[r];
    }
    return *this;
}

MPEncMatrix& MPEncMatrix::operator-=(const MPEncMatrix &oth)
{
    if (rowsNum() != oth.rowsNum()) {
        printf("Warnning! MPEncMatrix operator -=!\n");
        return *this;
    }

    for (long r = 0; r < oth.rowsNum(); r++) {
        ctxts[r] -= oth.ctxts[r];
    }
    return *this;
}
