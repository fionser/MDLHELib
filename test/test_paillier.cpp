#include "paillier/Paillier.hpp"
#include <iostream>
int main() {
    auto keys = MDL::Paillier::GenKey(120, 6);
    MDL::Paillier::SecKey sk(keys.first);
    MDL::Paillier::PubKey pk(keys.second);
    MDL::Paillier::Ctxt ctxt(pk);
    std::vector<long> slots = {1, 2, 3};
    pk.Pack(ctxt, slots);
    ctxt *= 3;
    sk.Unpack(slots, ctxt);
    for (auto s : slots) {
        std::cout << s << " ";
    }
    std::cout <<"\n";
    return 0;
}
