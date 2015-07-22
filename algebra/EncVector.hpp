#ifndef ENCVCTOR_HPP
#define ENCVCTOR_HPP
#include "fhe/EncryptedArray.h"
#include "fhe/Ctxt.h"
#include "fhe/FHE.h"

#include "Vector.hpp"
namespace MDL {
class EncMatrix;
class EncVector : public Ctxt {
public:
    EncVector(const FHEPubKey& pk) : Ctxt(pk) {}

    ~EncVector() {}

    EncVector& pack(const Vector<long>  & vec,
                    const EncryptedArray& ea);

    static std::vector<EncVector>partition_pack(const Vector<long>  & vec,
                                                const FHEPubKey     & pk,
                                                const EncryptedArray& ea);

    EncVector& dot(const EncVector     & oth,
                   const EncryptedArray& ea);

    template<typename U>
    bool unpack(Vector<U>           & result,
                const FHESecKey     & sk,
                const EncryptedArray& ea,
                bool                  negate = false) const;

    EncMatrix covariance(const EncryptedArray& ea,
                         long                  actualDimension = 0);
};
} // namespace MDL
#endif // ENCVECTOR_HPP
