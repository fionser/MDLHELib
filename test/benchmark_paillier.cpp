#include "paillier/Paillier.hpp"
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
	omp_set_num_threads(sysconf( _SC_NPROCESSORS_ONLN ));
#pragma omp parallel for
    for (long i = 0; i < data.rows(); i++) {
        pk.Pack(ctxts[i], data[i]);
    }
    timer.end();
    printf("Enc %zd records cost %f sec\n", data.rows(), timer.second());
    return ctxts;
}

MDL::Paillier::Ctxt mean(const std::vector<MDL::Paillier::Ctxt> &ctxts) {
	MDL::Paillier::Ctxt sum(ctxts.front());
	auto sze = ctxts.size();
    MDL::Timer timer;
    timer.start();
	for (long i = 1; i < sze; i++) {
		sum += ctxts[i];
	}
    timer.end();
    printf("Mean of %zd records cost %f sec\n", sze, timer.second());
	return sum;
}

int main(int argc, char *argv[]) {
    std::string file;
    long key_len = 1024;
    ArgMapping argmap;
    argmap.arg("f", file, "file");
    argmap.arg("k", key_len, "key length");
    argmap.parse(argc, argv);

    auto data = load_csv(file);

    auto keys = MDL::Paillier::GenKey(key_len, 6);
    MDL::Paillier::SecKey sk(keys.first);
    MDL::Paillier::PubKey pk(keys.second);

    auto ctxts = encrypt(data, pk);
	auto res = mean(ctxts);
	std::vector<NTL::ZZ> slots;
	sk.Unpack(slots, res);
	for (auto s : slots) {
		std::cout << s << " ";
	}
	std::cout << "\n";
    return 0;
}
