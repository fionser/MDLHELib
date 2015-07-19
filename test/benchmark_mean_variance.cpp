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
long WORKER_NR = 8;
#else // ifdef FHE_THREADS
long WORKER_NR = 1;
#endif // ifdef FHE_THREAD

MDL::EncVector encrypt_variance(const MDL::Matrix<long>& data,
                                const FHEPubKey        & pk,
                                const EncryptedArray   & ea)
{
    MDL::EncVector sq_sum(pk), sum_sq(pk);
    MDL::EncVector encRow(pk);
    NTL::ZZX   N(data.rows());
    MDL::Timer timer, encTimer, evlTimer;

    timer.start();

    for (size_t row = 0; row < data.rows(); row++) {
        encTimer.start();
        encRow.pack(data[row], ea);
        encTimer.end();
        evlTimer.start();

        if (row == 0) {
            sum_sq = encRow;
            encRow.square();
            sq_sum = encRow;
        } else {
            sum_sq += encRow;
            encRow.square();
            sq_sum += encRow;
        }
        evlTimer.end();
    }
    evlTimer.start();
    sq_sum.multByConstant(N);
    sum_sq.square();
    sq_sum -= sum_sq;
    evlTimer.end();
    timer.end();

    printf("Encrypt & Variance of %ld records totally costed %fs(%f/%f)\n",
           data.rows(),
           timer.second(), encTimer.second(), evlTimer.second());
    return sq_sum;
}

std::vector<MDL::EncVector>encrypt(const MDL::Matrix<long>& data,
                                   const FHEPubKey        & pk,
                                   const EncryptedArray   & ea,
                                   long                     from = 0,
                                   long                     to = 0)
{
    MDL::Timer timer;
    std::vector<MDL::EncVector> ctxts(to - from, pk);
    std::vector<std::thread>    workers;
    std::atomic<size_t> counter(from);

    to = to == 0 ? data.rows() : to;
    timer.start();

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&data, &ea, &to,
                                                 &counter, &ctxts]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < to) {
                ctxts[next].pack(data[next], ea);
            }
        })));
    }

    for (auto && wr : workers) wr.join();
    timer.end();
    printf("Encrypt %ld data with %ld workers costed %f sec\n", to - from,
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
    printf("Sum %zd data with %ld workers costed %f sec\n", ctxts.size(),
           WORKER_NR, timer.second());
    return partials[0];
}

MDL::EncVector squareSum(std::vector<MDL::EncVector>& ctxts)
{
    MDL::Timer timer;
    std::atomic<size_t> counter(0);
    std::vector<std::thread> workers;

    timer.start();

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&ctxts, &counter]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < ctxts.size()) {
                ctxts[next].square();
            }
        })));
    }

    for (auto && wr : workers) wr.join();
    auto sq_sum = mean(ctxts);
    timer.end();
    printf("Square Sum costed %f sec\n", timer.second());
    return sq_sum;
}

MDL::EncVector variance(const MDL::Matrix<long>& data,
                        const EncryptedArray   & ea,
                        const FHEPubKey        & pk)
{
    MDL::EncVector square_sum(pk), sum_square(pk);
    NTL::ZZX N(data.rows());
    const long BATCH_SIZE = 5000;
    for (long part = 0; part * BATCH_SIZE < data.rows(); part++) {
        long from  = std::min<long>(part * BATCH_SIZE, data.rows());
        long to    = std::min<long>(from + BATCH_SIZE, data.rows());
        auto ctxts = encrypt(data, pk, ea, from, to);

        if (part > 0) {
            sum_square += mean(ctxts);
            square_sum += squareSum(ctxts);
        } else {
            sum_square = mean(ctxts);
            square_sum = squareSum(ctxts);
        }
    }

    sum_square.square();
    square_sum.multByConstant(N);
    square_sum -= sum_square;
    return square_sum;
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

    // auto ctxts  = encrypt(data, pk, ea);

    // {
    //     MDL::Vector<long> ret;
    //     auto summaiton = mean(ctxts);
    //     summaiton.unpack(ret, sk, ea);
    //     std::cout << ret << std::endl;
    //     std::cout << result[0] << std::endl;
    // }

    {
        MDL::Vector<long> ret;
        auto var = variance(data, ea, pk);
        var.unpack(ret, sk, ea);
        std::cout << ret << std::endl;
        std::cout << result[1] << std::endl;
    }
    return 0;
}
