#include "multiprecision/Multiprecision.h"
#include "algebra/Vector.hpp"
#include "utils/timer.hpp"
#include "fhe/NumbTh.h"

#include <NTL/ZZ.h>
#include <iostream>
#include <map>
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

    auto res = MMM(D, pk, ea);
    MDL::Matrix<NTL::ZZ> mat;
    res.unpack(mat, sk, ea);
    for (int i = 0; i < D; i++) {
        for (int j = 0; j < D; j++)
            std::cout << mat[i][j] << " ";
        std::cout << "\n";
    }
    return 0;
}
