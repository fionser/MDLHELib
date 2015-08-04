#include "MPRotate.h"
#include "Multiprecision.h"
#include <thread>
#include <vector>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

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
