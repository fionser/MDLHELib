#include "protocol/PCA.hpp"
#include "algebra/NDSS.h"
#include "utils/FHEUtils.hpp"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include "fhe/EncryptedArray.h"
#include <thread>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif
long BATCH_SZE = 100;
MDL::Timer totalTimer, encTimer;

MDL::EncMatrix encrypt(const MDL::Matrix<double> &X,
                       const FHEPubKey &pk,
                       const EncryptedArray &ea,
                       MDL::Timer &totalTimer,
                       MDL::Timer &encTimer)
{
    const long divider = 10;
    const long rows = X.rows();
    long n = (rows + BATCH_SZE - 1) / BATCH_SZE;
    std::vector<MDL::Matrix<long>> local_sigma(n);
    totalTimer.end();
    for (int i = 0; i < n; i++) {
        long from = std::min(rows - 1, i * BATCH_SZE);
        long to = std::min(rows - 1, from + BATCH_SZE - 1);
        if (to + 1 < n && to - n < BATCH_SZE / 10) {
            to = n - 1;
            n -= 1;
        }
        auto submat = X.submatrix(from, to);
        auto transpose = submat.transpose();
        local_sigma[i] = transpose.dot(submat).div(divider);
    }

    std::vector<MDL::EncMatrix> encMat(n, pk);
    std::vector<std::thread> workers;
    std::atomic<long> counter(0);

    totalTimer.start();
    encTimer.start();
    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&counter, &ea, &n,
                                                &encMat, &local_sigma]() {
                                                long next;
                                                while ((next = counter.fetch_add(1)) < n) {
                                                encMat[next].pack(local_sigma[next], ea);
                                                } })));
    }
    for (auto &&wr : workers) wr.join();
    encTimer.end();

    for (long i = 1; i < n; i++) encMat[0] += encMat[i];
    totalTimer.end();
    return encMat[0];
}

int main(int argc, char *argv[]) {
    long m, p, r, L;
    ArgMapping argmap;
    MDL::Timer keyTimer;
    std::string file;
    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("B", BATCH_SZE, "BATCH_SZE");
    argmap.parse(argc, argv);

    keyTimer.start();
    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey  sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    keyTimer.end();
    std::vector<std::string> files{};
    for (auto &file : files) {
        MDL::Timer totalTimer, encTimer, iterationTimer;
        auto mat = load_csv_d(file);
        long rows = mat.rows();
        long dimension = mat.cols();
        auto truePrincipleComp = mat.maxEigenValue() / rows;
        printf("Dimension %ld, %f\n", dimension, truePrincipleComp);

        totalTimer.start();
        auto encMat = encrypt(mat, pk, ea, totalTimer, encTimer);
        iterationTimer.start();
        auto vecs = MDL::runPCA(encMat, ea, pk, dimension);
        iterationTimer.end();

        MDL::Vector<NTL::ZZX> vec(dimension), vec2(dimension);
        vecs.first.unpack(vec, sk, ea, true);
        vecs.second.unpack(vec2, sk, ea, true);
        auto approx = vec.L2() / vec2.L2() / rows * 10;
        totalTimer.end();
        std::cout << file << " Enc " << encTimer.second() << " iteation " << iterationTimer.second()
            << " total: " << totalTimer.second() << " error: "
            << std::abs(approx - truePrincipleComp) / truePrincipleComp << std::endl;
    }
    return 0;
}
