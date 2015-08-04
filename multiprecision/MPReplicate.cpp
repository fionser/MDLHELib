#include "MPReplicate.h"
#include "MPEncVector.hpp"
#include "MPEncArray.hpp"
#include "fhe/replicate.h"
void replicate(MPEncVector &vec,
               const MPEncArray &ea,
               const long c)
{
    if (vec.partsNum() != ea.arrayNum()) {
        printf("Warning! replicate vec size was mismatched with ea\n");
        return;
    }

    for (long i = 0; i < ea.arrayNum(); i++) {
        replicate(*ea.get(i), vec.get(i), c);
    }
}
