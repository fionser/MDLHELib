#include "MPContext.hpp"
#include "MPSecKey.hpp"
#include <vector>
#include <thread>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

MPSecKey::MPSecKey(const MPContext &context)
{
    std::vector<std::thread> worker;
    std::atomic<size_t> counter(0);
    const size_t num = context.partsNum();
    auto job = [this, &counter, &num, &context]() {
        size_t i;
        while ((i = counter.fetch_add(1)) < num) {
            skeys[i] = std::make_shared<FHESecKey>(*(context.get(i)));
            skeys[i]->GenSecKey(64);
            ::addSome1DMatrices(*skeys[i]);
        }
    };
   
    skeys.resize(context.partsNum());

    for (long wr = 0; wr < WORKER_NR; wr++) worker.push_back(std::thread(job));

    for (auto &wr : worker) wr.join();
}
