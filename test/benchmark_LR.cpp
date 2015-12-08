#include "multiprecision/Multiprecision.h"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include "protocol/LR.hpp"
#include "protocol/PCA.hpp"
#include <thread>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif
std::string gfile;
long gD;
long gMU;
#ifdef USE_EIGEN
MDL::Vector<double> getTrueW(const MDL::Matrix<long> &XtX,
                             const MDL::Vector<long> &XtY)
{
	printf("INFO X'X %zd %zd\n", XtX.rows(), XtX.cols());
    auto inv = XtX.inverse();
    return inv.dot(XtY.reduce(1.0));
}
#endif

void benchmarkLR(const MPContext &context,
                 const MPSecKey &sk,
                 const MPPubKey &pk,
                 const MPEncArray &ea)
{
    MPEncMatrix XtX;
    MPEncVector XtY(pk), MU(pk);
    MDL::Timer evalTimer, decTimer;
    auto _raw = load_csv(gfile);
    auto _XtX = _raw.submatrix(0, -1, 0, gD - 2);
    auto _XtY = _raw.submatrix(0, -1, gD - 1, gD - 1).vector();
    MDL::Vector<long> _MU(_XtY.dimension());

#ifdef USE_EIGEN
    std::cout << "trueW " << getTrueW(_XtX, _XtY) << std::endl;
#endif
    XtX.pack(_XtX, pk, ea);
    XtY.pack(_XtY, ea);
    evalTimer.start();
    /* auto pair = MDL::runPCA(XtX, ea, pk); */
	{
		/* MDL::Vector<ZZ> v1, v2; */
	    /* pair.first.unpack(v1, sk, ea); */
	    /* pair.second.unpack(v2, sk, ea); */
		/* auto mu = long(v1.L2() / v2.L2()); */
	    long mu = gMU;
		for (long i = 0; i < _MU.dimension(); i++) _MU[i] = mu;
    	MU.pack(_MU, ea);
		std::cout << "MU " << mu << std::endl;
	}

	MDL::Timer lrEvalTimer;
	lrEvalTimer.start();
    MDL::MPMatInverseParam param = {pk, ea, gD - 1};
    auto inv = MDL::inverse(XtX, MU, param);

    auto W = inv.sDot(XtY, pk, ea);
    evalTimer.end();
	lrEvalTimer.end();

    MDL::Vector<NTL::ZZ> _W;
    decTimer.start();
    W.unpack(_W, sk, ea);
    decTimer.end();

    auto factor = NTL::power(NTL::to_ZZ(_MU[0]), std::pow(2, MDL::LR::ITERATION));
    double dfactor;
    NTL::conv(dfactor, factor);
    std::cout << _W.reduce(dfactor) << std::endl;
    printf("%f %f %f\n", evalTimer.second(), lrEvalTimer.second(),
		   decTimer.second());
}

int main(int argc, char *argv[]) {
    long m, p, r, L, P;
    ArgMapping argmap;

    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("P", P, "P");
    argmap.arg("f", gfile, "file");
    argmap.arg("u", gMU, "mu");
	argmap.arg("D", gD, "dimension");
    argmap.parse(argc, argv);
	printf("m = %ld p = %ld r = %ld P = %ld L = %ld file = %s D = %ld Mu = %ld\n",
		   m, p, r, P, L, gfile.c_str(), gD, gMU);
    MDL::Timer keyTimer;
    keyTimer.start();
    MPContext context(m, p, r, P);
    context.buildModChain(L);
    MPSecKey sk(context);
    MPPubKey pk(sk);
    MPEncArray ea(context);
    keyTimer.end();
    std::cout << "slots " << ea.slots() << " plainText: " << context.precision() << std::endl;
    benchmarkLR(context, sk, pk, ea);
    return 0;
}
