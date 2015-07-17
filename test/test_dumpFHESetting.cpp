#include "utils/FHEUtils.hpp"
#include "utils/timer.hpp"
#include "fhe/EncryptedArray.h"
#include <string>
void test_load(long m, long p, long r, long L)
{
    MDL::Timer  timer;
    std::string path("fhe_setting");

    timer.start();
    FHEcontext context(m, p, r);
    timer.end();
    printf("Initial Context costed %f\n", timer.second());

    timer.reset();
    FHESecKey sk(context);
    timer.end();
    printf("Initial SK costed %f\n", timer.second());

    timer.reset();
    load_FHE_setting(path, context, sk);
    timer.end();
    printf("Load costed %f sec.\n", timer.second());
    FHEPubKey pk = sk;
    NTL::ZZX  plain;
    Ctxt ctxt(pk);

    pk.Encrypt(ctxt, NTL::to_ZZX(10));
    ctxt *= ctxt;
    sk.Decrypt(plain, ctxt);
    assert(plain == 100);

    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    assert(ea.size() == 4);
}

void test_dump(long m, long p, long r, long L)
{
    MDL::Timer timer;

    timer.start();
    dump_FHE_setting_to_file("fhe_setting_32", 80, m, p, r, L);
    timer.end();
    printf("Cost %f to dump m : %ld, p : %ld, r : %ld, L : %ld\n",
           timer.second(), m, p, r, L);
}

int main(int argc, char *argv[]) {
    ArgMapping arg;
    long m, p, r, L;

    arg.arg("m", m, "m");
    arg.arg("p", p, "p");
    arg.arg("r", r, "r");
    arg.arg("L", L, "L");
    arg.parse(argc, argv);
    //test_load(m, p, r, L);

    test_dump(m, p, r, L);
    return 0;
}
