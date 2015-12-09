#ifndef MULTIPRECISION_MPREPEAT_H
#define MULTIPRECISION_MPREPEAT_H
class MPEncVector;
class MPPubKey;
class MPEncArray;
void repeat(MPEncVector &out,
            const MPEncVector &origin,
            long k,
            const MPPubKey &pk,
            const MPEncArray &ea);
#endif // multiprecision/MPRepeat.h
