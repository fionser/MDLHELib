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

MDL::EncVector encrypt(const MDL::Matrix<long> &mat,
                       const FHEPubKey &pk,
                       const EncryptedArray &ea)
{
    std::vector<MDL::EncVector> ctxts(mat.rows(), pk);
    std::vector<std::thread> workers;
    std::atomic<long> counter(0);
    MDL::Timer encTimer, sumTimer;
    const long ATTRIBUTE = 0;
    encTimer.start();
    for (long wr = 0; wr < NR_WORKERS; wr++) {
        workers.push_back(std::thread([&mat, &counter,
                                       &ctxts, &ea]() {
            long next;
            while ((next = counter.fetch_add(1)) < mat.rows()) {
                auto encoded = MDL::encoding::indicator(mat[next][ATTRIBUTE], ea);
                ctxts[next].pack(encoded, ea);
            } }));
    }

    for (auto &&wr : workers) wr.join();
    encTimer.end();

    sumTimer.start();
    for (size_t i = 1; i < mat.rows(); i++) {
        ctxts[0] += ctxts[i];
    }
    sumTimer.end();
    printf("enc %f, joined %f\n", encTimer.second(), sumTimer.second());
    return ctxts[0];
}

int main(int argc, char *argv[]) {
    long m, p, r, L, N;
    ArgMapping argmap;
    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("N", N, "N");
    argmap.parse(argc, argv);

    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey  sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    FHEPubKey pk = sk;
    printf("Slot %ld\n", ea.size());

    MDL::Timer evalTimer, decTimer, totalTimer;
    auto category = load_csv("category.data", N);
	N = category.rows();
	printf("%zd records\n", category.rows());
    auto joined = encrypt(category, pk, ea);
    const long domainOfCategory = 20;
	const long BATCHSZE = domainOfCategory / 10;
    MDL::Mode::Input input {joined, {0, 0, domainOfCategory}, N};
	MDL::Matrix<long> mat(domainOfCategory + 1, domainOfCategory + 1);

	totalTimer.start();
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
	auto evalT = evalTimer.second();
	auto decT = decTimer.second();
    printf("The mode is %ld-th category. Eval Dec Other\n %f %f %f\n",
		   MDL::argMode(mat), evalT, decT, totalTimer.second() - evalT - decT);
    return 0;
}
