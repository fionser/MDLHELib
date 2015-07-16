#ifndef GREATER_THAN_UTILS_HPP
#define GREATER_THAN_UTILS_HPP
#include "algebra/Vector.hpp"
#include <NTL/ZZX.h>
#include <vector>

/// @param range [1, range]
/// @param slot_nr
/// @return several Vector<long> each of Vector<long> contains slot_nr integers.
std::vector<MDL::Vector<long>> permutated_range(long range, long slot_nr);

/// make random noise
/// @param range [1, range]
/// @param slot_nr
/// @param domain, the noise will choose from [1, domain)
/// @param return serveral polynomails that each slot contains a random noise.
std::vector<NTL::ZZX> random_noise(long range, long slot_nr, long domain);
#endif // GREATER_THAN_UTILS_HPP
