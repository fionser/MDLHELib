#include "multiprecision/Multiprecision.h"
#include "protocol/PCA.hpp"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include <thread>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif
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

int main(int argc, char *argv[]) {
    ArgMapping argMap;
    long m = 0, p = 0, r = 0, L = 0, N = 0, P = 0, T = 3;
    std::string file;
    argMap.arg("m", m, "m: cyclotomic");
    argMap.arg("p", p, "p: plaintext space");
    argMap.arg("r", r, "r: lifting");
    argMap.arg("L", L, "L: Levels");
    argMap.arg("N", N, "N: How many record to use");
    argMap.arg("P", P, "P: How many primes to use");
    argMap.arg("T", T, "T: How many iteration");
    argMap.arg("f", file, "f: file");
    argMap.parse(argc, argv);
    if (m == 0 || p == 0 || r == 0 || L == 0) {
        printf("parameter!\n");
        return -1;
    }

    auto X = load_csv(file);
    MDL::Timer encTimer, evalTimer, decTimer, keyTimer;
#ifdef USE_EIGEN
    auto maxEigValue = X.maxEigenValue();
#endif
    keyTimer.start();
    MPContext context(m, p, r, P);
    context.buildModChain(L);
    MPSecKey sk(context);
    MPPubKey pk(sk);
    MPEncArray ea(context);
    keyTimer.end();
    std::cout << "slots " << ea.slots() << " plainText: " << context.precision() << std::endl;

    MPEncMatrix encMat;
    encMat.pack(X, pk, ea);
    std::vector<double> evalTimes, decTimes;
    for (long trial = 0; trial < 10; trial ++) {
	    evalTimer.start();
	    auto pca = MDL::runPCA(encMat, ea, pk, T);
	    evalTimer.end();
	    evalTimes.push_back(evalTimer.second());
            evalTimer.reset();

	    decTimer.start();
	    MDL::Vector<ZZ> v1, v2;
	    pca.first.unpack(v1, sk, ea);
	    pca.second.unpack(v2, sk, ea);
	    decTimer.end();
	    decTimes.push_back(decTimer.second());
            decTimer.reset();
#ifdef USE_EIGEN
	    if (trial > 0) continue;
	    auto approx = v1.L2() / v2.L2();
	    printf("%f %f\n", approx, maxEigValue);
	    std::cout << "Error: " << std::abs(approx - maxEigValue) / maxEigValue << std::endl;
#endif
    }
    auto ms1 = mean_std(evalTimes);
    auto ms2 = mean_std(decTimes);
    printf("%f %f\n", ms1.first, ms1.second);
    printf("%f %f\n", ms2.first, ms2.second);
    return 0;
}
