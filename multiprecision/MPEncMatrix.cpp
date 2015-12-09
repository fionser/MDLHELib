#include "MPEncMatrix.hpp"
#include "MPPubKey.hpp"
#include "MPSecKey.hpp"
#include "MPEncArray.hpp"
#include "MPReplicate.h"
#include "MPRotate.h"
#include <map>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

MPEncMatrix::MPEncMatrix(const std::vector<MPEncVector> &copy) { ctxts = copy; }

void MPEncMatrix::pack(const MDL::Matrix<long> &mat,
                       const MPPubKey &pk,
                       const MPEncArray &ea)
{
    auto rows = mat.rows();
    ctxts.resize(rows, pk);

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
                              const MPPubKey &pk,
                              const MPEncArray &ea) const
{
    std::vector<MPEncVector> parts(rowsNum(), pk);
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

MPEncMatrix& MPEncMatrix::dot2(const MPEncMatrix &oth,
                               const MPEncArray &ea,
                               const MPPubKey &pk,
                               long columnToProces)
{
    if (columnToProces <= 0) columnToProces = ea.slots();
    auto rows = rowsNum();

    for (long row = 0; row < rows; row++) {
        MPEncVector oneRow(pk);

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

MPEncMatrix& MPEncMatrix::dot(const MPEncMatrix &oth,
                              const MPEncArray &ea,
                              const MPPubKey &pk,
                              long columnToProces)
{
    if (columnToProces <= 0) columnToProces = ea.slots();
    auto rows = rowsNum();

    for (long row = 0; row < rows; row++) {
        MPEncVector oneRow(pk);

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

MPEncMatrix mulMatrix(const MPEncVector &vec,
                      const MDL::Matrix<long> &mat,
                      const MPEncArray &ea)
{
    std::vector<MPEncVector> ctxts;
    for (auto &row : mat) {
        auto tmp(vec);
        ctxts.push_back(tmp.mulConstant(row, ea));
    }
    return MPEncMatrix(ctxts);
}

static MPEncVector recrusiveFlatten(const std::vector<MPEncVector> &rows,
                                    const std::vector<long> lengths,
                                    const MPEncArray &ea) {
    auto sze = rows.size();
    if (sze == 1) return rows.front();
    std::vector<MPEncVector> new_rows;
    std::vector<long> new_lengths;
    for (size_t i = 0; i < sze; i += 2) {
        if (i + 1 < sze) {
            auto tmp(rows[i]), tmp2(rows[i + 1]);
            rotate(tmp2, ea, lengths[i]);
            tmp += tmp2;
            new_rows.push_back(tmp);
            new_lengths.push_back(lengths[i] + lengths[i + 1]);
        } else {
            new_rows.push_back(rows[i]);
            new_lengths.push_back(lengths[i]);
        }
    }
    return recrusiveFlatten(new_rows, new_lengths, ea);
}

MPEncVector MPEncMatrix::flatten(const MPEncArray &ea, long columnToProces) const
{
    auto rows = ctxts.size();
    assert(rows * columnToProces <= ea.slots());
    std::vector<long> lengths(rows, columnToProces);
    return recrusiveFlatten(ctxts, lengths, ea);
}

MPEncMatrix& MPEncMatrix::repeatEachRow(const long k, long columns,
                                        const MPEncArray &ea, const MPPubKey &pk) {
    if (ea.slots() < k * columns) {
        printf("WARNNING: Not enough slots for repeating each row\n");
        return *this;
    }

    for (auto &ctxt : ctxts) {
        ctxt = repeat(ctxt, ea, pk, columns, k);
    }
    return *this;
}
