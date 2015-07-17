#include "Gt.hpp"
#include "utils/GreaterThanUtils.hpp"
namespace MDL {
GTResult GT(const GTInput       & input,
            const EncryptedArray& ea)
{
    auto parts_nr   = (input.domain + ea.size() - 1) / ea.size();
    auto permutated = permutated_range(input.domain, ea.size());
    auto noises     = random_noise(input.domain, ea.size(), input.plainSpace);

    std::vector<MDL::EncVector> result(parts_nr, input.X);
    NTL::ZZX poly;

    for (size_t i = 0; i < parts_nr; i++) {
        ea.encode(poly, permutated[i]);
        result[i] -= input.Y;
        result[i].addConstant(-poly);
        ea.encode(poly, noises[i]);
        result[i].multByConstant(poly);
    }
    return { result }   ;
}

bool decrypt_gt_result(const GTResult      & result,
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
} // namespace MDL
