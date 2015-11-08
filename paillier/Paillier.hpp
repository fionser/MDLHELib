#ifndef MDL_PAILLER_HPP
#define MDL_PAILLER_HPP
#include <NTL/ZZ.h>
#include <memory>
#include <vector>
namespace MDL {
namespace Paillier {
//forward declaration
class Ctxt;
typedef std::vector<NTL::ZZ> PrimeSet;

class PubKey {
public:
    explicit PubKey(const NTL::ZZ &n);
    PubKey(const PubKey &oth);
    PubKey& operator=(const PubKey &oth) = delete;
    bool operator==(const PubKey &oth) const;
    ~PubKey() {}
    void Encrypt(Ctxt &ctxt, const NTL::ZZ &plain) const;
    void Encrypt(Ctxt &ctxt, const long plain) const;
    void Pack(Ctxt &ctxt, const std::vector<long> &slots, int bits) const;
    const NTL::ZZ& GetN() const;
    const NTL::ZZ& GetG() const;
    const NTL::ZZ& GetN2() const;
    const PrimeSet& GetPrimes() const;
private:
    class PubKeyImp;
    std::shared_ptr<PubKeyImp> imp = nullptr;
};

class SecKey {
public:
    explicit SecKey(const NTL::ZZ &p, const NTL::ZZ &q, const PubKey &pk);
    SecKey(const SecKey &oth);
    SecKey& operator=(const SecKey &oth) = delete;
    bool operator==(const SecKey &oth) const;
    ~SecKey() {}
    void Encrypt(Ctxt &ctxt, const NTL::ZZ &plain) const;
    void Encrypt(Ctxt &ctxt, const long plain) const;
    void Decrypt(NTL::ZZ &plain, const Ctxt &ctxt) const;
    void Decrypt(long &plain, const Ctxt &ctxt) const;
    void Unpack(std::vector<NTL::ZZ> &slots, const Ctxt &ctxt, int bits) const;
    const PubKey &GetPk() const;
private:
    class SecKeyImp; // implementation
    std::shared_ptr<SecKeyImp> imp = nullptr;
};

class Ctxt {
public:
    explicit Ctxt(const PubKey &pk);
    Ctxt(const Ctxt &oth);
    Ctxt& operator=(const Ctxt &oth) = delete;
    Ctxt& operator+=(const Ctxt &oth);
    Ctxt& operator+=(long v);
    Ctxt& operator+=(const NTL::ZZ &v);
    Ctxt& operator-=(const Ctxt &oth);
    Ctxt& operator-=(long v);
    Ctxt& operator-=(const NTL::ZZ &v);
    Ctxt& operator*=(long v);
    Ctxt& operator*=(const NTL::ZZ &v);
    ~Ctxt() {}
    void SetCtxt(const NTL::ZZ &number);
    void SetCtxt(const long number);
    const PubKey& GetPk() const;
    const NTL::ZZ& GetValue() const;
private:
    class CtxtImp;
    std::shared_ptr<CtxtImp> imp = nullptr;
};

std::pair<SecKey, PubKey> GenKey(long bits);
}// namespace Paillier
}// namespace MDL
#endif // MDL_PAILLER_HPP
