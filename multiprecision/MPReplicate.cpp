#include "MPReplicate.h"
#include "MPEncVector.hpp"
#include "MPEncMatrix.hpp"
#include "MPEncArray.hpp"
#include "MPRotate.h"
#include "fhe/replicate.h"
#include <vector>
#include <thread>
#include <cstring>
#include <map>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

void replicate(MPEncVector &vec,
               const MPEncArray &ea,
               const long c)
{
    std::vector<std::thread> worker;
    std::atomic<long> counter(0);
    const long parts = vec.partsNum();
    auto job = [&counter, &parts, &vec, &ea, &c]() {
        long i;
        while ((i = counter.fetch_add(1)) < parts) {
            replicate(*ea.get(i), vec.get(i), c);
        }
    };

    for (long wr = 0; wr < WORKER_NR; wr++) worker.push_back(std::thread(job));
    for (auto &&wr : worker) wr.join();
}

MPEncVector repeat(const MPEncVector &vec,
                   const MPEncArray &ea,
                   const MPPubKey &pk,
                   long columns,
                   long k) {
    assert(ea.slots() >= k * columns);
    MPEncVector rv(vec), res(pk);
    std::map<int, MPEncVector> map;

    long length = columns;
    while (k > 0) {
        if ((k & 1) == 1) {
            map.insert(std::make_pair(length, rv));
        }
        auto tmp(rv);
        rotate(tmp, ea, length);
        rv += tmp;
        length <<= 1;
        k >>= 1;
    }

    for (auto &kv : map) {
        if (res.partsNum() > 0) {
            rotate(res, ea, kv.first);
            res += kv.second;
        } else {
            res = kv.second;
        }
    }
    return res;
}

void swap(MPEncVector &out, const MPEncVector &origin,
          const MPEncArray &ea, const pairs_t &pairs) {
    assert(!pairs.empty());
    MDL::Vector<long> bmask(ea.slots());
    NTL::ZZX mask;
    MPEncVector left(origin), right(origin);
    auto offset = std::abs(pairs[0].first - pairs[0].second);

    std::memset(bmask.data(), 0, sizeof(long) * bmask.size());
    for (auto& p : pairs) {
        bmask[p.first] = 1;
    }
    left.mulConstant(bmask, ea);
    rotate(left, ea, offset);

    std::memset(bmask.data(), 0, sizeof(long) * bmask.size());
    for (auto& p : pairs) {
        bmask[p.second] = 1;
    }
    right.mulConstant(bmask, ea);
    rotate(right, ea, -offset);

    out = left;
    out += right;
}

void rearrange(MPEncVector &result, const MPEncVector &origin,
               const MPEncArray &ea,
               const long columns, const long K) {
    assert(columns * K <= ea.slots());
    MDL::Vector<long> mask(ea.slots());
    long boundary = columns * K;
    for (size_t i = 0; i < mask.dimension(); i++) {
        mask[i] = 0;
        auto id = i / columns;
        if (id >= columns) continue;
        auto j =  id * columns + id;
        assert(j < mask.dimension());
        mask[j] = 1;
    }

    result = origin;
    result.mulConstant(mask, ea);
    auto tmp(origin);
    for (long step = 1; step < columns; step++) {
        pairs_t pairs;
        for (long i = 0; i < columns; i++) {
            long j = i * columns + i + step;
            long k = j + step * (columns - 1);
            if (k < boundary) {
                pairs.push_back(std::make_pair(j, k));
            }
        }
        swap(tmp, origin, ea, pairs);
        result += tmp;
    }
}

void rearrange2(MPEncVector &result, const MPEncMatrix &origin,
                const long columns,
                const MPEncArray &ea, const MPPubKey &pk) {
    assert(columns == origin.rowsNum());
    long size = columns * columns;
    long boundary = size * columns;
    assert(boundary <= ea.slots());
    MDL::Vector<long> mask(ea.slots());
    std::memset(mask.data(), 0, sizeof(long) * mask.size());

    for (size_t i = 0; i < columns; i++) {
        auto j = i * size + i;
        for (size_t k = 0; k < columns; k++) {
            mask[j + k * columns] = 1;
        }
    }

    auto repeated = repeat(origin.flatten(ea, columns), ea, pk,
                           size, origin.rowsNum());
    result = repeated;
    result.mulConstant(mask, ea);
    MPEncVector tmp(pk);
    for (long step = 1; step < columns; step++) {
        pairs_t pairs;
        auto offset = step * (size - 1);
        for (long b = 0; b < columns; b++) {
            for (long j = b * size + step + b; j < (b + 1) * size; j += columns) {
                auto k = j + offset;
                if (k < boundary) {
                    pairs.push_back(std::make_pair(j, k));
                    //printf("%ld <-> %ld ", j, k);
                }
            }
            //printf(" | ");
        }
        //printf("\n");
        if (pairs.empty()) continue;
        swap(tmp, repeated, ea, pairs);
        result += tmp;
    }
}
