#ifndef ENCVCTOR_HPP
#define ENCVCTOR_HPP
#include "fhe/EncryptedArray.h"
#include "fhe/Ctxt.h"
#include "fhe/FHE.h"

#include "Vector.hpp"
namespace MDL {
class EncVector : public Ctxt {
public:
    EncVector(const FHEPubKey& pk) : Ctxt(pk) {}

    EncVector& pack(const Vector<long>  & vec,
                    const EncryptedArray& ea);

    EncVector& dot(const EncVector     & oth,
                   const EncryptedArray& ea);

    template<typename U>
    void unpack(Vector<U>           & result,
                const FHESecKey     & sk,
                const EncryptedArray& ea) const;
};
} // namespace MDL
#endif // ENCVECTOR_HPP
