#include "encoding.hpp"
namespace MDL {
namespace encoding {
Vector<long>indicator(long index, const EncryptedArray& ea)
{
    Vector<long> vec(ea.size());
    vec[index] = 1;
    return vec;
}

Vector<long>staircase(long index, const EncryptedArray& ea)
{
    Vector<long> vec(ea.size());

    for (long i = index; i < ea.size(); i++) vec[i] = 1;
    return vec;
}
} // namepspace encoding
} // namepspace MDL
