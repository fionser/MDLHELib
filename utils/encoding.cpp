#include "encoding.hpp"
#include <algorithm>
namespace MDL {
namespace encoding {
Vector<long>indicator(long index, const EncryptedArray& ea)
{
    Vector<long> vec(ea.size());
    vec[index] = 1;
    return vec;
}

Vector<long>staircase(long index, const EncryptedArray& ea, long domain)
{
    Vector<long> vec(ea.size());
	domain = domain == 0 ? ea.size() : domain;
	auto itr = vec.begin();
	auto end = vec.begin();
	std::advance(itr, index);
	std::advance(end, domain);
	for (; itr != end; itr++) *itr = 1;
    return vec;
}
} // namepspace encoding
} // namepspace MDL
