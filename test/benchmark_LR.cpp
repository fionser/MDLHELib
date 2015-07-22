#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "algebra/NDSS.h"
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
    const long dimension = 8;
    const long mu = 8;
    MDL::Matrix<long> muR0 = MDL::eye(dimension);
    MDL::Matrix<long> sigma(dimension, dimension);
    MDL::EncMatrix M(pk), R(pk);

    sigma.random(3);
    std::cout << sigma << std::endl;
    std::cout << sigma.maxEigenValue() << std::endl;
    std::cout << sigma.inverse() << std::endl;
    M.pack(sigma, ea);
    R.pack(muR0, ea);
    long MU = mu;
    for (int itr = 0; itr < 4; itr++) {
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

    M.unpack(muR0, sk, ea, true);
    std::cout << muR0 << std::endl;

    R.unpack(muR0, sk, ea, true);
    std::cout << muR0 << std::endl;
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
