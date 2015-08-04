#ifndef MULTIPRECISION_MPSECKEY_HPP
#define MULTIPRECISION_MPSECKEY_HPP
#include "fhe/FHE.h"
#include <memory>
#include <vector>
class MPContext;
class MPSecKey {
public:
    typedef std::shared_ptr<FHESecKey> secKeyPtr;
    MPSecKey(const MPContext &context);

    secKeyPtr get(int index) const { return skeys[index]; }

    size_t keyNum() const { return skeys.size(); }
private:
    std::vector<secKeyPtr> skeys;
};
#endif // multiprecision/MPSecKey.hpp
