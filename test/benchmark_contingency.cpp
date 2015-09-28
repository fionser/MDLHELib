#include "algebra/NDSS.h"
#include "utils/timer.hpp"
#include "utils/FileUtils.hpp"
#include "utils/encoding.hpp"
#include <thread>
#include <mutex>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

static void process_in_log(MDL::EncVector& res,
                           const std::vector<MDL::EncVector>& input) {
    const size_t ele_nr = input.size();

    if (ele_nr > 1) {
        std::vector<MDL::EncVector> tmp((ele_nr + 1) / 2, input[0].getPubKey());
        const size_t sze = tmp.size();

        for (size_t i = 0; i < sze; i++) {
            tmp[i] = input[i];
        }

        for (size_t i = 0; i < sze && (sze + i) < ele_nr; i++) {
            tmp[i].multiplyBy(input[i + sze]);
        }
        process_in_log(res, tmp);
    } else {
        res = input[0];
    }
}

std::vector<std::vector<MDL::EncVector>>
encrypt(const MDL::Matrix<long> &mat,
        const FHEPubKey &pk,
        const EncryptedArray &ea,
        const std::vector<int> &cols)
{
    MDL::Timer encTimer;
    std::atomic<long> counter(0);
    std::vector<std::thread> workers;
    std::vector<std::vector<MDL::EncVector>> ctxts(mat.rows(),
                                              {cols.size(), pk});
    encTimer.start();
    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::thread([&mat, &counter,
                                      &ctxts, &ea, &cols]() {
        long next;
        while ((next = counter.fetch_add(1)) < mat.rows()) {
            for (int i = 0; i < cols.size(); i++) {
                auto attr = cols[i];
                auto encoded = MDL::encoding::indicator(mat[next][attr], ea);
                ctxts[next][i].pack(encoded, ea);
            }
        }
        }));
    }

    for (auto &&wr : workers) wr.join();
    encTimer.end();
    printf("enc %f sec\n", encTimer.second());
    return ctxts;
}

MDL::EncVector get_cell(std::vector<std::vector<MDL::EncVector>> &ctxts,
                        const EncryptedArray &ea,
                        const std::vector<int> &targets)
{
    MDL::Timer timer;
    const auto sze = ctxts.size();
    MDL::EncVector res{ctxts[0][0]};
    std::vector<std::thread> workers;
    std::atomic<size_t> counter(0);
    std::mutex lock;

    for (int i = 0; i < WORKER_NR; i++) {
        workers.push_back(std::thread([&]() {
            while (true) {
                auto next = counter.fetch_add(1);
                if (next >= sze) break;
                auto &vec = ctxts[next];
                for (int j = 1; j < targets.size(); j++) {
                    ea.rotate(vec[j], targets[0] - targets[j]);
                }

                MDL::EncVector product{vec[0]};
                process_in_log(product, vec);
                ea.rotate(product, -targets[0]);

                lock.lock();
                if (next > 0) res += product;
                else res = product;
                lock.unlock();
            }
        }));
    }

    timer.start();
    for (auto && wr : workers) wr.join();
    timer.end();

    printf("eval %f sec\n", timer.second());
    return res;
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

    MDL::Timer keyTimer;
    keyTimer.start();
    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
    FHEPubKey pk = sk;
    keyTimer.end();
    printf("KeyGen %f sec, Slot %ld\n", keyTimer.second(), ea.size());

    auto category = load_csv("category.data", N);
    printf("%zd %zd\n", category.rows(), category.cols());
    std::vector<int> cols{0, 1};
    auto ctxts = encrypt(category, pk, ea, cols);
    std::vector<int> targets{1, 3};
    auto cell = get_cell(ctxts, ea, targets);
    MDL::Vector<long> vec;
    cell.unpack(vec, sk, ea);
    std::cout << vec << std::endl;
    return 0;
}
