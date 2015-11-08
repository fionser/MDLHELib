#include "CRT.hpp"
#include "NTL/ZZ.h"
namespace MDL {
static NTL::ZZ InvMod(NTL::ZZ a, NTL::ZZ p)
{
    NTL::ZZ p0(p), t, q;
    NTL::ZZ x0(0), x1(1);
    if (p == 1) return NTL::to_ZZ(1);
    while (a > 1) {
        q = a / p;
        t = p; p = a % p; a = t;
        t = x0; x0 = x1 - q * x0; x1 = t;
    }
    if (x1 < 0) x1 += p0;
    return  x1;
}

template<typename T>
NTL::ZZ CRT(const std::vector<long> &a,
            const std::vector<T> &primes) {
    NTL::ZZ product(1), sum(0);
    auto size = a.size();

    for (auto &prime : primes) product *= prime;

    for (size_t i = 0; i < size; i++) {
        auto p = product / primes[i];
        sum += (a[i] * MDL::InvMod(p, NTL::to_ZZ(primes[i])) * p);
    }
    return sum % product;
}

} // namespace MDL
