#include "Multiprecision.h"
#include <map>
void repeat(MPEncVector &out,
            const MPEncVector &origin,
            long k,
            const MPPubKey &pk,
            const MPEncArray &ea) {
    std::map<int, MPEncVector> store;
    auto length = origin.getLength();
    assert(length > 0 && length * k <= ea.slots());

    auto tmp(origin);
    while (k > 0) {
        if ((k & 1) == 1) store.insert(std::make_pair(length, tmp));

        auto tmp2(tmp);
        rotate(tmp2, ea, length);
        tmp += tmp2;
        length <<= 1;
        k >>= 1;
    }

    length = 0;
    for (auto &kv : store) {
        if (length == 0) {
            out = kv.second;
            length = kv.first;
        } else {
            if (length < kv.first)
                rotate(kv.second, ea, length);
            else
                rotate(out, ea, kv.first);

            out += kv.second;
            length += kv.first;
        }
    }
}
