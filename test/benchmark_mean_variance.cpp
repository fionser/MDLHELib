#include <fhe/FHEContext.h>
#include <fhe/FHE.h>
#include <fhe/NumbTh.h>
#include <fhe/EncryptedArray.h>
#include <utils/FileUtils.hpp>
#include <utils/timer.hpp>
#include <algebra/NDSS.h>
#include <thread>
#include <atomic>
#include <vector>
std::vector<MDL::EncVector>encrypt(const MDL::Matrix<long>& data,
                                   const FHEPubKey        & pk,
                                   const EncryptedArray   & ea)
{
    #ifdef FHE_THREADS
    const long WORKER_NR = 8;
    #else // ifdef FHE_THREADS
    const long WORKER_NR = 1;
    #endif // ifdef FHE_THREADS
    MDL::Timer timer;
    std::vector<MDL::EncVector> ctxts(data.rows(), pk);
    std::vector<std::thread>    workers;
    std::atomic<size_t> counter(0);
    timer.start();

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&data, &ea,
                                                 &counter, &ctxts]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < data.rows()) {
                ctxts[next].pack(data[next], ea);
            }
        })));
    }

    for (auto && wr : workers) wr.join();
    timer.end();
    printf("Encrypt %ld data with %ld workers costed %f sec\n", data.rows(),
           WORKER_NR, timer.second());
    return ctxts;
}

MDL::EncVector mean(const std::vector<MDL::EncVector>& ctxts)
{
    MDL::EncVector summation(ctxts.front());
    MDL::Timer    timer;
    timer.start();

    for (auto itr = ctxts.begin() + 1; itr != ctxts.end(); itr++) {
        summation += *itr;
    }
    timer.end();
    printf("Mean %zd data with 1 workers costed %f sec\n", ctxts.size(),
           timer.second());
    return summation;
}

int main(int argc, char *argv[]) {
    long m, p, r, L;
    ArgMapping argmap;

    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.parse(argc, argv);

    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;

    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);

    auto data   = load_csv("adult.data");
    auto result = load_csv("adult_result");

    auto ctxts = encrypt(data, pk, ea);
    {
        MDL::Vector<long> ret;
        auto summaiton = mean(ctxts);
        summaiton.unpack(ret, sk, ea);
        for (size_t i = 0; i < result.cols(); i++) {
            assert(ret[i] == result[0][i]);
        }
    }
    return 0;
}
