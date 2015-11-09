#ifndef MDL_GT_HPP
#define MDL_GT_HPP
#include "algebra/EncVector.hpp"
#include "fhe/EncryptedArray.h"
#include "fhe/FHE.h"
#include "fhe/Ctxt.h"
#include "paillier/Paillier.hpp"
#include <vector>
namespace MDL {
template <class T = void>
struct GTInput {
    const MDL::EncVector& X;
    const MDL::EncVector& Y;
    long domain;
    long plainSpace;
};

template<>
struct GTInput<Paillier::Encryption> {
    const Paillier::Ctxt &X;
    const Paillier::Ctxt &Y;
    const Paillier::PubKey &pk;
    int bits;
    long domain;
};

template <class T = void>
struct GTResult {
    std::vector<MDL::EncVector>parts;
};

template <>
struct GTResult<Paillier::Encryption> {
    std::vector<Paillier::Ctxt> parts;
};

template<class T = void>
GTResult<T> GT(const GTInput<T>    & input,
               const EncryptedArray& ea);

template<class T = Paillier::Encryption>
GTResult<T> GT(const GTInput<T>    & input);

template<class T = void>
bool decrypt_gt_result(const GTResult<T>   & result,
                       const FHESecKey     & sk,
                       const EncryptedArray& ea);

bool decrypt_gt_result(const GTResult<Paillier::Encryption> &result,
                       int bit,
                       const Paillier::SecKey &sk);

} // namespace MDL
#endif // MDL_GT_HPP
