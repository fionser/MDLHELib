#include "MPPubKey.hpp"
#include "MPSecKey.hpp"
MPPubKey::MPPubKey(const MPSecKey &key)
{
    size_t keysNum = key.keyNum();
    pkeys.reserve(keysNum);
    for (long k = 0; k < keysNum; k++) {
        pkeys.push_back(std::make_shared<FHEPubKey>(*key.get(k)));
    }
}
