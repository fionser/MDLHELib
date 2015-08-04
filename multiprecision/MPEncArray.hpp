#ifndef MULTIPRECISION_MPENCARRAY_HPP
#define MULTIPRECISION_MPENCARRAY_HPP
#include "fhe/EncryptedArray.h"
#include <vector>
#include <memory>
class MPContext;
class MPEncArray {
public:
    typedef std::shared_ptr<EncryptedArray> encArrayPtr;
    MPEncArray(const MPContext &context);

    encArrayPtr get(int index) const { return arrays[index]; }

    long slots() const { return minimumSlot; }

    size_t arrayNum() const { return arrays.size(); }

    std::vector<long> primes() const { return m_primes; }

    NTL::ZZ plainSpace() const { return m_plainSpace; }
private:
    long minimumSlot = 0;
    NTL::ZZ m_plainSpace;
    std::vector<long> m_primes;
    std::vector<encArrayPtr> arrays;
};
#endif // multiprecision/MPEncArray.hpp
