#ifndef MULTIPRECISION_MPROTATE_H
#define MULTIPRECISION_MPROTATE_H
class MPEncVector;
class MPEncArray;
void rotate(MPEncVector &vec,
            const MPEncArray &ea,
            const long r);

void totalSums(MPEncVector &vec,
               const MPEncArray &ea,
               const long blockSize = 1);
#endif // multiprecision/MPRotate.h
