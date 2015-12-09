#ifndef MULTIPRECISION_MPREPLICATE_H
#define MULTIPRECISION_MPREPLICATE_H
#include <vector>
typedef std::vector<std::pair<long, long>> pairs_t;
class MPEncVector;
class MPEncArray;
class MPEncMatrix;
class MPPubKey;
void replicate(MPEncVector &vec,
               const MPEncArray &ea,
               const long c);

//repeat vec for k times
MPEncVector repeat(const MPEncVector &vec,
                   const MPEncArray &ea,
                   const MPPubKey &pk,
                   long columns,
                   long k);

void swap(MPEncVector &out, const MPEncVector &origin,
          const MPEncArray &ea, const pairs_t &pairs);
// row-major rearrange
void rearrange(MPEncVector &out, const MPEncVector &origin,
               const MPEncArray &ea,
               const long column, const long k);

// column-major rearrange
void rearrange2(MPEncVector &out, const MPEncMatrix &origin,
                const long column,
                const MPEncArray &ea,
                const MPPubKey &pk);
#endif // multiprecision/MPReplicate.h
