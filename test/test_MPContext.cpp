#include "multiprecision/Multiprecision.h"
#include "multiprecision/MPRotate.h"
#include "multiprecision/MPReplicate.h"
#include "algebra/Vector.hpp"
#include "protocol/LR.hpp"
#include "utils/timer.hpp"
#include <NTL/ZZ.h>
#include <iostream>
#include <map>

int main() {
    long m, p, r, P;
    m = 5227;
    p = 67499;
    r = 1;
    P = 1;
    MPContext context(m, p, r, P);
    context.buildModChain(8);
    MPSecKey sk(context);
    MPPubKey pk(sk);
    MPEncArray ea(context);
    printf("going to pack with %ld slots\n", ea.slots());
    MDL::Vector<long> vec(13);
    MDL::Matrix<long> mat(3, 3);
    for (int i = 0; i < vec.dimension(); i++)
        vec[i] = i;
    mat[0][0] = 1; mat[0][1] = 2; mat[0][2] = 3;
    mat[1][0] = 2; mat[1][1] = 3; mat[1][2] = 4;
    mat[2][0] = 3; mat[2][1] = 4; mat[2][2] = 5;

	MPEncVector encVec(pk);
    MPEncMatrix encMat, encMat2;
    encVec.pack(vec, ea);
    encMat.pack(mat, pk, ea);
    encMat2.pack(mat, pk, ea);

    // auto rep = repeat(encVec, ea, pk, vec.size(), vec.size());
    // totalSums(rep, ea, vec.size());
    // {
    //     MDL::Vector<NTL::ZZ> res;
    //     rep.unpack(res, sk, ea);
    //     std::cout << res << "\n";
    // }
    // MDL::Timer timer;
    // timer.start();
    // encMat = encMat.dot(encMat2, ea, pk, 3);
    // timer.end();
    // MDL::Matrix<NTL::ZZ> zzMat(3, 3);
    // encMat.unpack(zzMat, sk, ea, true);
    // std::cout << zzMat << "\n";
    // printf("mult %f\n", timer.second());
    return 0;
}
