#include <fhe/FHEContext.h>
#include <fhe/FHE.h>
#include <fhe/NumbTh.h>
#include <fhe/EncryptedArray.h>
#include <utils/timer.hpp>

#include <algebra/NDSS.h>

int main(int argc, char *argv[]) {
    long m, p, r, L;
    ArgMapping argmap;
    MDL::Timer timer;

    // MDL::Timer timer;
    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.parse(argc, argv);
    timer.start();
    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;
    timer.end();
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    printf("M = %ld, p = %ld, r = %ld, L = %ld, slot = %ld\n", m, p,
           r, L, ea.size());
    printf("KeyGen %fs\n",                                     timer.second());

    MDL::Vector<long> plain(ea.size());
    MDL::EncVector    ctxt(pk);
    {
        timer.reset();
        timer.start();

        for (int i = 0; i < 1000; i++) {
            ctxt.pack(plain, ea);
        }
        timer.end();
        printf("Encryption %f ms", timer.second());
    }
    {
        timer.reset(); timer.start();

        for (int i = 0; i < 1000; i++) {
            ctxt.unpack(plain, sk, ea);
        }
        timer.end();
        printf("Decryption %f ms", timer.second());
    }
    {
        timer.reset(); timer.start();

        for (int i = 0; i < 1000; i++) {
            auto tmp(ctxt);
            tmp += tmp;
        }
        timer.end();
        printf("Addition %f ms", timer.second());
    }
    {
        timer.reset(); timer.start();

        for (int i = 0; i < 1000; i++) {
            auto tmp(ctxt);
            tmp.multiplyBy(tmp);
        }
        timer.end();
        printf("Addition %f ms", timer.second());
    }
    {
        MDL::Vector<long> constant(ea.size(), 1);
        NTL::ZZX poly;
        ea.encode(poly, constant);
        timer.reset(); timer.start();
        for (int i = 0; i < 1000; i++) {
            auto tmp(ctxt);
            tmp.multByConstant(poly);
        }
        timer.end();
        printf("Mult with constant %f ms", timer.second());

        timer.reset(); timer.start();
        for (int i = 0; i < 1000; i++) {
            auto tmp(ctxt);
            tmp.addConstant(poly);
        }
        timer.end();
        printf("Add with constant %f ms", timer.second());
    }

    return 0;
}
