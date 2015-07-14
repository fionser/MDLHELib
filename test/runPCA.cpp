#include "algebra/NDSS.h"
#include "utils/FHEUtils.hpp"
#include "utils/Timer.hpp"
#include "fhe/EncryptedArray.h"
#include "fhe/NumbTh.h"
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
    MDL::Timer timer;

    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.parse(argc, argv);
    timer.start();
    FHEcontext context(m, p, r);
    FHESecKey  sk(context);
    load_FHE_setting("fhe_setting", context, sk);
    FHEPubKey pk = sk;
    auto G = context.alMod.getFactorsOverZZ()[0];
    timer.end();
    printf("Setup %f\n", timer.second());

    EncryptedArray ea(context, G);
    long dimension = ea.size();
    MDL::Matrix<long> mat(dimension, dimension);
    MDL::Vector<long> vec(dimension), vec2(dimension);
    printf("Dimension %ld\n", dimension);

    for (size_t i = 0; i < dimension; i++) {
        vec[i] = NTL::RandomBnd(3);

        for (size_t j = 0; j < dimension; j++) {
            mat[i][j] = NTL::RandomBnd(1000);
        }
    }
    std::cout << mat.maxEigenValue() << std::endl;

    MDL::EncMatrix encMat(pk);
    MDL::EncVector encVec(pk);
    TIMING_THIS(
        encMat.pack(mat, ea);
        encVec.pack(vec, ea);
        );

    TIMING_THIS(encVec = encMat.dot(encVec, ea); );
    TIMING_THIS(encVec = encMat.dot(encVec, ea); );
    TIMING_THIS(encVec = encMat.dot(encVec, ea); );
    auto encVec2 = encMat.dot(encVec, ea);

    {
        MDL::Vector<NTL::ZZX> vec(dimension), vec2(dimension);
        encVec.unpack(vec, sk, ea);
        encVec2.unpack(vec2, sk, ea);
        std::cout << vec2.L2() / vec.L2() << std::endl;
    }
}
