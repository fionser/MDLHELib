#include "algebra/EncVector.hpp"
#include "algebra/EncMatrix.hpp"
#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "utils/FHEUtils.hpp"
#include "utils/timer.hpp"
void testEncVector(FHEPubKey& pk, FHESecKey& sk,
                   EncryptedArray& ea)
{
    MDL::Vector<long> vec(ea.size());

    vec[0] = 1;
    vec[1] = 2;
    vec[2] = 3;
    MDL::EncVector encVector(pk);
    encVector.pack(vec, ea);
    MDL::Vector<long> result;
    MDL::Matrix<long> Cox;
    encVector.unpack(result, sk, ea);
    assert(result[0] == vec[0]);
    assert(result[1] == vec[1]);
    //
    // MDL::EncVector oth(encVector);
    // auto dot = encVector.dot(oth, ea);
    // dot.unpack(result, sk, ea);
    // assert(result[0] == 14);
    // assert(result[1] == 14);
    auto cox = encVector.covariance(ea, 3);
    cox.unpack(Cox, sk, ea);
    std::cout << Cox << std::endl;
}


void testNegateUnpack(FHEPubKey& pk, FHESecKey& sk,
                      EncryptedArray& ea)
{
    MDL::Matrix<long> mat(ea.size(), ea.size());
    MDL::EncMatrix encMat(pk);
    mat[0][0] = 10;
    mat[1][1] = -10;
    encMat.pack(mat, ea);
    {
        MDL::Matrix<NTL::ZZX> result;
        encMat.unpack(result, sk, ea, true);
        assert(result[0][0] == 10);
        assert(result[1][1] == -10);
    }
    {
        MDL::Matrix<long> result;
        encMat.unpack(result, sk, ea, true);
        assert(result[0][0] == 10);
        assert(result[1][1] == -10);
    }
}

void testEncMatrix(FHEPubKey& pk, FHESecKey& sk,
                   EncryptedArray& ea)
{
    MDL::Matrix<long> mat(ea.size(), ea.size());
    MDL::Vector<long> vec(ea.size());
    // [[1 2]     [3
    //  [3 4]]  *  1]
    mat[0][0] = 1;
    mat[0][1] = 2;
    mat[1][0] = 3;
    mat[1][1] = 4;
    vec[0]    = 3;
    vec[1]    = 1;

    MDL::EncMatrix encMatrix(pk);
    encMatrix.pack(mat, ea);
    MDL::EncVector encVec(pk);
    encVec.pack(vec, ea);
    {
        MDL::Matrix<long> result;
        encMatrix.unpack(result, sk, ea);
        assert(result[0][0] == mat[0][0]);
        assert(result[1][1] == mat[1][1]);
    }
    while (0) {
        MDL::Timer time;
        MDL::Vector<long> result;
        time.start();
        auto dot = encMatrix.column_dot(encVec, ea);
        time.end();
        std::cout << "Mat dot Vector: " << time.second() << std::endl;
        dot.unpack(result, sk, ea);
        assert(result[0] == 6);
        assert(result[1] == 10);
        break;
    }
    {
        MDL::Timer time;
        time.start();
        encMatrix.transpose(ea);
        time.end();
        std::cout << "Mat transpose: " << time.second() << std::endl;
        time.reset();time.start();
        auto dot = encMatrix.column_dot(encVec, ea);
        time.end();
        std::cout << "Mat Vec Dot: " << time.second() << std::endl;
        MDL::Vector<long> result;
        dot.unpack(result, sk, ea);
        assert(result[0] == 5);
        assert(result[1] == 13);
    }
}

void testMatrixDotMatrix(const FHEPubKey &pk,
                         const FHESecKey &sk,
                         const EncryptedArray &ea)
{
    MDL::Timer timer;
    const long dimension = 8;
    MDL::Matrix<long> m1(dimension, dimension);
    MDL::Matrix<long> m2(dimension, dimension);
    MDL::EncMatrix encM1(pk);
    MDL::EncMatrix encM2(pk);
    m1[0][0] = 1; m1[0][1] = 2;
    m1[1][0] = 3; m1[1][1] = 4;

    m2[0][0] = 5; m2[0][1] = 6;
    m2[1][0] = 7; m2[1][1] = 8;
    printf("going to encrypt\n");
    encM1.pack(m1, ea);
    encM2.pack(m2, ea);
    timer.start();
    auto prod = encM1.dot(encM2, ea, dimension);
    timer.end();
    printf("Product two matrices with %ld dimensions costed %f sec.\n",
           dimension, timer.second());
    MDL::Matrix<long> result;
    prod.unpack(result, sk, ea);
    assert(result[0][0] == 19);
    assert(result[0][1] == 22);
    assert(result[1][0] == 43);
    assert(result[1][1] == 50);
}

int main() {
    FHEcontext context(4097, 283, 1);

    buildModChain(context, 5);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    printf("slot %ld\n", ea.size());
    testEncVector(pk, sk, ea);
    testEncMatrix(pk, sk, ea);
    testMatrixDotMatrix(pk, sk, ea);
    testNegateUnpack(pk, sk, ea);
    std::cout << "All Tests Passed" << std::endl;
    return 0;
}
