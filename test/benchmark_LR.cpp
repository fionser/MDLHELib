#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include <thread>
#include "protocol/LR.hpp"
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

std::pair<MDL::EncMatrix, MDL::EncVector>
encrypt(const MDL::Matrix<double> &X,
        const MDL::Vector<double> &Y,
        const FHEPubKey &pk,
        const EncryptedArray &ea)
{
    const long BATCH_SZE = 100;
    const long divider = 10;
    const long rows = X.rows();
    const long n = (rows + BATCH_SZE - 1) / BATCH_SZE;
    std::vector<MDL::Matrix<long>> local_sigma(n);
    std::vector<MDL::Vector<long>> local_xy(n);
    for (int i = 0; i < n; i++) {
        long from = std::min(rows - 1, i * BATCH_SZE);
        long to = std::min(rows - 1, from + BATCH_SZE - 1);
        if (to + 1 < n && to - n < BATCH_SZE / 10) {
			to = n - 1;
			i += 1;
		}
        auto submat = X.submatrix(from, to);
		auto transpose = submat.transpose();
        auto subvec = Y.subvector(from, to);
        local_sigma[i] = transpose.dot(submat).div(divider);
        local_xy[i] = transpose.dot(subvec).div(divider);
    }
    MDL::Timer timer;
    std::vector<MDL::EncMatrix> encMat(n, pk);
    std::vector<MDL::EncVector> encVec(n, pk);
    std::vector<std::thread> workers;
    std::atomic<long> counter(0);
    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&counter, &ea, &n, &encVec,
                                                &encMat, &local_sigma, &local_xy]() {
                                                long next;
                                                while ((next = counter.fetch_add(1)) < n) {
                                                encMat[next].pack(local_sigma[next], ea);
                                                encVec[next].pack(local_xy[next], ea);
                                                } })));
    }
    for (auto &&wr : workers) wr.join();
    for (long i = 1; i < n; i++) {
        encMat[0] += encMat[i];
        encVec[0] += encVec[i];
    }

    return { encMat[0], encVec[0] };
}

void benchmarkLR(const FHEPubKey &pk,
                 const FHESecKey &sk,
                 const EncryptedArray &ea)
{
    MDL::Matrix<double> D = load_csv_d("all_float_data");
    auto X = D.submatrix(0, -1, 0, -2);
    auto Y = D.submatrix(0, -1,  -1, -1).vector();
    auto Xy = X.transpose().dot(Y).reduce(10.0);
    auto sigma = X.transpose().dot(X).reduce(10.0);
    long mu = static_cast<long>(sigma.maxEigenValue());
    auto trueW = sigma.inverse().dot(Xy);

    const long dimension = sigma.cols();
    MDL::Matrix<long> muR0 = MDL::eye(dimension);
    MDL::MatInverseParam params{ pk, ea, dimension };

    MDL::Timer timer;
    timer.start();
    auto pair = encrypt(X, Y, pk, ea);
    MDL::EncMatrix Q(pair.first);
    MDL::EncVector eXy(pair.second);

    auto M = MDL::inverse(Q, mu, params);
    auto W = M.column_dot(eXy, ea, dimension);
    timer.end();
    printf("Enc -> Inverse %f\n", timer.second());

    MDL::Vector<long> result;
    W.unpack(result, sk, ea, true);
    for (int i = 0; i < MDL::LR::ITERATION; i++) mu = mu * mu;
    auto w = result.reduce(double(mu));
    timer.end();

    std::cout << w << std::endl;
    std::cout << trueW << std::endl;
    printf("Total %f\n", timer.second());
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
    printf("slot %ld\n", ea.size());
    benchmarkLR(pk, sk, ea);
}
