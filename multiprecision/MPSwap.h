#ifndef MULTIPRECISION_MPSWAP_H
#define MULTIPRECISION_MPSWAP_H
#include <vector>
class MPEncVector;
class MPPubKey;
class MPEncArray;
typedef std::vector<std::pair<int, int>> swap_t;
void batchSwap(MPEncVector &out,
               const MPEncVector &origin,
               const swap_t &locs,
               const MPPubKey &pk,
               const MPEncArray &ea);
#endif // multiprecision/MPSwap.h
