#include "multiprecision/Multiprecision.h"
#include "protocol/PCA.hpp"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include <thread>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif
MDL::Matrix<long> load_data(const std::string &file,
                            long Magnifier = 100,
                            long recordsToProcess = 0)
{
    auto raw = load_csv_d(file, recordsToProcess);
    raw *= Magnifier;
    return raw.div(1.0);
}

MPEncMatrix summation(std::vector<MPEncMatrix> &encMats)
{
    std::atomic<long> counter(WORKER_NR);
    std::vector<std::thread> workers;
    auto addJob = [&encMats](std::atomic<long> &counter,
                             MPEncMatrix &sum) {
        long next;
        auto size = encMats.size();
        while ((next = counter.fetch_add(1)) < size) {
            sum += encMats[next];
        }
    };

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::thread(addJob,
                                      std::ref(counter),
                                      std::ref(encMats[wr])));
    }

    for (auto &&wr : workers) wr.join();

    for (long wr = 1; wr < WORKER_NR; wr++) encMats[0] += encMats[wr];

    return encMats[0];
}

MPEncMatrix encryptAndSum(MDL::Timer &encTimer,
                          MDL::Timer &evalTimer,
                          const MDL::Matrix<long> &X,
                          const MPPubKey &pk,
                          const MPEncArray &ea)
{
    const long BATCH = 2000;
    std::vector<std::thread> worker;
    auto totalRows = X.rows();
    MPEncMatrix result(pk);

    result.pack(covariance(X[0], X[0]), ea);
    for (long from = 1; from < totalRows; from += BATCH) {
        const long to = std::min<long>(from + BATCH, totalRows);
        std::vector<MPEncMatrix> encMats(to - from, pk);
        std::atomic<long> counter(from);
        std::vector<std::thread> workers;
        auto encryptJob = [&]() {
            long next;
            while ((next = counter.fetch_add(1)) < to) {
                auto xx = covariance(X[next], X[next]);
                encMats[next - from].pack(xx, ea);
            }
        };

        encTimer.start();
        for (long wr = 0; wr < WORKER_NR; wr++) {
            workers.push_back(std::thread(encryptJob));
        }
        for (auto &&wr : workers) wr.join();
        encTimer.end();

        evalTimer.start();
        result += summation(encMats);
        evalTimer.end();
    }
    return result;
}

int main(int argc, char *argv[]) {
    ArgMapping argMap;
    long m, p, r, L, N = 0, P = 0, M = 100;
    argMap.arg("m", m, "m: cyclotomic");
    argMap.arg("p", p, "p: plaintext space");
    argMap.arg("r", r, "r: lifting");
    argMap.arg("L", L, "L: Levels");
    argMap.arg("N", N, "N: How many record to use");
    argMap.arg("P", P, "P: How many primes to use");
    argMap.arg("M", M, "M: Magnifier");
    argMap.parse(argc, argv);

    auto X = load_data("float_adult.data", M, N);

    MPContext context(m, p, r, P);
    context.buildModChain(L);
    MPSecKey sk(context);
    MPPubKey pk(sk);
    MPEncArray ea(context);
    MDL::Timer encTimer, evalTimer;

    auto encMat = encryptAndSum(encTimer, evalTimer, X, pk, ea);
    MDL::Matrix<ZZ> result;
    encMat.unpack(result, sk, ea, true);
    std::cout << result << std::endl;
    return 0;
}
