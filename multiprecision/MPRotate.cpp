#include "MPRotate.h"
#include "Multiprecision.h"
void rotate(MPEncVector &vec,
            const MPEncArray &ea,
            const long r)
{
    auto parts = vec.partsNum();
    if (parts != ea.arrayNum()) return;
    for (long i = 0; i < parts; i++) {
        ea.get(i)->rotate(vec.get(i), r);
    }
}
