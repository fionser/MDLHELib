#include "protocol/Gt.hpp"
#include "paillier/Paillier.hpp"
#include "fhe/FHEContext.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "utils/FHEUtils.hpp"
#include "utils/timer.hpp"

void test_FHE_GT(int argc, char *argv[]) {
    long m, p, r, L, D;
    ArgMapping argmap;

    // MDL::Timer timer;
    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("D", D, "D");
    argmap.parse(argc, argv);

    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey  sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    FHEPubKey pk = sk;
    printf("Slot %ld\n", ea.size());
    std::vector<long> vec(ea.size());
    vec[0] = 2;
    vec[1] = 3;
    Ctxt ctxt(pk);
    ea.encrypt(ctxt, pk, vec);
}

void test_Paillier_GT(int argc, char *argv[]) {
    long key = 1024;
    long domain = 4000;
    int  bits = 32;
    ArgMapping argmap;

    argmap.arg("k", key, "key length");
    argmap.arg("D", domain, "domain");
    argmap.arg("b", bits, "bits for one slot");
    argmap.parse(argc, argv);

    using namespace MDL;
    auto keys = Paillier::GenKey(key);
    Paillier::SecKey sk(keys.first);
    Paillier::PubKey pk(keys.second);
    printf("to gt\n");
    long x = 12;
    long y = 11;
    Paillier::Ctxt eX(pk);
    Paillier::Ctxt eY(pk);
    pk.Pack(eX, x, bits);
    pk.Pack(eY, y, bits);

    GTInput<Paillier::Encryption> input {eX, eY, pk, bits, domain};
    MDL::Timer t;
    t.start();
    auto gt = GT(input);
    t.end();
    printf("gt %f\n", t.second());

    t.reset();
    t.start();
    auto ok = decrypt_gt_result(gt, bits, sk);
    t.end();
    printf("dec %f %d\n", t.second(), ok);
}

int main(int argc, char *argv[]) {
    test_Paillier_GT(argc, argv);
    return 0;
}
