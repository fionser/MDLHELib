#include "MPEncArray.hpp"
#include "MPContext.hpp"
MPEncArray::MPEncArray(const MPContext &context)
    : m_primes(context.primes()),
      m_plainSpace(context.plainSpace())
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
