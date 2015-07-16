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
    std::generate_n(random.begin(), range, []() {
        static long i = 1;
        return i++;
    });
    std::shuffle(random.begin(), random.end(), g);
    auto itr = random.begin();
    for (int p = 0; p < parts_nr; p++) {
        auto &vec = parts[p];
        auto end = std::distance(random.end(), itr) > slot_nr ? itr + slot_nr : random.end();
        std::copy(itr, end, vec.begin());
        itr = end;
    }
    return parts;
}

std::vector<NTL::ZZX> random_noise(long range, long slot_nr, long domain)
{
   auto parts_nr = (range + slot_nr - 1) / slot_nr;
   std::vector<NTL::ZZX> noises(parts_nr);
   for (auto &noise : noises) {
        noise.SetLength(slot_nr);
        for (long s = 0; s < slot_nr; s++) {
            NTL::SetCoeff(noise, s, 1 + NTL::RandomBnd(domain - 1));
        }
   }
   return noises;
}
