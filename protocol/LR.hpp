#ifndef MDL_LR_HPP
#define MDL_LR_HPP

class FHEPubKey;
class EncryptedArray;
class EncMatrix;
class MPPubKey;
class MPEncArray;
class MPEncMatrix;
class MPEncVector;
namespace MDL
{
namespace LR {
const long ITERATION = 2;
} //namespace

struct MatInverseParam {
    const FHEPubKey &pk;
    const EncryptedArray &ea;
    const long columnsToProcess;
};

struct MPMatInverseParam {
    const MPPubKey &pk;
    const MPEncArray &ea;
    const long columnsToProcess;
};

EncMatrix inverse(const EncMatrix &Q, long mu,
                  const MatInverseParam &param);

MPEncMatrix inverse(const MPEncMatrix &Q, const MPEncVector &mu,
                    const MPMatInverseParam &param);
} // namespace MDL
#endif // MDL_LR_HPP
