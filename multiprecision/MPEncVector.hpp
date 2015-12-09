#ifndef MULTIPRECISION_MPENCVECTOR_HPP
#define MULTIPRECISION_MPENCVECTOR_HPP
#include "algebra/EncVector.hpp"
#include "algebra/Vector.hpp"
#include <NTL/ZZ.h>
#include <NTL/ZZX.h>
class MPSecKey;
class MPPubKey;
class MPEncArray;
class MPEncVector {
public:
    MPEncVector(const MPPubKey &pk);

    ~MPEncVector() {}

    void pack(const MDL::Vector<long> &vec,
              const MPEncArray &ea);

    void unpack(MDL::Vector<NTL::ZZ> &vec,
                const MPSecKey &sk,
                const MPEncArray &ea,
                bool negate = true);

    void multiplyBy(const MPEncVector &oth);

    /// scalar product
    MPEncVector& dot(const MPEncVector &oth,
                     const MPEncArray &ea);

    MPEncVector& operator*=(const MPEncVector &oth);

    MPEncVector& operator+=(const MPEncVector &oth);

    MPEncVector& operator-=(const MPEncVector &oth);

    void negate();

    MPEncVector& addConstant(const MDL::Vector<long> &con,
                             const MPEncArray &ea);

    MPEncVector& mulConstant(const MDL::Vector<long> &con,
                             const MPEncArray &ea);

    size_t partsNum() const { return ctxts.size(); }

    MDL::EncVector& get(int index) { return ctxts[index]; }

    const MDL::EncVector& get(int index) const { return ctxts[index]; }

    void reLinearize();
private:
    std::vector<MDL::EncVector> ctxts;
};
#endif // multiprecision/MPEncVector.hpp
