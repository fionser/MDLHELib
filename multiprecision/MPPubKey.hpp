#ifndef MULTIPRECISION_MPPUBKEY_HPP
#define MULTIPRECISION_MPPUBKEY_HPP
#include "fhe/FHE.h"
#include <vector>
#include <memory>
class MPSecKey;
class MPPubKey {
public:
    typedef std::shared_ptr<FHEPubKey>  pubKeyPtr;
    MPPubKey(const MPSecKey &sk);

    pubKeyPtr get(int index) const { return pkeys[index]; }

    size_t keyNum() const { return pkeys.size(); }
private:
    std::vector<pubKeyPtr> pkeys;
};
#endif // multiprecision/MPPubKey.hpp
