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
enum class Attribute {
	WORKCLASS = 0,
	EDUCATION = 1
};

enum class Domain {
	WORKCLASS = 8,
	EDUCATION = 16,
};

MDL::EncVector encrypt(const MDL::Matrix<long> &mat,
                       const FHEPubKey &pk,
                       const EncryptedArray &ea,
					   MDL::Timer &encTimer,
					   MDL::Timer &evalTimer)
{
    std::vector<MDL::EncVector> ctxts(mat.rows(), pk);
    std::vector<std::thread> workers;
    std::atomic<long> counter(0);
    const long ATTRIBUTE = static_cast<long>(Attribute::WORKCLASS);
    encTimer.start();
    for (long wr = 0; wr < NR_WORKERS; wr++) {
        workers.push_back(std::thread([&mat, &counter,
                                       &ctxts, &ea]() {
            long next;
            while ((next = counter.fetch_add(1)) < mat.rows()) {
			    auto attr = mat[next][ATTRIBUTE];
				if (attr > 0) attr -= 1;
                auto encoded = MDL::encoding::indicator(attr, ea);
                ctxts[next].pack(encoded, ea);
            } }));
    }

    for (auto &&wr : workers) wr.join();
    encTimer.end();

    evalTimer.start();
    for (size_t i = 1; i < mat.rows(); i++) {
        ctxts[0] += ctxts[i];
    }
    evalTimer.end();
   // printf("enc %f, joined %f\n", encTimer.second(), sumTimer.second());
    return ctxts[0];
}

int main(int argc, char *argv[]) {
    long m, p, r, L, N;
	const long TRIAL = 10;
    ArgMapping argmap;
    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("N", N, "N");
    argmap.parse(argc, argv);

	auto category = load_csv("category.data", N);
	MDL::Timer encTimer, evalTimer, decTimer, totalTimer;
	for (long trial = 0; trial < TRIAL; trial++) {
		totalTimer.start();
		FHEcontext context(m, p, r);
		buildModChain(context, L);
		FHESecKey  sk(context);
		sk.GenSecKey(64);
		addSome1DMatrices(sk);
		auto G = context.alMod.getFactorsOverZZ()[0];
		EncryptedArray ea(context, G);
		FHEPubKey pk = sk;
		if (trial == 0) printf("Slot %ld ", ea.size());
		N = category.rows();

		auto joined = encrypt(category, pk, ea, encTimer, evalTimer);
		const long domainOfCategory = static_cast<long>(Domain::WORKCLASS);
		const long BATCHSZE = std::max<long>(domainOfCategory, 1);
		MDL::Mode::Input input {joined, {0, 0, domainOfCategory}, N};
		MDL::Matrix<long> mat(domainOfCategory + 1, domainOfCategory + 1);

		for (long from = 0; from <= domainOfCategory; from += BATCHSZE) {
			MDL::Mode::Result::ptr modeResults = nullptr;

			evalTimer.start();
			input.gt.processFrom = from;
			input.gt.processTo = std::min(from + BATCHSZE, domainOfCategory);
			MDL::computeMode(input, ea, modeResults);
			evalTimer.end();

			decTimer.start();
			MDL::argMode(input, modeResults, sk, ea, mat);
			decTimer.end();
		}
		totalTimer.end();
		if (trial == 0) printf(" mode:%ld\n", MDL::argMode(mat));
	}
	printf("%ld %f %f %f %f\n", N,
		   encTimer.second()/TRIAL, evalTimer.second()/TRIAL,
		   decTimer.second()/TRIAL, totalTimer.second()/TRIAL);
	return 0;
}
