#include "encoding.hpp"
namespace MDL {
namespace encoding {
Vector<long>indicator(long index, const EncryptedArray& ea)
{
    return indicator(index, ea.size());
}

Vector<long>staircase(long index, const EncryptedArray& ea)
{
    return staircase(index, ea.size());
}

Vector<long> indicator(long index, size_t slots_per_cipher) {
    Vector<long> vec(slots_per_cipher);
    vec[index] = 1;
    return vec;
}

Vector<long> staircase(long index, size_t slots_per_cipher) {
    Vector<long> vec(slots_per_cipher);
    for (size_t i = index; i < slots_per_cipher; i++) vec[i] = 1;
    return vec;
}

} // namepspace encoding
} // namepspace MDL
