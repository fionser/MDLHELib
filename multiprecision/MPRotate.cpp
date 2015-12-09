#include "MPRotate.h"
#include "Multiprecision.h"
#include <thread>
#include <vector>
#include <map>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

static void totalSums(const EncryptedArray& ea, long blockSize, Ctxt& ctxt);

void rotate(MPEncVector &vec,
            const MPEncArray &ea,
            const long r)
{
    auto parts = vec.partsNum();
    std::vector<std::thread> worker;
    std::atomic<long> counter(0);
    auto job = [&parts, &counter, &ea, &vec, &r]() {
        long i;
        while ((i = counter.fetch_add(1)) < parts) {
            ea.get(i)->rotate(vec.get(i), r);
        }
    };

    if (parts != ea.arrayNum()) return;

    for (long wr = 0; wr < WORKER_NR; wr++) {
        worker.push_back(std::thread(job));
    }

    for (auto &&wr : worker) wr.join();
}

void totalSums(MPEncVector &vec, const MPEncArray &ea, const long blockSize) {
    auto parts = vec.partsNum();
    std::vector<std::thread> worker;
    std::atomic<long> counter(0);
    auto job = [&]() {
        long i;
        while ((i = counter.fetch_add(1)) < parts) {
            totalSums(*ea.get(i), blockSize, vec.get(i));
        }
    };

    if (parts != ea.arrayNum()) return;

    for (long wr = 0; wr < WORKER_NR; wr++) {
        worker.push_back(std::thread(job));
    }

    for (auto &&wr : worker) wr.join();
}

static void totalSums(const EncryptedArray& ea, long blockSize, Ctxt& ctxt) {
    if (ea.size() % blockSize != 0)
        printf("WARNINNG! the totalSums might be wrong!: blockSize %ld with %zd slots\n",
               blockSize, ea.size());
    long n = ea.size();
    n = n / blockSize;
    if (n == 1) return;
    Ctxt orig = ctxt;

    long k = NumBits(n);
    long e = 1;

    for (long i = k-2; i >= 0; i--) {
        Ctxt tmp1 = ctxt;
        ea.rotate(tmp1, e * blockSize);
        ctxt += tmp1; // ctxt = ctxt + (ctxt >>> e)
        e = 2 * e;

        if (bit(n, i)) {
            Ctxt tmp2 = orig;
            ea.rotate(tmp2, e * blockSize);
            ctxt += tmp2; // ctxt = ctxt + (orig >>> e)
            // NOTE: we could have also computed
            // ctxt =  (ctxt >>> e) + orig, however,
            // this would give us greater depth/noise
            e += 1;
        }
    }
}


