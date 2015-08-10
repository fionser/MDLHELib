#include "MPEncVector.hpp"
#include "MPPubKey.hpp"
#include "MPSecKey.hpp"
#include "MPEncArray.hpp"
#include "algebra/CRT.hpp"
#include <vector>
#include <thread>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

MPEncVector::MPEncVector(const MPPubKey &pk)
{
    auto num = pk.keyNum();
    ctxts.reserve(num);
    for (int i = 0; i < num; i++) {
        ctxts.push_back(MDL::EncVector(*pk.get(i)));
    }
}

void MPEncVector::pack(const MDL::Vector<long> &vec,
                       const MPEncArray &ea)
{
    auto num = ea.arrayNum();
    for (int i = 0; i < num; i++) {
        ctxts[i].pack(vec, *ea.get(i));
    }
}

void MPEncVector::unpack(MDL::Vector<NTL::ZZ> &vec,
                         const MPSecKey &sk,
                         const MPEncArray &ea,
                         bool negate)
{
    auto slots = ea.slots();
    const auto num = ea.arrayNum();
    auto rPrimes = ea.rPrimes();
    auto plainSpace = ea.plainSpace();
    std::vector<MDL::Vector<long>> tmps(num);
    std::vector<std::thread> worker;
    std::atomic<long> counter(0);
    auto job = [this, &sk, &ea, &tmps, &num, &counter]() {
       long i;
       while ((i = counter.fetch_add(1)) < num) {
           bool ok = ctxts[i].unpack(tmps[i], *sk.get(i), *ea.get(i));
           if (!ok) printf("Warning! the decryption maybe incorrect!\n");
       }
    };

    for (long wr = 0; wr < WORKER_NR; wr++) worker.push_back(std::thread(job));
    for (auto &&wr : worker) wr.join();

    vec.resize(slots);
    for (long s = 0; s < slots; s++) {
        std::vector<long> values(num);
        for (long i = 0; i < num; i++)
            values[i] = tmps[i][s];

        vec[s] = MDL::CRT(values, rPrimes);
        if (negate && vec[s] > (plainSpace >> 1)) {
            vec[s] -= plainSpace;
        }
    }
}

void MPEncVector::negate()
{
    for (auto &ctxt : ctxts) ctxt.negate();
}

void MPEncVector::multiplyBy(const MPEncVector &oth)
{
    std::vector<std::thread> worker;
    std::atomic<long> counter(0);
    const auto num = oth.ctxts.size();
    auto job = [&counter, &num, this, &oth]() {
        long i;
        while ((i = counter.fetch_add(1)) < num) {
            ctxts[i].multiplyBy(oth.ctxts[i]);
        }
    };

    if (num != ctxts.size()) {
        printf("Error! MPEncVectors multiplyBy!\n");
        return;
    }


    for (long wr = 0; wr < WORKER_NR; wr++) worker.push_back(std::thread(job));

    for (auto &&wr : worker) wr.join();
}

MPEncVector& MPEncVector::operator*=(const MPEncVector &oth)
{
    std::vector<std::thread> worker;
    std::atomic<long> counter(0);
    const auto num = oth.ctxts.size();
    auto job = [&counter, &num, this, &oth]() {
        long i;
        while ((i = counter.fetch_add(1)) < num) {
            ctxts[i] *= oth.ctxts[i];
        }
    };

    if (num != ctxts.size()) {
        printf("Error! MPEncVectors operator*=!\n");
        return *this;
    }


    for (long wr = 0; wr < WORKER_NR; wr++) worker.push_back(std::thread(job));

    for (auto &&wr : worker) wr.join();

    return *this;
}

MPEncVector& MPEncVector::operator+=(const MPEncVector &oth)
{
    auto num = oth.ctxts.size();
    if (num != ctxts.size()) {
        printf("Error! MPEncVectors operator += !\n");
        return *this;
    }

    for (long i = 0; i < num; i++) {
        ctxts[i] += oth.ctxts[i];
    }

    return *this;
}

MPEncVector& MPEncVector::operator-=(const MPEncVector &oth)
{
    auto num = oth.ctxts.size();
    if (num != ctxts.size()) {
        printf("Error! MPEncVectors operator -= !\n");
        return *this;
    }

    for (long i = 0; i < num; i++) {
        ctxts[i] -= oth.ctxts[i];
    }

    return *this;
}

MPEncVector& MPEncVector::dot(const MPEncVector &oth,
                              const MPEncArray &ea)
{
    auto num = oth.ctxts.size();
    assert(num == ctxts.size());

    for (long i = 0; i < num; i++) {
       ctxts[i].dot(oth.ctxts[i], *ea.get(i));
    }

    return *this;
}

MPEncVector& MPEncVector::addConstant(const MDL::Vector<long> &con,
                                      const MPEncArray &ea)
{
    if (ea.arrayNum() != partsNum()) {
        printf("Warnning! MPEncVector addConstant!\n");
        return *this;
    }

    for (long i = 0; i < ea.arrayNum(); i++) {
        NTL::ZZX poly;

        if (ea.get(i)->size() > con.dimension()) {
            auto tmp(con);
            tmp.resize(ea.get(i)->size());
            ea.get(i)->encode(poly, tmp);
        } else {
            ea.get(i)->encode(poly, con);
        }
        get(i).addConstant(poly);
    }
    return *this;
}

MPEncVector& MPEncVector::mulConstant(const MDL::Vector<long> &con,
                                      const MPEncArray &ea)
{
    if (ea.arrayNum() != partsNum()) {
        printf("Warnning! MPEncVector mulConstant!\n");
        return *this;
    }

    for (long i = 0; i < ea.arrayNum(); i++) {
        NTL::ZZX poly;

        if (ea.get(i)->size() > con.dimension()) {
            auto tmp(con);
            tmp.resize(ea.get(i)->size());
            ea.get(i)->encode(poly, tmp);
        } else {
            ea.get(i)->encode(poly, con);
        }
        get(i).multByConstant(poly);
    }
    return *this;
}

void MPEncVector::reLinearize()
{
    for (auto &ctxt : ctxts) ctxt.reLinearize();
}
