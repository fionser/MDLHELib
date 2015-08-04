#include "MPContext.hpp"
#include "MPSecKey.hpp"
MPSecKey::MPSecKey(const MPContext &context)
{
    skeys.reserve(context.partsNum());

    for (size_t i = 0; i < context.partsNum(); i++) {
        skeys.push_back(std::make_shared<FHESecKey>(*(context.get(i))));
    }

    for (auto &key : skeys) {
        key->GenSecKey(64);
        addSome1DMatrices(*key);
    }
}
