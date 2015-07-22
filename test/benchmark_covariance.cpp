#include <fhe/FHEContext.h>
#include <fhe/FHE.h>
#include <fhe/NumbTh.h>
#include <fhe/EncryptedArray.h>
#include <utils/FileUtils.hpp>
#include <utils/timer.hpp>
#include <algebra/NDSS.h>
#include <thread>
#include <vector>
typedef std::pair<MDL::EncVector, MDL::EncVector>mpair;
#ifdef FHE_THREADS
long WORKER_NR = 8;
#else // ifdef FHE_THREADS
long WORKER_NR = 1;
#endif // ifdef FHE_THREAD
std::vector<mpair>encrypt(const MDL::Matrix<long>& data,
                          const FHEPubKey        & pk,
                          const EncryptedArray   & ea,
                          long                     from = 0,
                          long                     to = 0)
{
    to = to == 0 ? data.rows() : to;
    MDL::Timer timer;
    std::vector<mpair> ctxts(to - from, { pk, pk });
    std::vector<std::thread> workers;
    std::atomic<size_t> counter(from);

    timer.start();

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&data, &ea, &to, &from,
                                                 &counter, &ctxts]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < to) {
                auto vec = covariance(data[next], data[next]).vector();
                ctxts[next - from].first.pack(data[next], ea);
                ctxts[next - from].second.pack(vec, ea);
            }
        })));
    }

    for (auto && wr : workers) wr.join();
    timer.end();
    printf("Encrypt %ld data with %ld workers costed %f sec\n", to - from,
           WORKER_NR, timer.second());
    return ctxts;
}

mpair summation(const std::vector<mpair>& ctxts)
{
    std::vector<std::thread>    workers;
    std::vector<MDL::EncVector> mu(WORKER_NR, ctxts[0].first.getPubKey());
    std::vector<MDL::EncVector> sigma(WORKER_NR, ctxts[0].first.getPubKey());

    std::atomic<size_t> counter(WORKER_NR);
    MDL::Timer timer;

    timer.start();

    for (long i = 0; i < WORKER_NR; i++) {
        mu[i] = ctxts[i].first;
        sigma[i] = ctxts[i].second;
        workers.push_back(std::move(std::thread([&counter, &ctxts]
                                                    (MDL::EncVector& vec,
                                                    MDL::EncVector& mat) {
            size_t next;

            while ((next = counter.fetch_add(1)) < ctxts.size()) {
                vec += ctxts[next].first;
                mat += ctxts[next].second;
            }
        }, std::ref(mu[i]), std::ref(sigma[i]))));
    }

    for (auto && wr : workers) wr.join();

    for (long i = 1; i < WORKER_NR; i++) {
        mu[0]    += mu[i];
        sigma[0] += sigma[i];
    }
    timer.end();
    return { mu[0], sigma[0] };
}

void benchmark(const EncryptedArray   & ea,
               const FHEPubKey        & pk,
               const FHESecKey        & sk,
               const MDL::Matrix<long>& data)
{
    const long BATCH_SIZE = 5000;
    MDL::Timer encTimer, evalTimer;
    MDL::EncVector mu(pk), sigma(pk);

    for (long part = 0; part *BATCH_SIZE < data.rows(); part++) {
        long from  = std::min<long>(part * BATCH_SIZE, data.rows());
        long to    = std::min<long>(from + BATCH_SIZE, data.rows());
        encTimer.start();
        auto ctxts = encrypt(data, pk, ea, from, to);
        encTimer.end();
        evalTimer.start();

        auto sum = summation(ctxts);
        mu    += sum.first;
        sigma += sum.second;
        evalTimer.end();
    }
    evalTimer.start();
    auto mu_mu = mu.covariance(ea, data.cols());
    NTL::ZZX N;
    std::vector<long> n(ea.size(), data.rows());
    ea.encode(N, n);
    sigma.multByConstant(N);
    evalTimer.end();
    for (size_t col = 0; col < data.cols(); col++) {
        ea.rotate(mu_mu[col], col * data.cols());
        sigma -= mu_mu[col];
    }
    MDL::Vector<long> mat;
    sigma.unpack(mat, sk, ea, true);
    for (int i = 0; i < data.cols(); i++) {
        for (int j = 0; j < data.cols(); j++) {
            std::cout << mat[i * data.cols() + j] << " ";
        }
        std::cout << std::endl;
    }
    printf("Covariance of %zd data, enc %f, eval %f\n", data.rows(),
           encTimer.second(), evalTimer.second());
}

int main(int argc, char *argv[]) {
    long m, p, r, L, R;
    ArgMapping argmap;
	MDL::Timer timer;
    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("R", R, "R");
    argmap.parse(argc, argv);
	timer.start();
    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;

    auto G       = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
	timer.end();
    printf("slots %ld\n", ea.size());
	printf("Key Gen %f\n", timer.second());
    auto data = load_csv("adult.data", R);
    benchmark(ea, pk, sk, data);
}
