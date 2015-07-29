#include "protocol/Gt.hpp"
#include "fhe/FHEContext.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "utils/FHEUtils.hpp"
int main(int argc, char *argv[]) {
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
    return 0;
}
