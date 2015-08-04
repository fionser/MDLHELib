#include "algebra/CRT.hpp"
#include "fhe/FHEcontext.h"
#include "fhe/FHE.h"
#include "fhe/EncryptedArray.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <memory>
struct FHE {
    std::shared_ptr<FHEcontext> context;
    std::shared_ptr<FHESecKey> sk;
    std::shared_ptr<FHEPubKey> pk;
    std::shared_ptr<EncryptedArray> ea;
};

FHE setFHE(long m, long p, long r, long L)
{
    FHE fhe;
    fhe.context = std::make_shared<FHEcontext>(m, p, r);
    buildModChain(*fhe.context, L);
    fhe.sk = std::make_shared<FHESecKey>(*fhe.context);
    fhe.sk->GenSecKey(64);
    addSome1DMatrices(*fhe.sk);
    fhe.pk = std::make_shared<FHEPubKey>(*fhe.sk);
    auto G = fhe.context->alMod.getFactorsOverZZ()[0];
    fhe.ea = std::make_shared<EncryptedArray>(*fhe.context, G);
    return fhe;
}

int main() {
    std::vector<long> a{2, 30};

    auto fhe1 = setFHE(1031, 2, 2, 3);
    auto fhe2 = setFHE(1031, 5, 2, 3);
    Ctxt c1(*fhe1.pk), c2(*fhe2.pk);
    fhe1.ea->encrypt(c1, *fhe1.pk, a);
    fhe2.ea->encrypt(c2, *fhe2.pk, a);

    c1 += c1;
    c2 += c2;
    std::vector<long> v1, v2;
    fhe1.ea->decrypt(c1, *fhe1.sk, v1);
    fhe2.ea->decrypt(c2, *fhe2.sk, v2);
    std::cout << v1 << std::endl;
    std::cout << v2 << std::endl;
    std::cout << MDL::CRT({v1[0], v2[0]}, {4, 25}) << std::endl;
    std::cout << MDL::CRT({v1[1], v2[1]}, {4, 25}) << std::endl;
    return 0;
}
