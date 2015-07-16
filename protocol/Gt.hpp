#ifndef MDL_GT_HPP
#define MDL_GT_HPP
#include "algebra/EncVector.hpp"
#include "fhe/EncryptedArray.h"
#include "fhe/FHE.h"
#include "fhe/Ctxt.h"

#include <vector>
namespace MDL {
struct GTInput {
    const MDL::EncVector& X;
    const MDL::EncVector& Y;
    long domain;
    long plainSpace;
};

struct GTResult {
    std::vector<MDL::EncVector>parts;
};

GTResult GT(const GTInput       & input,
            const EncryptedArray& ea);

bool decrypt_gt_result(const GTResult      & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea);
} // namespace MDL
#endif // MDL_GT_HPP
