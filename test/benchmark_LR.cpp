#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#define TIMING_THIS(codes_block)                           \
    do {                                                   \
        MDL::Timer timer;                                  \
        timer.start();                                     \
        codes_block                                        \
        timer.end();                                       \
        printf("This codes run %f sec\n", timer.second()); \
    } while (0)

void benchmarkLR(const FHEPubKey &pk,
                 const FHESecKey &sk,
                 const EncryptedArray &ea)
{
    const long dimension = 5;
    const long mu = 3255;
    MDL::Matrix<long> muR0 = MDL::eye(dimension);
    MDL::Matrix<long> sigma(dimension, dimension);
    MDL::EncMatrix M(pk), R(pk);

    sigma[0] = vector<long>{3255, -249, 118, 252, 188};
    sigma[1] = vector<long>{-249, 3256, -140, 1, -33};
    sigma[2] = vector<long>{118, -140,  3256, 399, 260};
    sigma[3] = vector<long>{252,  1, 399, 3256, -102};
    sigma[4] = vector<long>{188,  -33, 260, -102, 3255};
    std::cout << muR0 << std::endl;
    std::cout << sigma << std::endl;
    M.pack(sigma, ea);
    R.pack(muR0, ea);
    long MU = mu;
    MDL::Timer timer;
    timer.start();
    for (int itr = 0; itr < 2; itr++) {
        auto tmpR(R), tmpM(M);
        MDL::Vector<long> mag(ea.size(), 2 * MU);
        tmpR.dot(M, ea, dimension);
        R.multByConstant(mag.encode(ea));
        R -= tmpR;

        tmpM.dot(M, ea, dimension);
        M.multByConstant(mag.encode(ea));
        M -= tmpM;
        MU *= MU;
    }
    timer.end();
    std::cout << MU << std::endl;
    printf("Iteration %f\n", timer.second());
    M.unpack(muR0, sk, ea, true);
    std::cout << muR0.reduce(double(MU)) << std::endl;

    R.unpack(muR0, sk, ea, true);
    std::cout << muR0.reduce(double(MU)) << std::endl;
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
