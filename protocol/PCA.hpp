#ifndef PROTOCOL_PCA_HPP
#define PROTOCOL_PCA_HPP
#include <utility>
class EncryptedArray;
class FHEPubKey;
namespace MDL
{
class EncVector;
class EncMatrix;
namespace PCA {
    const long ITERATION = 4;
};

std::pair<EncVector, EncVector> runPCA(const EncMatrix &mat,
                                       const EncryptedArray &ea,
                                       const FHEPubKey &pk,
                                       const long dimension);
} // namespace MDL
#endif // PROTOCOL_PCA_HPP
