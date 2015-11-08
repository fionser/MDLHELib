#include "Paillier/Paillier.hpp"
#include "utils/FileUtils.hpp"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include <vector>

std::vector<MDL::Paillier::Ctxt> encrypt(const MDL::Matrix<long> &data,
                                         const MDL::Paillier::PubKey &pk) {
    MDL::Timer timer;
    timer.start();
    MDL::Paillier::Ctxt c(pk);
    std::vector<MDL::Paillier::Ctxt> ctxts(data.rows(), c);
    for (long i = 0; i < data.rows(); i++) {
        pk.Pack(ctxts[i], data[i]);
    }
    timer.end();
    printf("Enc %zd records cost %f sec\n", data.rows(), timer.second());
    return ctxts;
}

int main(int argc, char *argv[]) {
    std::string file;
    long key_len = 1024;
    ArgMapping argmap;
    argmap.arg("f", file, "file");
    argmap.arg("k", key_len, "key length");
    argmap.parse(argc, argv);

    auto data = load_csv(file);

    auto keys = MDL::Paillier::GenKey(key_len, 32);
    MDL::Paillier::SecKey sk(keys.first);
    MDL::Paillier::PubKey pk(keys.second);

    auto ctxts = encrypt(data, pk);
    return 0;
}
