#include "LR.hpp"
#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include "multiprecision/Multiprecision.h"
#include <thread>
namespace MDL
{
MPEncMatrix inverse(const MPEncMatrix &Q, const MPEncVector &mu,
                    const MPMatInverseParam &param,
					const long ITERATION) {
    auto M(Q), R(Q);
    auto MU(mu);
    auto I = MDL::eye(param.columnsToProcess); I *= 2;
    for (int i = 0; i < ITERATION; i++) {
        auto tmp(M);
        tmp.negate();
        auto muI = mulMatrix(MU, I, param.ea);
        tmp += muI; // tmp = 2 * mu * I - M
        auto th1 = std::thread([&i, &R, &tmp, &param]() {
                               if (i != 0) {
                               R.dot(tmp, param.ea, param.pk,
                                     param.columnsToProcess); // R = R(2 * mu * I - M)
                               } else {
                               R = tmp;
                               }
                               });
        auto th2 = std::thread([&tmp, &M, &param]() {
                               M.dot(tmp, param.ea, param.pk,
                                     param.columnsToProcess); // M = M(2 * mu * I - M)
                               });
        th1.join();
        th2.join();
        MU.multiplyBy(MU);
    }
    return R;
}

EncMatrix inverse(const EncMatrix &Q, long mu,
                  const MatInverseParam &param,
				  const long ITERATION)
{
    long MU = mu;
    MDL::EncMatrix M(Q), R(Q);
    MDL::Timer timer;
    auto R0 = MDL::eye(param.columnsToProcess);
    timer.start();
    {
        R0 *= (2 * MU);
        R.negate();
        R.addConstant(R0, param.ea); // R1 = 2 * mu * I - I * M
        M.dot(R, param.ea, param.columnsToProcess); // M1 = Q * R1
        MU = MU * MU;
    }

    for (int itr = 1; itr < ITERATION; itr++) {
        auto tmpR(R), tmpM(M);
        MDL::Vector<long> mag(param.ea.size(), 2 * MU);
        std::thread computeR([&tmpR, &param, &R, &M, &mag](){
                             tmpR.dot(M, param.ea, param.columnsToProcess);
                             R.multByConstant(mag.encode(param.ea));
                             R -= tmpR;
                             });
#ifndef FHE_THREADS
        computeR.join();
#endif
        std::thread computeM([&tmpM, &param, &M, &mag](){
                             tmpM.dot(M, param.ea, param.columnsToProcess);
                             });
#ifdef FHE_THREADS
        computeR.join();
#endif
        computeM.join();
        M.multByConstant(mag.encode(param.ea));
        M -= tmpM;
        MU *= MU;
    }
    timer.end();
    return R;
}
} // namespace MDL
