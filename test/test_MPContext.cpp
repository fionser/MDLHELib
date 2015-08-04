#include "multiprecision/MPContext.hpp"
#include "multiprecision/MPSecKey.hpp"
#include "multiprecision/MPPubKey.hpp"
#include "multiprecision/MPEncArray.hpp"
#include "multiprecision/MPEncVector.hpp"
#include "multiprecision/MPEncMatrix.hpp"
#include "algebra/Vector.hpp"
#include <NTL/ZZ.h>
int main() {
    MPContext context(4907, 283, 1, 2);
    context.buildModChain(5);
    std::cout << context.primes() << std::endl;
    MPSecKey sk(context);
    MPPubKey pk(sk);
    MPEncArray ea(context);
    MDL::Vector<long> vec(3);
    MDL::Matrix<long> mat(3, 3);
    vec[0] = 10; vec[1] = 20; vec[2] = 40;
    mat[0][0] = 1; mat[0][1] = 2; mat[0][2] = 3;
    mat[1][0] = 2; mat[1][1] = 3; mat[1][2] = 4;
    mat[2][0] = 3; mat[2][1] = 4; mat[2][2] = 5;

    MPEncVector encVec(pk);
    MPEncMatrix encMat(pk);
    encVec.pack(vec, ea);
    encMat.pack(mat, ea);
    {
        auto prod = encMat.sDot(encVec, ea);
        MDL::Vector<ZZ> result;
        prod.unpack(result, sk, ea);
        std::cout << result << std::endl;
    }
    return 0;
}
