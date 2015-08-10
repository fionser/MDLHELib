#include "multiprecision/Multiprecision.h"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include "protocol/LR.hpp"
#include <thread>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

MDL::Vector<double> getTrueW(const MDL::Matrix<long> &XtX,
                             const MDL::Vector<long> &XtY)
{
    auto inv = XtX.inverse();
    return inv.dot(XtY.reduce(1.0));
}

void benchmarkLR(const MPContext &context,
                 const MPSecKey &sk,
                 const MPPubKey &pk,
                 const MPEncArray &ea)
{
    MPEncMatrix XtX;
    MPEncVector XtY(pk), MU(pk);
    auto _raw = load_csv("LR_10");
    auto _XtX = _raw.submatrix(0, -1, 0, 4);
    auto _XtY = _raw.submatrix(0, -1, 5, 5).vector();
    MDL::Vector<long> _MU(_XtY.dimension());

    for (long i = 0; i < _MU.dimension(); i++) _MU[i] = 10000;

    std::cout << "trueW " << getTrueW(_XtX, _XtY) << std::endl;
    XtX.pack(_XtX, pk, ea);
    XtY.pack(_XtY, ea);
    MU.pack(_MU, ea);

    MDL::MPMatInverseParam param = {pk, ea, 5};
    auto inv = MDL::inverse(XtX, MU, param);

    auto W = inv.sDot(XtY, pk, ea);
    MDL::Vector<NTL::ZZ> _W;
    W.unpack(_W, sk, ea);
    std::cout << _W << std::endl;
}

int main(int argc, char *argv[]) {
    long m, p, r, L, P;
    ArgMapping argmap;

    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("P", P, "P");
    argmap.parse(argc, argv);

    MDL::Timer keyTimer;
    keyTimer.start();
    MPContext context(m, p, r, P);
    context.buildModChain(L);
    MPSecKey sk(context);
    MPPubKey pk(sk);
    MPEncArray ea(context);
    keyTimer.end();
    std::cout << "slots " << ea.slots() <<
        " plainText: " << context.precision() << std::endl;
    benchmarkLR(context, sk, pk, ea);
    return 0;
}
