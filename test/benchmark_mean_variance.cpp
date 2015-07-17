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

#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else // ifdef FHE_THREADS
const long WORKER_NR = 1;
#endif // ifdef FHE_THREAD

std::vector<MDL::EncVector>encrypt(const MDL::Matrix<long>& data,
                                   const FHEPubKey        & pk,
                                   const EncryptedArray   & ea,
                                   long                     rows_to_proces = 0)
{
    MDL::Timer timer;
    std::vector<MDL::EncVector> ctxts(data.rows(), pk);
    std::vector<std::thread>    workers;
    std::atomic<size_t> counter(0);

    rows_to_proces = rows_to_proces == 0 ? data.rows() : rows_to_proces;
    timer.start();

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&data, &ea, &rows_to_proces,
                                                 &counter, &ctxts]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < rows_to_proces) {
                ctxts[next].pack(data[next], ea);
            }
        })));
    }

    for (auto && wr : workers) wr.join();
    timer.end();
    printf("Encrypt %ld data with %ld workers costed %f sec\n", rows_to_proces,
           WORKER_NR, timer.second());
    return ctxts;
}

MDL::EncVector mean(const std::vector<MDL::EncVector>& ctxts)
{
    std::vector<std::thread>    workers;
    std::vector<MDL::EncVector> partials(WORKER_NR, ctxts[0].getPubKey());
    std::atomic<size_t> counter(WORKER_NR);
    MDL::Timer timer;

    timer.start();

    for (long i = 0; i < WORKER_NR; i++) {
        partials[i] = ctxts[i];
        workers.push_back(std::move(std::thread([&counter, &ctxts]
                                                    (MDL::EncVector& ct) {
            size_t next;

            while ((next = counter.fetch_add(1)) < ctxts.size()) {
                ct += ctxts[next];
            }
        }, std::ref(partials[i]))));
    }

    for (auto && wr : workers) {
        wr.join();
    }

    for (long i = 1; i < WORKER_NR; i++) {
        partials[0] += partials[i];
    }
    timer.end();
    printf("Mean %zd data with %ld workers costed %f sec\n", ctxts.size(),
           WORKER_NR, timer.second());
    return partials[0];
}

MDL::EncVector variance(std::vector<MDL::EncVector>& ctxts)
{
    MDL::Timer timer;
    NTL::ZZX   N(ctxts.size());
    std::atomic<size_t> counter(0);
    std::vector<std::thread> workers;

    timer.start();
    WORKER_NR = 1;
    auto sum_sq = mean(ctxts);
    sum_sq.square();
    MDL::Timer mult_timer;
    mult_timer.start();
    WORKER_NR = 8;

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&ctxts, &counter]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < ctxts.size()) {
                ctxts[next].square();
            }
        })));
    }

    for (auto && wr : workers) wr.join();
    mult_timer.end();
    printf("Square costed %f sec\n", mult_timer.second());
    WORKER_NR = 1;
    auto sq_sum = mean(ctxts);
    WORKER_NR = 8;
    sq_sum.multByConstant(N);
    sq_sum.addCtxt(sum_sq, true);
    timer.end();
    printf("Variance %zd data with %ld workers costed %f sec\n", ctxts.size(),
           WORKER_NR, timer.second());
    return sq_sum;
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

    auto data   = load_csv("adult.data", 100);
    auto result = load_csv("adult_result");
    auto ctxts  = encrypt(data, pk, ea);

    // {
    //     MDL::Vector<long> ret;
    //     auto summaiton = mean(ctxts);
    //     summaiton.unpack(ret, sk, ea);
    //     std::cout << ret << std::endl;
    //     std::cout << result[0] << std::endl;
    // }

    {
        MDL::Vector<long> ret;

        auto var = variance(ctxts);
        var.unpack(ret, sk, ea);
        std::cout << ret << std::endl;
        std::cout << result[1] << std::endl;
    }
    return 0;
}
