#ifndef PROTOCOL_PCA_HPP
#define PROTOCOL_PCA_HPP
#include <utility>
class EncryptedArray;
class FHEPubKey;

class MPEncArray;
class MPPubKey;
class MPEncMatrix;
class MPEncVector;
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

std::pair<MPEncVector, MPEncVector> runPCA(const MPEncMatrix &mat,
                                           const MPEncArray &ea,
                                           const MPPubKey &pk);
} // namespace MDL
#endif // PROTOCOL_PCA_HPP
