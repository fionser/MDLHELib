#include "Paillier.hpp"
#include "algebra/CRT.hpp"
#include <cassert>
#include "algorithm"
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
    PubKeyImp(const NTL::ZZ &n) : n(n), g(n + 1) {
        n2 = n * n;
        assert(n > 0);
        long target_bits_len = NTL::NumBits(n) >> 1;
        long bits_each = 10;
        assert(bits_each >= 2);
		while (target_bits_len >= bits_each) {
            long trial = 0;
            while (trial++ < 30) {
                auto prime = NTL::RandomPrime_ZZ(bits_each);
                if (std::find(primes.begin(), primes.end(), prime) == primes.end()) {
                    primes.push_back(prime);
                    target_bits_len -= NTL::NumBits(prime);
                    break;
                }
            }
        }
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

    void Pack(Ctxt &ctxt, const std::vector<long> &slots, int bits) const {
        assert(!primes.empty());
		std::vector<NTL::ZZ> tmp_primes;
		long bits_need = bits;
		NTL::ZZ tmp(1);
		for (auto &p : primes) {
			if (bits_need > 0) {
				tmp *= p;
				bits_need -= NTL::NumBits(p);
			} else {
				tmp_primes.push_back(tmp);
				tmp = NTL::to_ZZ(1);
				bits_need = bits;
			}
		}

        auto crt = MDL::CRT(slots, tmp_primes);
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

    const PrimeSet& GetPrimes() const {
        return primes;
    }
private:
    NTL::ZZ n, g, n2;
    PrimeSet primes;
};

PubKey::PubKey(const NTL::ZZ &n) {
    imp = std::make_shared<PubKeyImp>(n);
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

void PubKey::Pack(Ctxt &ctxt, const std::vector<long> &slots, int bits) const {
    imp->Pack(ctxt, slots, bits);
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

const PrimeSet& PubKey::GetPrimes() const {
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

    void Pack(Ctxt &ctxt, const std::vector<long> &slots, int bits) const {
        pk.Pack(ctxt, slots, bits);
    }

    void Decrypt(NTL::ZZ &plain, const Ctxt &ctxt) const {
        assert(pk == ctxt.GetPk());
        NTL::PowerMod(plain, ctxt.GetValue(), l, pk.GetN2());
        plain -= 1;
        plain /= pk.GetN();
        NTL::MulMod(plain, plain, il, pk.GetN());
    }

    void Unpack(std::vector<NTL::ZZ> &slots, const Ctxt &ctxt, int bits) const {
        NTL::ZZ plain;
        Decrypt(plain, ctxt);
        const auto &primes = pk.GetPrimes();
        assert(!primes.empty());
        slots.resize(0);
		std::vector<NTL::ZZ> tmp_primes;
		long bits_need = bits;
		NTL::ZZ tmp(1);
		for (auto &p : primes) {
			if (bits_need > 0) {
				tmp *= p;
				bits_need -= NTL::NumBits(p);
			} else {
				tmp_primes.push_back(tmp);
				tmp = NTL::to_ZZ(1);
				bits_need = bits;
			}
		}

        slots.reserve(tmp_primes.size());
        for (auto prime : tmp_primes) {
            slots.push_back(plain % prime);
        }
    }

    const PubKey &GetPk() const { return pk; }
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

void SecKey::Unpack(std::vector<NTL::ZZ> &slots, const Ctxt &ctxt, int bits) const {
    imp->Unpack(slots, ctxt, bits);
}

const PubKey& SecKey::GetPk() const {
    return imp->GetPk();
}

std::pair<SecKey, PubKey> GenKey(long bits) {
    assert(bits > 2);
    auto half = (bits + 1) >> 1;
    NTL::ZZ p, q;
    NTL::RandomPrime(p, half);
    do {
        NTL::RandomPrime(q, half);
    } while (q == p);

    PubKey pk(p * q);
    SecKey sk(p, q, pk);

    return std::make_pair(sk, pk);
}

}// namespace Paillier
}// namespace MDL
