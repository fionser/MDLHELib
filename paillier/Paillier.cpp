#include "Paillier.hpp"
#include "algebra/CRT.hpp"
#include <cassert>
namespace MDL {
namespace Paillier {
class Ctxt::CtxtImp {
public:
    CtxtImp(const PubKey &pk) : pk(pk) {}
    CtxtImp(const CtxtImp &oth) : pk(oth.pk) {
        this->value = oth.value;
    }
    CtxtImp& operator=(const CtxtImp &oth) = delete;

    void SetCtxt(const NTL::ZZ &v) {
        this->value = v;
    }

    void SetCtxt(const long v) {
        this->value = NTL::to_ZZ(v);
    }

    const NTL::ZZ& GetValue() const {
        return value;
    }

    const PubKey& GetPk() const {
        return pk;
    }

    CtxtImp& operator+=(const CtxtImp &oth) {
        assert(pk == oth.pk);
        NTL::MulMod(value, value, oth.value, pk.GetN2());
        return *this;
    }

    CtxtImp& operator+=(long v) {
        return operator+=(NTL::to_ZZ(v));
    }

    CtxtImp& operator+=(const NTL::ZZ &v) {
        Ctxt c(pk);
        pk.Encrypt(c, v);
        return (*this += *c.imp);
    }

    CtxtImp& operator-=(const CtxtImp &oth) {
        auto tmp(oth);
        tmp *= -1;
        return (*this += tmp);
    }

    CtxtImp& operator-=(long v) {
        return operator-=(NTL::to_ZZ(v));
    }

    CtxtImp& operator-=(const NTL::ZZ &v) {
        Ctxt c(pk);
        pk.Encrypt(c, -v);
        return (*this += *c.imp);
    }

    CtxtImp& operator*=(long v) {
        NTL::PowerMod(value, value, v, pk.GetN2());
        return *this;
    }
    CtxtImp& operator*=(const NTL::ZZ &v) {
        NTL::PowerMod(value, value, v, pk.GetN2());
        return *this;
    }
private:
    NTL::ZZ value;
    const PubKey &pk;
};

Ctxt::Ctxt(const PubKey &pk) {
    imp = std::make_shared<CtxtImp>(pk);
}

Ctxt::Ctxt(const Ctxt &oth) {
    imp = std::make_shared<CtxtImp>(*oth.imp);
}

const NTL::ZZ& Ctxt::GetValue() const {
    return imp->GetValue();
}

void Ctxt::SetCtxt(const NTL::ZZ &number) {
    imp->SetCtxt(number);
}

void Ctxt::SetCtxt(const long number) {
    imp->SetCtxt(number);
}

Ctxt& Ctxt::operator+=(const Ctxt &oth) {
    imp->operator+=(*oth.imp);
    return *this;
}

Ctxt& Ctxt::operator+=(const long v) {
    imp->operator+=(v);
    return *this;
}

Ctxt& Ctxt::operator+=(const NTL::ZZ &v) {
    imp->operator+=(v);
    return *this;
}

Ctxt& Ctxt::operator-=(const Ctxt &oth) {
    imp->operator-=(*oth.imp);
    return *this;
}

Ctxt& Ctxt::operator-=(const long v) {
    imp->operator-=(v);
    return *this;
}

Ctxt& Ctxt::operator-=(const NTL::ZZ &v) {
    imp->operator-=(v);
    return *this;
}
Ctxt& Ctxt::operator*=(const long v) {
    imp->operator*=(v);
    return *this;
}

Ctxt& Ctxt::operator*=(const NTL::ZZ &v) {
    imp->operator*=(v);
    return *this;
}

const PubKey& Ctxt::GetPk() const {
    return imp->GetPk();
}
class PubKey::PubKeyImp {
public:
    PubKeyImp(const NTL::ZZ &n, const long slot_nr) : n(n), g(n + 1) {
        n2 = n * n;
        assert(slot_nr > 0);
        assert(n > 0);
        if (slot_nr == 1) return;

        primes.reserve(slot_nr);
        long bits_each = std::lround(1.0 * NTL::NumBits(n) / slot_nr);
        assert(bits_each >= 2);
        for (long i = 0; i < slot_nr; i++) {
            long trial = 0;
            while (trial++ < 30) {
                long prime = NTL::RandomPrime_long(bits_each);
                if (std::find(primes.begin(), primes.end(),
                              prime) == primes.end()) {
                    primes.push_back(prime);
                    break;
                }
            }
        }
        // if there was not enough primes to find
        assert(primes.size() == slot_nr);
    }

    PubKeyImp(const PubKeyImp &oth) : n(oth.n), g(oth.g), n2(oth.n2), primes(oth.primes) { }

    bool operator==(const PubKey &oth) const {
        return *this == *oth.imp;
    }

    bool operator==(const PubKey::PubKeyImp &oth) const {
        if (oth.primes.size() != primes.size()) return false;
        if (oth.n != n) return false;
        for (size_t i = 0; i < primes.size(); i++) {
            if (oth.primes[i] != primes[i]) return false;
        }

        return true;
    }

    void Encrypt(Ctxt &ctxt, const long plain) const {
        Encrypt(ctxt, NTL::to_ZZ(plain));
    }

    void Encrypt(Ctxt &ctxt, const NTL::ZZ &plain) const {
        assert(*this == ctxt.GetPk());
        auto bits = NTL::NumBits(n);
        NTL::ZZ r, res;
        NTL::RandomBits(r, bits);
        NTL::PowerMod(r, r, n, n2);
        NTL::PowerMod(res, g, plain, n2);
        NTL::MulMod(res, r, n2);
        ctxt.SetCtxt(res);
    }

    void Pack(Ctxt &ctxt, const std::vector<long> &slots) const {
        assert(!primes.empty());
        auto crt = MDL::CRT(slots, primes);
        Encrypt(ctxt, crt);
    }

    const NTL::ZZ& GetN() const {
        return n;
    }

    const NTL::ZZ& GetG() const {
        return n;
    }

    const NTL::ZZ& GetN2() const {
        return n2;
    }

    const std::vector<long>& GetPrimes() const {
        return primes;
    }
private:
    NTL::ZZ n, g, n2;
    std::vector<long> primes;
};

PubKey::PubKey(const NTL::ZZ &n, long slot_nr) {
    imp = std::make_shared<PubKeyImp>(n, slot_nr);
}

PubKey::PubKey(const PubKey &oth) {
    imp = std::make_shared<PubKeyImp>(*oth.imp);
}

bool PubKey::operator==(const PubKey &oth) const {
    return *imp == *oth.imp;
}

void PubKey::Encrypt(Ctxt &ctxt, const NTL::ZZ &plain) const {
    imp->Encrypt(ctxt, plain);
}

void PubKey::Encrypt(Ctxt &ctxt, const long plain) const {
    imp->Encrypt(ctxt, plain);
}

void PubKey::Pack(Ctxt &ctxt, const std::vector<long> &slots) const {
    imp->Pack(ctxt, slots);
}

const NTL::ZZ& PubKey::GetN() const {
    return imp->GetN();
}

const NTL::ZZ& PubKey::GetG() const {
    return imp->GetG();
}

const NTL::ZZ& PubKey::GetN2() const {
    return imp->GetN2();
}

const std::vector<long>& PubKey::GetPrimes() const {
    return imp->GetPrimes();
}

class SecKey::SecKeyImp {
public:
    SecKeyImp(const NTL::ZZ p, const NTL::ZZ &q, const PubKey &pk) : pk(pk) {
        assert(p * q == pk.GetN());
        auto p_1 = p - 1;
        auto q_1 = q - 1;
        // l = lcm(p-1, q-1)
        l = (p_1 * q_1) / NTL::GCD(p_1, q_1);
        // il = l^-1 % n
        NTL::InvMod(il, l, pk.GetN());
    }

    SecKeyImp(const SecKeyImp &oth) : l(oth.l), il(oth.il), pk(oth.pk) { }

    bool operator==(const SecKeyImp &oth) {
        return l == oth.l && pk == oth.pk;
    }

    void Encrypt(Ctxt &ctxt, const NTL::ZZ &plain) const {
        pk.Encrypt(ctxt, plain);
    }

    void Encrypt(Ctxt &ctxt, const long plain) const {
        pk.Encrypt(ctxt, plain);
    }

    void Pack(Ctxt &ctxt, const std::vector<long> &slots) const {
        pk.Pack(ctxt, slots);
    }

    void Decrypt(NTL::ZZ &plain, const Ctxt &ctxt) const {
        assert(pk == ctxt.GetPk());
        NTL::PowerMod(plain, ctxt.GetValue(), l, pk.GetN2());
        plain -= 1;
        plain /= pk.GetN();
        NTL::MulMod(plain, plain, il, pk.GetN());
    }

    void Unpack(std::vector<long> &slots, const Ctxt &ctxt) const {
        NTL::ZZ plain;
        Decrypt(plain, ctxt);
        const auto &primes = pk.GetPrimes();
        assert(!primes.empty());
        slots.resize(0);
        slots.reserve(primes.size());
        for (auto prime : primes) {
            slots.push_back(plain % prime);
        }
    }

private:
    NTL::ZZ l, il;
    const PubKey pk;
};

SecKey::SecKey(const NTL::ZZ &p, const NTL::ZZ &q, const PubKey &pk) {
    imp = std::make_shared<SecKeyImp>(p, q, pk);
}

SecKey::SecKey(const SecKey &oth) {
    imp = std::make_shared<SecKeyImp>(*oth.imp);
}

bool SecKey::operator==(const SecKey &oth) const {
    return imp == oth.imp;
}

void SecKey::Encrypt(Ctxt &ctxt, const NTL::ZZ &plain) const {
    imp->Encrypt(ctxt, plain);
}

void SecKey::Encrypt(Ctxt &ctxt, const long plain) const {
    imp->Encrypt(ctxt, plain);
}

void SecKey::Decrypt(NTL::ZZ &r, const Ctxt &ctxt) const {
    imp->Decrypt(r, ctxt);
}

void SecKey::Unpack(std::vector<long> &slots, const Ctxt &ctxt) const {
    imp->Unpack(slots, ctxt);
}

std::pair<SecKey, PubKey> GenKey(long bits, long slot_nr) {
    assert(bits > 2);
    auto half = (bits + 1) >> 1;
    NTL::ZZ p, q;
    NTL::RandomPrime(p, half);
    do {
        NTL::RandomPrime(q, half);
    } while (q == p);

    PubKey pk(p * q, slot_nr);
    SecKey sk(p, q, pk);

    return std::make_pair(sk, pk);
}

}// namespace Paillier
}// namespace MDL
