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

int main(int argc, char *argv[]) {
    ArgMapping argMap;
    long m = 0, p = 0, r = 0, L = 0, N = 0, P = 0, M = 100;
    argMap.arg("m", m, "m: cyclotomic");
    argMap.arg("p", p, "p: plaintext space");
    argMap.arg("r", r, "r: lifting");
    argMap.arg("L", L, "L: Levels");
    argMap.arg("N", N, "N: How many record to use");
    argMap.arg("P", P, "P: How many primes to use");
    argMap.arg("M", M, "M: Magnifier");
    argMap.parse(argc, argv);
    if (m == 0 || p == 0 || r == 0 || L == 0) {
        printf("parameter!\n");
        return -1;
    }
    auto X = load_csv("PCA_1000");
    MDL::Timer encTimer, evalTimer, decTimer, keyTimer;
#ifdef USE_EIGEN
    auto maxEigValue = X.maxEigenValue();
#endif
    keyTimer.start();
    MPContext context(m, p, r, P);
    context.buildModChain(L);
    std::cout << "context done" << std::endl;
    MPSecKey sk(context);
    std::cout << "sk done" << std::endl;
    MPPubKey pk(sk);
    std::cout << "pk done" << std::endl;
    MPEncArray ea(context);
    std::cout << "ea done" << std::endl;
    keyTimer.end();
    std::cout << "slots " << ea.slots() << " plainText: " << context.precision() << std::endl;


    MPEncMatrix encMat;
    encMat.pack(X, pk, ea);

    std::cout << "run PCA" << std::endl;
    evalTimer.start();
    auto pca = MDL::runPCA(encMat, ea, pk);
    evalTimer.end();

    decTimer.start();
    MDL::Vector<ZZ> v1, v2;
    pca.first.unpack(v1, sk, ea);
    pca.second.unpack(v2, sk, ea);
    decTimer.end();

    auto approx = v1.L2() / v2.L2();
#ifdef USE_EIGEN
    printf("%f %f\n", approx, maxEigValue);
    std::cout << "Error: " << std::abs(approx - maxEigValue) / maxEigValue << std::endl;
#endif
    printf("KeyGen\tEnc\tEval\tDec\n%f\t%f\t%f\t%f\n",
           keyTimer.second(), encTimer.second(),
           evalTimer.second(), decTimer.second());
    return 0;
}
