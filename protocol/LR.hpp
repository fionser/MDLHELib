#ifndef MDL_LR_HPP
#define MDL_LR_HPP

class FHEPubKey;
class EncryptedArray;
class EncMatrix;
namespace MDL
{
namespace LR {
const long ITERATION = 2;
} //namespace LR

struct MatInverseParam {
    const FHEPubKey &pk;
    const EncryptedArray &ea;
    const long columnsToProcess;
};

EncMatrix inverse(const EncMatrix &Q, long mu,
                  const MatInverseParam &param);
} // namespace MDL
#endif // MDL_LR_HPP
