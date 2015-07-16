#include "GreaterThanUtils.hpp"
#include <random>
#include <algorithm>

std::vector<MDL::Vector<long> >permutated_range(long range, long slot_nr)
{
    auto parts_nr = (range + slot_nr - 1) / slot_nr;
    std::vector<MDL::Vector<long> > parts(parts_nr, slot_nr);
    std::vector<long>  random(range);
    std::random_device rd;
    std::mt19937 g(rd());
    long i = 1;
    std::generate_n(random.begin(), range, [&i]() {
        return i++;
    });
    std::shuffle(random.begin(), random.end(), g);
    auto first = random.begin();

    for (auto &vec : parts) {
        auto last = first;
        auto distance = std::distance(first, random.end());
        if (distance >= slot_nr)
            std::advance(last, slot_nr);
        else
            std::advance(last, distance);
        std::copy(first, last, vec.begin());
        first = last;
    }
    return parts;
}

std::vector<MDL::Vector<long>>random_noise(long domain, long slot_nr, long random_range)
{
    auto parts_nr = (domain + slot_nr - 1) / slot_nr;
    std::vector<MDL::Vector<long>> noises(parts_nr, MDL::Vector<long>(slot_nr));

    for (auto& noise : noises) {

        for (long s = 0; s < slot_nr; s++) {
            noise[s] = 1 + NTL::RandomBnd(random_range - 1);
        }
    }
    return noises;
}
