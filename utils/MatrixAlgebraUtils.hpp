#ifndef MARTIXALGEBRA_HPP
#define MARTIXALGEBRA_HPP
#include "fhe/EncryptedArray.h"

#include <NTL/ZZX.h>

#include <cassert>
NTL::ZZX make_bit_mask(const EncryptedArray& ea,
                       const int             index)
{
    assert(ea.size() > 0 && index >= 0
           && index < ea.size());
    NTL::ZZX mask;
    std::vector<long> slots(ea.size(), 0);
    slots[index] = 1;
    ea.encode(mask, slots);
    return mask;
}

#endif // MARTIXALGEBRA_HPP
