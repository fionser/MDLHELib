#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include <thread>
void benchmarkLR(const FHEPubKey &pk,
                 const FHESecKey &sk,
                 const EncryptedArray &ea)
{
    const long mu = 5652;
    MDL::Matrix<long> sigma = load_csv("covariance2.data");
    const long dimension = sigma.cols();
    MDL::Matrix<long> muR0 = MDL::eye(dimension);
    MDL::EncMatrix M(pk), R(pk);

    std::cout << sigma << std::endl;
    M.pack(sigma, ea);
    R.pack(muR0, ea);
    long MU = mu;
    MDL::Timer timer;
    for (auto &row : sigma.inverse()) {
    	std::cout << row << std::endl;
    } 
    timer.start();
    for (int itr = 0; itr < 2; itr++) {
        auto tmpR(R), tmpM(M);
        MDL::Vector<long> mag(ea.size(), 2 * MU);
	std::thread computeR([&tmpR, &ea, &dimension,
                              &R, &M, &mag](){
		tmpR.dot(M, ea, dimension);
		R.multByConstant(mag.encode(ea));
		R -= tmpR;
	});

	std::thread computeM([&tmpM, &ea, &dimension,
                              &M, &mag](){
		tmpM.dot(M, ea, dimension);
	});

	computeR.join();
        computeM.join();
	/*
	tmpR.dot(M, ea, dimension);
	R.multByConstant(mag.encode(ea));
	R -= tmpR;

	tmpM.dot(M, ea, dimension);
	 */
	M.multByConstant(mag.encode(ea));
	M -= tmpM;
        MU *= MU;
    }
    timer.end();
    std::cout << MU << std::endl;
    printf("Iteration %f\n", timer.second());
    M.unpack(muR0, sk, ea, true);
    for (auto &row : muR0.reduce(double(MU))) {
    	std::cout << row << std::endl;
    } 
    std::cout << "================" << std::endl;
    R.unpack(muR0, sk, ea, true);
    for (auto &row : muR0.reduce(double(MU))) {
    	std::cout << row << std::endl;
    } 
}

int main(int argc, char *argv[]) {
    long m, p, r, L;
    ArgMapping argmap;

    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.parse(argc, argv);

    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;

    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    printf("slot %ld\n", ea.size());
    benchmarkLR(pk, sk, ea);
}
