#ifndef MULTIPRECISION_MPENCARRAY_HPP
#define MULTIPRECISION_MPENCARRAY_HPP
#include "fhe/EncryptedArray.h"
#include <vector>
#include <memory>
class MPContext;
class MPEncArray {
public:
    typedef std::shared_ptr<EncryptedArray> encArrayPtr;
    MPEncArray(const MPContext &context);

    encArrayPtr get(int index) const { return arrays[index]; }

    long slots() const { return minimumSlot; }

    size_t arrayNum() const { return arrays.size(); }
private:
    long minimumSlot = 0;
    std::vector<encArrayPtr> arrays;
};
#endif // multiprecision/MPEncArray.hpp
