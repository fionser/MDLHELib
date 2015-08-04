#include "LR.hpp"
#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "fhe/NumbTh.h"
#include "fhe/EncryptedArray.h"
#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include <thread>
namespace MDL
{
EncMatrix inverse(const EncMatrix &Q, long mu,
                  const MatInverseParam &param)
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

    for (int itr = 1; itr < LR::ITERATION; itr++) {
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
    printf("iteration time %f\n", timer.second());
    return R;
}
} // namespace MDL
