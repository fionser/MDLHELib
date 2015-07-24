#include "algebra/NDSS.h"
#include "utils/FHEUtils.hpp"
#include "utils/timer.hpp"
#include "fhe/EncryptedArray.h"
#include "fhe/NumbTh.h"
#include "utils/FileUtils.hpp"
#define TIMING_THIS(codes_block)                           \
    do {                                                   \
        MDL::Timer timer;                                  \
        timer.start();                                     \
        codes_block                                        \
        timer.end();                                       \
        printf("This codes run %f sec\n", timer.second()); \
    } while (0)

int main(int argc, char *argv[]) {
    long m, p, r, L;
    ArgMapping argmap;
    MDL::Timer keyTimer, totalTimer;

    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.parse(argc, argv);
    MDL::Matrix<long> mat = load_csv("covariance3.data");
    long dimension = mat.cols();
    MDL::Vector<long> vec(dimension), vec2(dimension);
    printf("Dimension %ld\n", dimension);

    for (size_t i = 0; i < dimension; i++) vec[i] = NTL::RandomBnd(3);
    std::cout << mat.maxEigenValue() << std::endl;
    totalTimer.start();
    keyTimer.start();
    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey  sk(context);
    sk.GenSecKey(64);	
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;
    auto G = context.alMod.getFactorsOverZZ()[0];
    keyTimer.end();
    printf("KeyGen %f\n", keyTimer.second());

    EncryptedArray ea(context, G);
        MDL::EncMatrix encMat(pk);
    MDL::EncVector encVec(pk), encVec2(pk);
    TIMING_THIS(
        encMat.pack(mat, ea);
        encVec.pack(vec, ea);
        );

    TIMING_THIS(encVec  = encMat.column_dot(encVec, ea, dimension); );
    TIMING_THIS(encVec  = encMat.column_dot(encVec, ea, dimension); );
    TIMING_THIS(encVec  = encMat.column_dot(encVec, ea, dimension); );
    TIMING_THIS(encVec2 = encMat.column_dot(encVec, ea, dimension); );

    {
        MDL::Vector<NTL::ZZX> vec(dimension), vec2(dimension);
        TIMING_THIS(
            encVec.unpack(vec, sk, ea);
            encVec2.unpack(vec2, sk, ea);
            );
        std::cout << vec2.L2() / vec.L2() << std::endl;
    }
    totalTimer.end();
    printf("total %f\n", totalTimer.second());
}
