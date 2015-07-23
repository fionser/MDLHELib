#include "EncVector.hpp"
#include "EncMatrix.hpp"
#include "fhe/replicate.h"
#include <NTL/ZZX.h>
#include <vector>
#include <atomic>
#include <thread>
namespace MDL {
#ifdef FHE_THREAD
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif
EncVector& EncVector::pack(const Vector<long>  & vec,
                           const EncryptedArray& ea)
{
    assert(vec.size() <= ea.size());
    if (vec.size() < ea.size()) {
        auto tmp(vec);
        tmp.resize(ea.size(), 0);
        ea.encrypt(*this, getPubKey(), tmp);
    } else {
        ea.encrypt(*this, getPubKey(), vec);
    }
    return *this;
}

std::vector<EncVector>EncVector::partition_pack(const Vector<long>  & vec,
                                                const FHEPubKey     & pk,
                                                const EncryptedArray& ea)
{
    auto parts_nr = (vec.size() + ea.size() - 1) / ea.size();
    std::vector<EncVector> ctxts(parts_nr, pk);

    return ctxts;
}

template<>
bool EncVector::unpack(Vector<long>        & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea,
                       bool                  negate) const
{
    ea.decrypt(*this, sk, result);
    if (negate) {
        auto plainSpace = ea.getContext().alMod.getPPowR();
        auto half = plainSpace >> 1;
        for (auto &e : result) {
            if (e > half) e -= plainSpace;
        }
    }
    return this->isCorrect();
}

template<>
bool EncVector::unpack(Vector<NTL::ZZX>    & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea,
                       bool                  negate) const
{
    ea.decrypt(*this, sk, result);
    if (negate) {
        auto plainSpace = ea.getContext().alMod.getPPowR();
        auto half = plainSpace >> 1;
        for (auto &e : result) {
            // if e is 0, the length of NTL::ZZX is zero
            if (e.rep.length() > 0 && e[0] > half) {
                e[0] -= plainSpace;
            }
        }
    }
    return this->isCorrect();
}

EncVector& EncVector::dot(const EncVector     & oth,
                          const EncryptedArray& ea)
{
    this->multiplyBy(oth);
    totalSums(ea, *this);
    return *this;
}

EncMatrix EncVector::covariance(const EncryptedArray& ea, long actualDimension)
{
    actualDimension = actualDimension == 0 ? ea.size() : actualDimension;
    EncMatrix mat(getPubKey());
    mat.resize(actualDimension, getPubKey());
    std::atomic<long> counter(0);
    std::vector<std::thread> workers;
    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&counter, &actualDimension,
                                                &mat, &ea, this]() {
            long next;
            while((next = counter.fetch_add(1)) < actualDimension) {
                auto tmp(*this);
                replicate(ea, tmp, next);
                mat[next].multiplyBy(tmp);
            }
            })));
    }

    return mat;
}
} // namespace MDL
