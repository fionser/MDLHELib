#include "multiprecision/MPContext.hpp"
#include "multiprecision/MPSecKey.hpp"
#include "multiprecision/MPPubKey.hpp"
#include "multiprecision/MPEncArray.hpp"
int main() {
    MPContext context(4097, 1087, 1, 5);
    MPSecKey sk(context);
    MPPubKey pk(sk);
    MPEncArray ea(context);
    printf("%f\n", context.precision());
    return 0;
}
