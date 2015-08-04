#include "MPContext.hpp"
#include "fhe/NumbTh.h"
#include <set>
#include <NTL/ZZ.h>
#include <cmath>
static long getSlots(long m, long p)
{
    return phi_N(m) / multOrd(p, m);
}

static std::set<long> FindPrimes(long m, long p, long parts)
{
    auto slots = getSlots(m, p);
    auto bits = static_cast<long>(std::ceil(std::log2(static_cast<double>(p))));
    std::set<long> primes{p};
    long generated = 1;

    while (generated < parts) {
        auto prime = NTL::RandomPrime_long(bits);
        if (getSlots(m, prime) >= slots) {
            generated+= 1;
            primes.insert(prime);
        }
    }

    return primes;
}

MPContext::MPContext(long m, long p, long r, long parts)
{
    contexts.reserve(parts);
    auto primes = FindPrimes(m, p, parts);
    for (auto &prime : primes) {
        contexts.push_back(std::make_shared<FHEcontext>(m, prime, r));
        plainSpace *= prime;
    }
}

void MPContext::buildModChain(long L)
{
    for (auto& context : contexts) {
        ::buildModChain(*context, L);
    }
}

double MPContext::precision() const
{
    return NTL::log(plainSpace) / NTL::log(NTL::to_ZZ(2));
}
