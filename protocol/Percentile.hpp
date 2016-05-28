#ifndef MDL_PERCENTILE_HPP
#define MDL_PERCENTILE_HPP
#include <vector>
#include "Gt.hpp"
class EncryptedArray;
class FHEPubKey;
namespace MDL {
class EncVector;

struct PercentileParam {
    const long domain;
    const long nr_record; 
    const std::vector<MDL::EncVector> &replicated;
    const MDL::EncVector &K;
};

typedef std::vector<GTResult<void>> PercentileResult;

PercentileResult k_percentile(const PercentileParam &param,
			      const EncryptedArray &ea,
			      const FHEPubKey &pk);
} // namespace MDL
#endif
