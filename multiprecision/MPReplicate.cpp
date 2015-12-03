#include "MPReplicate.h"
#include "MPEncVector.hpp"
#include "MPEncArray.hpp"
#include "fhe/replicate.h"
#include <vector>
#include <thread>
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
