#include "MPSwap.h"
#include "multiprecision.h"
#include <cstring>
void batchSwap(MPEncVector &out,
               const MPEncVector &origin,
               const swap_t &locs,
               const MPPubKey &pk,
               const MPEncArray &ea)
{
    MDL::Vector<long> mask(ea.slots(), 0);
    long off = -1;
    for (auto &pair : locs) {
        mask[pair.first] = 1;
        mask[pair.second] = 1;
        if (off == -1)
            off = pair.second - pair.first;
        else
            assert(off == pair.second - pair.first);
    }

    for (auto &m : mask) m ^= 1;
    out = origin;
    out.mulConstant(mask, ea);

    for (auto &m : mask) m = 0;
    for (auto &pair : locs) mask[pair.first] = 1;
    auto left(origin);
    left.mulConstant(mask, ea);

    for (auto &m : mask) m = 0;
    for (auto &pair : locs) mask[pair.second] = 1;
    auto right(origin);
    right.mulConstant(mask, ea);

    rotate(right, ea, -off);
    rotate(left, ea, off);

    out += left;
    out += right;
}
