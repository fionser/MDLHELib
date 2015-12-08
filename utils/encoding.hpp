#ifndef ENCODING_HPP
#define ENCODING_HPP
#include <algebra/Vector.hpp>
#include <fhe/EncryptedArray.h>
namespace MDL {
namespace encoding {
Vector<long> indicator(long index, const EncryptedArray &ea);
Vector<long> staircase(long index, const EncryptedArray &ea);
Vector<long> indicator(long index, size_t slots_per_cipher);
Vector<long> staircase(long index, size_t slots_per_cipher);
} // namepspace encoding
} // namepspace MDL
#endif // ENCODING_HPP
