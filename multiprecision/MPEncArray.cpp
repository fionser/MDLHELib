#include "MPEncArray.hpp"
#include "MPContext.hpp"
MPEncArray::MPEncArray(const MPContext &context)
    : m_r(context.getR()),
      m_plainSpace(context.plainSpace()),
      m_primes(context.primes())
{
    auto parts = context.partsNum();
    arrays.reserve(parts);
    for (long i = 0; i < parts; i++) {
        auto cntxt = context.get(i);
        auto G = cntxt->alMod.getFactorsOverZZ()[0];
        arrays.push_back(std::make_shared<EncryptedArray>(*cntxt, G));
        if (minimumSlot == 0 || minimumSlot > arrays[i]->size()) {
            minimumSlot = arrays[i]->size();
        }
    }
}

std::vector<long> MPEncArray::rPrimes() const
{
    std::vector<long> rprimes = m_primes;

    for (auto &p : rprimes) {
        p = std::pow(p, m_r);
    }
    return rprimes;
}
