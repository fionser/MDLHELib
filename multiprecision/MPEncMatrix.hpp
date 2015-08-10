#ifndef MULTIPRECISION_ENCMATRIX_HPP
#define MULTIPRECISION_ENCMATRIX_HPP
#include "MPEncVector.hpp"
#include "algebra/EncMatrix.hpp"
#include "algebra/Matrix.hpp"
#include <vector>
#include <NTL/ZZ.h>
#include <NTL/ZZX.h>

class MPPubKey;
class MPEncArray;
class MPSecKey;

class MPEncMatrix {
public:
    MPEncMatrix() {}

    MPEncMatrix(const std::vector<MPEncVector> &copy);

    size_t rowsNum() const { return ctxts.size(); }

    void pack(const MDL::Matrix<long> &mat,
              const MPPubKey &pk,
              const MPEncArray &ea);

    void unpack(MDL::Matrix<NTL::ZZ> &result,
                const MPSecKey &sk,
                const MPEncArray &ea,
                bool negate = true);
    /// assume that the matrix is symmetric
    MPEncVector sDot(const MPEncVector &oth,
                     const MPPubKey &pk,
                     const MPEncArray &ea) const;

    MPEncMatrix& dot(const MPEncMatrix &oth,
                     const MPEncArray &ea,
                     const MPPubKey &pk,
                     long columnToProces = 0);

    MPEncMatrix& addConstant(const MDL::Matrix<long> &con,
                             const MPEncArray &ea);

    MPEncMatrix& mulConstant(const MDL::Matrix<long> &con,
                             const MPEncArray &ea);

    MPEncMatrix& mulConstant(const NTL::ZZX &con);

    MPEncMatrix& negate();

    MPEncMatrix& operator+=(const MPEncMatrix &oth);

    MPEncMatrix& operator-=(const MPEncMatrix &oth);

    const MPEncVector& get(int index) const { return ctxts[index]; }
private:
    // MPPubKey &_pk;
    std::vector<MPEncVector> ctxts;
};

MPEncMatrix mulMatrix(const MPEncVector &vec,
                      const MDL::Matrix<long> &mat,
                      const MPEncArray &ea);
#endif // multiprecision/EncMatrix.hpp
