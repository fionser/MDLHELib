#include "multiprecision/Multiprecision.h"
#include "algebra/Vector.hpp"
#include "protocol/LR.hpp"
#include <NTL/ZZ.h>
#include <iostream>
int main() {
    MPContext context(4907, 283, 1, 4);
    context.buildModChain(25);
    std::cout << context.primes() << std::endl;
    MPSecKey sk(context);
    MPPubKey pk(sk);
    MPEncArray ea(context);
    MDL::Vector<long> vec(3);
    MDL::Matrix<long> mat(3, 3);
    vec[0] = 10; vec[1] = 10; vec[2] = 10;
    mat[0][0] = 1; mat[0][1] = 2; mat[0][2] = 3;
    mat[1][0] = 2; mat[1][1] = 3; mat[1][2] = 4;
    mat[2][0] = 3; mat[2][1] = 4; mat[2][2] = 5;

    MPEncVector encVec(pk);
    MPEncMatrix encMat, encMat2;
    encVec.pack(vec, ea);
    encMat.pack(mat, pk, ea);
    encMat2.pack(mat, pk, ea);
    {
        // encVec *= encVec;
        // MDL::Vector<NTL::ZZ> vvec;
        // encVec.unpack(vvec, sk, ea);
        // std::cout << vvec << std::endl;
        MDL::MPMatInverseParam param = {pk, ea, 3};
        MDL::Matrix<NTL::ZZ> mmat;
        auto QQ = MDL::inverse(encMat, encVec, param);
        QQ.unpack(mmat, sk, ea);
        std::cout << mmat << std::endl;
    }
    return 0;
    {
        MDL::Matrix<NTL::ZZ> result;

        encMat.mulConstant(mat, ea);
        encMat.unpack(result, sk, ea, true);
        std::cout << result << std::endl;

        encMat2 -= encMat;
        encMat2.negate();
        encMat2.unpack(result, sk, ea, true);
        std::cout << result << std::endl;

        encMat.dot(encMat2, ea, pk, 3);
        encMat.unpack(result, sk, ea, true);
        std::cout << result << std::endl;
    }
    {
        auto prod = encMat.sDot(encVec, pk, ea);
        MDL::Vector<ZZ> result;
        prod.unpack(result, sk, ea, true);
        std::cout << result << std::endl;
    }
    return 0;
}
