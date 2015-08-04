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

MPEncVector summation(std::vector<MPEncVector> &encMats)
{
    std::atomic<long> counter(WORKER_NR);
    std::vector<std::thread> workers;
    auto addJob = [&encMats](std::atomic<long> &counter,
                             MPEncVector &sum) {
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
    const long BATCH = 500;
    std::vector<std::thread> worker;
    auto totalRows = X.rows();
    MPEncVector result(pk);

    result.pack(covariance(X[0], X[0]).vector(), ea);
    for (long from = 1; from < totalRows; from += BATCH) {
        const long to = std::min<long>(from + BATCH, totalRows);
        std::vector<MPEncVector> encMats(to - from, pk);
        std::atomic<long> counter(from);
        std::vector<std::thread> workers;
        auto encryptJob = [&]() {
            long next;
            while ((next = counter.fetch_add(1)) < to) {
                auto xx = covariance(X[next], X[next]).vector();
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

    evalTimer.start();
    auto cols = X.cols();
    std::vector<MPEncVector> rows(cols, result);
    for (long r = 0; r < cols; r++) {
        MDL::Vector<long> masking(ea.slots());
        for (long i = 0; i < cols; i++) {
            masking[r * cols + i] = 1;
        }
        rows[r].mulConstant(masking, ea);
        rotate(rows[r], ea, -r * cols);
    }
    evalTimer.end();
    return MPEncMatrix(pk, rows);
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
    std::cout << "slots " << ea.slots() << std::endl;
    MDL::Timer encTimer, evalTimer, decTimer;

    auto encMat = encryptAndSum(encTimer, evalTimer, X, pk, ea);
    MDL::Matrix<NTL::ZZ> mmmt;
    encMat.unpack(mmmt, sk, ea, true);
    std::cout << mmmt << std::endl;
    return 0;
    // evalTimer.start();
    // auto pca = MDL::runPCA(encMat, ea, pk);
    // evalTimer.end();
    //
    // decTimer.start();
    // MDL::Vector<ZZ> v1, v2;
    // pca.first.unpack(v1, sk, ea);
    // pca.second.unpack(v2, sk, ea);
    // decTimer.end();
    // std::cout << v1.L2() / v2.L2() / M / M << std::endl;
    return 0;
}
