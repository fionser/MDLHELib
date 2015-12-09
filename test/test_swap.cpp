#if 0
#include <fhe/FHEContext.h>
#include <fhe/FHE.h>
#include <fhe/NumbTh.h>
#include <fhe/EncryptedArray.h>
#endif
#include "multiprecision/Multiprecision.h"
#include "utils/timer.hpp"
#include <algebra/NDSS.h>

#include <cstring>
#include <vector>
void test_dot2(const MPEncMatrix &X,
               const MPEncMatrix &Y,
               const MPPubKey &pk,
               const MPSecKey &sk,
               const MPEncArray &ea,
               long D) {
    MDL::Timer timer;

    timer.start();
    auto flattenY = Y.flatten(ea, D);
    std::vector<MPEncVector> ctxts(D, pk);
    // for (int r = 0; r < X.rowsNum(); r++) {
    for (int r : {0}) {
        auto oneRow = repeat(X.get(r), ea, pk, D, D);
        MPEncVector result(pk);
        rearrange(ctxts[r], oneRow, ea, D, D);

        ctxts[r].multiplyBy(flattenY);
        totalSums(ctxts[r], ea, D);
    }
    timer.end();

    MDL::Matrix<NTL::ZZ> mat;
    MPEncMatrix XY(ctxts);
    XY.unpack(mat, sk, ea);
    for (int i = 0; i < D; i++) {
        for (int j = 0; j < D; j++)
            std::cout << mat[i][j] << " ";
        std::cout << "\n";
    }

    printf("dot2 of %ld dimension cost %fs\n", D, timer.second());
}

void test_dot3(const MPEncMatrix &X,
               MPEncMatrix &Y,
               const MPPubKey &pk,
               const MPSecKey &sk,
               const MPEncArray &ea,
               long D) {
    MDL::Timer timer;
    MPEncVector result(pk);
    MDL::Vector<NTL::ZZ> dec;
    timer.reset();
    rearrange2(result, X, D, ea, pk);
    Y.repeatEachRow(D, D, ea, pk);
    auto fY = Y.flatten(ea, D * D);
    result *= fY;
    totalSums(result, ea, D * D);
    result.reLinearize();
    MDL::Vector<long> mask(D * D, 1);
    result.mulConstant(mask, ea);
    timer.end();

    result.unpack(dec, sk, ea);
    std::cout << dec << "\n";
    printf("cost %fs with noise ", timer.second());
    std::cout << result.get(0).getNoiseVar() << "\n";
}

int main(int argc, char *argv[]) {
    long m, p, r, L, D;
    ArgMapping argmap;
    MDL::Timer timer;
    argmap.arg("m", m, "m");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("L", L, "L");
    argmap.arg("D", D, "D");
    argmap.parse(argc, argv);
    timer.start();
    MPContext context(m, p, r, 1);
    context.buildModChain(L);
    MPSecKey sk(context);
    MPPubKey pk = sk;
    MPEncArray ea(context);
    timer.end();
    printf("key gen %f\n", timer.second());

    MPEncMatrix X, Y;


    for (long d : {D}) {
        printf("slots %ld, Dimensiton %ld\n", ea.slots(), D);
        MDL::Matrix<long> vec(d, d);
        MDL::Vector<NTL::ZZ> dec;
        for (long i = 0; i < d; i++)
            for (long j = 0; j < d; j++)
                vec[i][j] = i * 10 + j;
        std::cout << vec.dot(vec) << "\n";
        X.pack(vec, pk, ea);
        Y.pack(vec, pk, ea);
        test_dot2(X, Y, pk, sk, ea, d);
        //test_dot3(X, Y, pk, sk, ea, d);
     }
    return 0;
}

