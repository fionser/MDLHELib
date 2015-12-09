#include "multiprecision/Multiprecision.h"
#include "algebra/Vector.hpp"
#include "utils/timer.hpp"
#include "fhe/NumbTh.h"

#include <NTL/ZZ.h>
#include <iostream>
#include <map>
void testBatchSwap(const MPPubKey &pk,
                   const MPSecKey &sk,
                   const MPEncArray &ea) {
    MDL::Vector<long> a(ea.slots());
    for (long i = 0; i < ea.slots(); i++) a[i] = i;

    swap_t pairs;
    pairs.push_back(std::make_pair(0, 1));
    pairs.push_back(std::make_pair(2, 3));
    MPEncVector c1(pk), c2(pk);
    c1.pack(a, ea);
    batchSwap(c2, c1, pairs, pk, ea);
    {
        MDL::Vector<NTL::ZZ> a;
        c2.unpack(a, sk, ea);
        std::cout << a << "\n";
    }
}

void testFlatten(const MPPubKey &pk,
                 const MPSecKey &sk,
                 const MPEncArray &ea) {
    MDL::Matrix<long> mat(2, 2);
    mat[0][0] = 1; mat[0][1] = 2;
    mat[1][0] = 3; mat[1][1] = 4;
    MPEncMatrix encMat;
    encMat.pack(mat, pk, ea);
    MPEncVector flatten(pk);
    encMat.flatten(flatten, pk, ea);
    MDL::Vector<NTL::ZZ> vec;
    flatten.unpack(vec, sk, ea);
    std::cout << vec << "\n";
}

void testRepeat(const MPPubKey &pk,
                const MPSecKey &sk,
                const MPEncArray &ea) {
    MDL::Vector<long> v(1);
    v[0] = 3;
    MPEncVector encV(pk), repeated(pk);
    encV.pack(v, ea);
    repeat(repeated, encV, 5, pk, ea);
    MDL::Vector<NTL::ZZ> res;
    repeated.unpack(res, sk, ea);
    std::cout << res << "\n";
}

MPEncMatrix MMM2(long D,
                 const MPPubKey &pk,
                 const MPEncArray &ea) {
    MDL::Matrix<long> mat(D, D);
    for (int i = 0; i < D; i++)
        for (int j = 0; j < D; j++)
            mat[i][j] = i * 10 + j;
    std::cout << mat << "\n";
    MPEncMatrix encM;
    encM.pack(mat, pk, ea);
    printf("columns %ld\n", encM.getColumns());
    MDL::Timer timer;
    auto tmp(encM);
    timer.start();
    tmp.dot2(encM, ea, pk);
    timer.end();
    printf("MMM with dimension %ld cost %f s\n", D, timer.second());
    return tmp;
}

MPEncMatrix MMM(long D,
                const MPPubKey &pk,
                const MPEncArray &ea) {
    MDL::Matrix<long> mat(D, D);
    for (int i = 0; i < D; i++)
        for (int j = 0; j < D; j++)
            mat[i][j] = i * 10 + j;
    std::cout << mat << "\n";
    MPEncMatrix encM;
    encM.pack(mat, pk, ea);
    printf("columns %ld\n", encM.getColumns());
    MDL::Timer timer;
    auto tmp(encM);
    timer.start();
    tmp.dot(encM, ea, pk);
    timer.end();
    printf("MMM with dimension %ld cost %f s\n", D, timer.second());
    return tmp;
}

int main(int argc, char *argv[]) {
    long p, r, L, m, K, D;
    ArgMapping parser;
    parser.arg("p", p, "p");
    parser.arg("r", r, "r");
    parser.arg("L", L, "level");
    parser.arg("m", m, "m");
    parser.arg("K", K, "K: number of ciphetexts for plain text expansion.");
    parser.arg("D", D, "Dimension of matrix");
    parser.parse(argc, argv);

    MPContext context(m, p, r, K);
    context.buildModChain(L);
    MPSecKey sk(context);
    MPPubKey pk = sk;
    MPEncArray ea(context);
    printf("slots %ld\n", ea.slots());

    auto res = MMM2(D, pk, ea);
    MDL::Matrix<NTL::ZZ> mat;
    res.unpack(mat, sk, ea);
    for (int i = 0; i < D; i++) {
        for (int j = 0; j < D; j++)
            std::cout << mat[i][j] << " ";
        std::cout << "\n";
    }

    // testBatchSwap(pk, sk, ea);

    // testFlatten(pk, sk, ea);

    //testRepeat(pk, sk, ea);
    return 0;
}
