#include "EncVector.hpp"
#include <NTL/ZZX.h>
namespace MDL {
EncVector& EncVector::pack(const Vector<long>  & vec,
                           const EncryptedArray& ea)
{
    assert(vec.size() == ea.size());
    ea.encrypt(*this, getPubKey(), vec);
    return *this;
}

template<>
bool EncVector::unpack(Vector<long>        & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea) const
{
    ea.decrypt(*this, sk, result);
    return this->isCorrect();
}

template<>
bool EncVector::unpack(Vector<NTL::ZZX>    & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea) const
{
    ea.decrypt(*this, sk, result);
    return this->isCorrect();
}

EncVector& EncVector::dot(const EncVector     & oth,
                          const EncryptedArray& ea)
{
    this->multiplyBy(oth);
    totalSums(ea, *this);
    return *this;
}
} // namespace MDL
