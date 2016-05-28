#include "Percentile.hpp"
#include "algebra/EncVector.hpp"
#include "fhe/EncryptedArray.h"
#include "fhe/FHE.h"
namespace MDL {
PercentileResult k_percentile(const PercentileParam &param,
			      const EncryptedArray &ea,
			      const FHEPubKey &pk) {
	long plainSpace  = ea.getContext().alMod.getPPowR();
        PercentileResult ret;		
	ret.reserve(param.domain);
        for (int d = 0; d < param.domain; d++) {
		GTInput<void> gt = { param.K, param.replicated.at(d), param.nr_record, plainSpace };
	        ret.push_back(GT(gt, ea));
        }
	return ret;
}

} // namespace MDL
