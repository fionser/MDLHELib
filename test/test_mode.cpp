#include "protocol/Mode.hpp"
#include "algebra/NDSS.h"
#include "fhe/FHEContext.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "utils/FHEUtils.hpp"
#include "utils/FileUtils.hpp"
#include "utils/encoding.hpp"
#include "utils/timer.hpp"

#include <vector>
#include <thread>
#ifdef FHE_THREADS
long NR_WORKERS = 8;
#else
long NR_WORKERS = 1;
#endif

long _ATTRIBUTE = 5;
MDL::Timer evalTimer, decTimer;
std::pair<double, double> mean_std(const std::vector<double> &v) {
	double m = 0;
	for (auto vv : v) m += vv;
	m /= v.size();

	double s = 0;
	for (auto vv : v) {
		s += (vv - m) * (vv - m);
	}

	if (v.size() > 1)
		s = std::sqrt(s / (v.size() - 1));
	else
		s = 0.0;

	return std::make_pair(m, s);
}

MDL::EncVector encrypt(const MDL::Matrix<long> &mat,
                       const FHEPubKey &pk,
                       const EncryptedArray &ea)
{
    std::vector<MDL::EncVector> ctxts(mat.rows(), pk);
    std::vector<std::thread> workers;
    std::atomic<long> counter(0);
    for (long wr = 0; wr < NR_WORKERS; wr++) {
        workers.push_back(std::thread([&mat, &counter,
                                       &ctxts, &ea]() {
            long next;
            while ((next = counter.fetch_add(1)) < mat.rows()) {
		auto attr = mat[next][_ATTRIBUTE];
		auto encoded = MDL::encoding::indicator(mat[next][_ATTRIBUTE], ea);
		ctxts[next].pack(encoded, ea);
            } }));
    }

    for (auto &&wr : workers) wr.join();

    evalTimer.start();
    for (size_t i = 1; i < mat.rows(); i++) {
        ctxts[0] += ctxts[i];
    }
    evalTimer.end();
    return ctxts[0];
}

int main(int argc, char *argv[]) {
    long m, p, r, L;
    long domainOfCategory = 8;
    ArgMapping argmap;
    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("A", _ATTRIBUTE, "the attribute");
    argmap.arg("D", domainOfCategory, "domainOfCategory");
    argmap.parse(argc, argv);

    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey  sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    FHEPubKey pk = sk;
    printf("slot %ld\n", ea.size());

    _ATTRIBUTE = 0;

    for (long N : {-1}) {
	    auto category = load_csv("category.data", N);
	    std::vector<double> evalTimes, decTimes;
            long mode;
            for (long trial = 0; trial < 10; trial++){
		    auto joined = encrypt(category, pk, ea);
		    MDL::Mode::Input input {joined, domainOfCategory, category.rows()};

		    evalTimer.start();
		    auto modeResults = MDL::computeMode(input, ea);
		    evalTimer.end();

		    decTimer.start();
		    mode = MDL::argMode(modeResults, sk, ea);
		    decTimer.end();

		    evalTimes.push_back(evalTimer.second());
		    decTimes.push_back(decTimer.second());
		    evalTimer.reset();
 		    decTimer.reset();
            }
            auto ms1 = mean_std(evalTimes);
            auto ms2 = mean_std(decTimes);
	    printf("%ld %ld %f +- %f, %f +- %f\n", N, mode, ms1.first, ms1.second,
                    ms2.first, ms2.second);
    }
    return 0;
}
