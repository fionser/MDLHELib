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

    MDL::EncVector encX(pk);
    MDL::EncVector encY(pk);
    std::vector<MDL::EncVector> result;

    for (long trial = 0; trial < 100; trial++) {
        MDL::Vector<long> x(ea.size(), NTL::RandomBnd(D));
        MDL::Vector<long> y(ea.size(), NTL::RandomBnd(D));
        encX.pack(x, ea);
        encY.pack(y, ea);
        MDL::GTInput input = { encX, encY, D, p };
        auto result = MDL::GT(input, ea);

        if (decrypt_gt_result(result, sk, ea) != (x[0] > y[0])) {
            printf("Error %ld %ld\n", x[0], y[0]);
            break;
        } else {
            printf("----\n");
        }
    }
    return 0;
}
