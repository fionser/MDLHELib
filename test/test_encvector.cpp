#include "algebra/EncVector.hpp"
#include "algebra/EncMatrix.hpp"
#include "fhe/FHEcontext.h"
#include "fhe/FHE.h"

void testEncVector(FHEPubKey& pk, FHESecKey& sk,
                   EncryptedArray& ea)
{
    MDL::Vector<long> vec(ea.size());

    vec[0] = 1;
    vec[1] = 3;

    MDL::EncVector encVector(pk);
    encVector.pack(vec, ea);

    MDL::Vector<long> result;
    encVector.unpack(result, sk, ea);
    assert(result[0] == vec[0]);
    assert(result[1] == vec[1]);

    MDL::EncVector oth(encVector);
    auto dot = encVector.dot(oth, ea);
    dot.unpack(result, sk, ea);
    assert(result[0] == 10);
    assert(result[1] == 10);
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
    {
        MDL::Vector<long> result;
        auto dot = encMatrix.dot(encVec, ea);
        dot.unpack(result, sk, ea);
        assert(result[0] == 5);
        assert(result[1] == 13);
    }
    {
        encMatrix.transpose(ea);
        auto dot = encMatrix.dot(encVec, ea);
        MDL::Vector<long> result;
        dot.unpack(result, sk, ea);
        assert(result[0] == 6);
        assert(result[1] == 10);
    }
}

int main() {
    FHEcontext context(1031, 101, 1);

    buildModChain(context, 3);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    testEncVector(pk, sk, ea);
    testEncMatrix(pk, sk, ea);
    std::cout << "All Tests Passed" << std::endl;
    return 0;
}
