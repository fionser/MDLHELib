#ifndef MULTIPRECISION_MPCONTEXT_HPP
#define MULTIPRECISION_MPCONTEXT_HPP
#include "fhe/FHEContext.h"
#include <memory>
#include <NTL/ZZ.h>
#include <iostream>
/// MultiPrecision FHEcontext
class MPContext {
public:
    typedef std::shared_ptr<FHEcontext> contextPtr;
    /// @param m, p, r is the same with FHEcontext
    /// @param parts is how many parts to use
    MPContext(long m, long p, long r, long parts);

    void buildModChain(long L);
    /// @return the specific FHEcontext
    contextPtr get(int index) const { return contexts[index]; }

    size_t partsNum() const { return contexts.size(); }

    double precision() const;

    ZZ plainSpace() const { return m_plainSpace; }

    std::vector<long> primes() const { return m_primes; }

    long getR() const { return m_r; }
private:
    ZZ m_plainSpace = NTL::to_ZZ(1);
    long m_r = 1;
    std::vector<long> m_primes;
    std::vector<contextPtr> contexts;
};
#endif // multiprecision/mpcontext.hpp
