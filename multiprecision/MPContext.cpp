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
    long trial = 0;
    while (generated < parts) {

        auto prime = NTL::RandomPrime_long(bits);
        if (getSlots(m, prime) >= slots) {
            auto ok = primes.insert(prime);
            if (ok.second) generated += 1;

        }

        if (trial++ > 1000) {
            printf("Error: Can not find enough primes, only found %ld\n",
                   generated);
            break;
        }
    }

    return primes;
}

MPContext::MPContext(long m, long p, long r, long parts)
{
    contexts.reserve(parts);
    auto primesSet = FindPrimes(m, p, parts);
    for (auto &prime : primesSet) {
        contexts.push_back(std::make_shared<FHEcontext>(m, prime, r));
        m_plainSpace *= prime;
        m_primes.push_back(prime);
    }
}

void MPContext::buildModChain(long L)
{
    for (auto &context : contexts) {
        ::buildModChain(*context, L);
    }
}

double MPContext::precision() const
{
    return NTL::log(plainSpace()) / NTL::log(NTL::to_ZZ(2));
}
