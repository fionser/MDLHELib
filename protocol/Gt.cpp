#include "Gt.hpp"
#include "utils/GreaterThanUtils.hpp"
namespace MDL {
template <>
GTResult<void> GT(const GTInput<void> & input,
                  const EncryptedArray& ea)
{
    auto parts_nr   = (input.domain + ea.size() - 1) / ea.size();
    auto permutated = permutated_range(input.domain, ea.size());
    auto noises     = random_noise(input.domain, ea.size(), input.plainSpace);

    MDL::EncVector tmp(input.X);
    tmp -= input.Y;
    std::vector<MDL::EncVector> result(parts_nr, tmp);
    NTL::ZZX poly;

    for (size_t i = 0; i < parts_nr; i++) {
        ea.encode(poly, permutated[i]);
        result[i].addConstant(-poly);
        ea.encode(poly, noises[i]);
        result[i].multByConstant(poly);
    }

    return { result };
}

template <>
GTResult<Paillier::Encryption> GT(const GTInput<Paillier::Encryption> &input) {
    auto nr_prime = input.pk.GetPrimes().size();
    auto bit_per_prime = input.pk.bits_per_prime();
    auto slot_one_cipher = nr_prime / ((input.bits + bit_per_prime - 1) / bit_per_prime);
    auto nr_cipher = (input.domain + slot_one_cipher - 1) / slot_one_cipher;
    auto permutated = permutated_range(input.domain, slot_one_cipher);
    Paillier::Ctxt c(input.X);
    c -= input.Y;
    GTResult<Paillier::Encryption> result = { std::vector<Paillier::Ctxt>(nr_cipher, c) };

    auto bits = NTL::NumBits(input.pk.GetN()) - input.pk.bits_all_prime();
    for (size_t i = 0; i < nr_cipher; i++) {
        input.pk.Pack(result.parts[i], permutated[i], input.bits);
        result.parts[i] -= c;
        auto r = NTL::RandomBits_ZZ(bits);
        result.parts[i] *= r;
    }

    return result;
}

template <>
bool decrypt_gt_result(const GTResult<void>& result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea)
{
    MDL::Vector<long> dec;

    for (const auto& ctxt : result.parts) {
        ctxt.unpack(dec, sk, ea);
        for (auto ele : dec) {
            if (ele == 0) return true;
        }
    }
    return false;
}

bool decrypt_gt_result(const GTResult<Paillier::Encryption> &result,
                       int bit,
                       const Paillier::SecKey &sk) {
    MDL::Vector<NTL::ZZ> dec;
    for (auto &c : result.parts) {
        sk.Unpack(dec, c, bit);
        for (auto & e : dec) {
            if (e == 0) return true;
        }
    }
    return false;
}
} // namespace MDL

