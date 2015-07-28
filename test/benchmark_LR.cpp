#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include <thread>
#include "protocol/LR.hpp"
void benchmarkLR(const FHEPubKey &pk,
                 const FHESecKey &sk,
                 const EncryptedArray &ea)
{
    MDL::Matrix<double> D = load_csv_d("all_float_data");
    auto X = D.submatrix(0, -1, 0, -2);
    auto Y = D.submatrix(0, -1,  -1, -1).vector();
    auto Xy = X.transpose().dot(Y).reduce(10.0);
    auto Xy_round = Xy.div(1.0);
    auto sigma = X.transpose().dot(X).reduce(10.0);
    auto sigma_round = sigma.div(1.0);
    long mu = static_cast<long>(sigma.maxEigenValue());
    auto trueW = sigma.inverse().dot(Xy);

    const long dimension = sigma.cols();
    MDL::Matrix<long> muR0 = MDL::eye(dimension);
    MDL::MatInverseParam params{ pk, ea, dimension };
    MDL::EncMatrix Q(pk);
    MDL::EncVector eXy(pk);
    MDL::Timer timer;

    timer.start();
    Q.pack(sigma_round, ea);
    eXy.pack(Xy_round, ea);

    auto M = MDL::inverse(Q, mu, params);
    auto W = M.column_dot(eXy, ea, dimension);
    timer.end();
    printf("Enc -> Inverse %f\n", timer.second());

    MDL::Vector<long> result;
    W.unpack(result, sk, ea, true);
    for (int i = 0; i < MDL::LR::ITERATION; i++) mu = mu * mu;
    auto w = result.reduce(double(mu));
    std::cout << w << std::endl;
    std::cout << trueW << std::endl;
    timer.end();
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
