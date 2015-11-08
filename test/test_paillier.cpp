#include "paillier/Paillier.hpp"
#include "algebra/CRT.hpp"
#include <iostream>
#include <vector>
class LCtxt {
public:
    LCtxt(const MDL::Paillier::PubKey &pk) : a(pk) { }
    MDL::Paillier::Ctxt a;
    NTL::ZZ p;
    std::vector<MDL::Paillier::Ctxt> b;
    bool mult = false;
};

template<typename T>
LCtxt Encrypt(T &m, MDL::Paillier::PubKey &pk) {
    auto b = NTL::RandomBnd(pk.GetN());
    LCtxt lc(pk);
    lc.p = m - b;
    MDL::Paillier::Ctxt c(pk);
    pk.Encrypt(c, b);
    lc.b.push_back(c);
    return lc;
}

LCtxt Mult(LCtxt &c1, LCtxt &c2, MDL::Paillier::PubKey &pk) {
    auto p = c1.p * c2.p;
    LCtxt ret(pk);
    pk.Encrypt(ret.a, p);

    MDL::Paillier::Ctxt b1(c1.b.front());
    MDL::Paillier::Ctxt b2(c2.b.front());
    b1 += 0; // re-rand
    b2 += 0; // re-rand
    ret.b.push_back(b1);
    ret.b.push_back(b2);

    b1 *= c2.p;
    b2 *= c1.p;
    ret.a += b1;
    ret.a += b2;

    ret.mult = true;
    return ret;
}

NTL::ZZ Decrypt(LCtxt &c, MDL::Paillier::SecKey &sk) {
    NTL::ZZ pl, bb;
    if (c.mult) {
        sk.Decrypt(pl, c.a);
    } else {
        pl = c.p;
    }

    bb = 1;
    for (auto &b : c.b) {
        NTL::ZZ tmp;
        sk.Decrypt(tmp, b);
        bb *= tmp;
    }

    return (pl + bb) % sk.GetPk().GetN();
}

int main() {
    auto keys = MDL::Paillier::GenKey(1024, 32);
    MDL::Paillier::SecKey sk(keys.first);
    MDL::Paillier::PubKey pk(keys.second);
    std::vector<long> slots = {1, 2, 3};
    auto &primes = pk.GetPrimes();
    auto packed = MDL::CRT(slots, primes);
    auto multed = packed * packed;

    auto pp = packed * packed % pk.GetN();
    auto lctxt = Encrypt(packed, pk);
    auto mult = Mult(lctxt, lctxt, pk);
    auto res = Decrypt(mult, sk);

    for (auto &p : primes) {
        std::cout << res % p << " " ;
    }
    std::cout << "\n";
    return 0;
}
