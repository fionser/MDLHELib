#include "Mode.hpp"
#include "Gt.hpp"
#include "algebra/Matrix.hpp"
#include "fhe/EncryptedArray.h"
#include "fhe/replicate.h"
#include "fhe/FHE.h"
#include <vector>
#include <thread>

#ifdef FHE_THREAD
const long NRWORKER = 8;
#else
const long NRWORKER = 1;
#endif
namespace MDL {
namespace Mode {
class ConcreteResult : public Result {
public:
    explicit ConcreteResult(size_t size) : m_matrixSize(size) {
        m_results.resize(size * (size - 1) / 2);
    }

    std::pair<GTResult, bool> get(long i, long j) const {
        if (i >= j || j >= m_matrixSize || j < 0) return {{}, false};
        auto index = (m_matrixSize * i - ((i + 1) * i >> 1)) + j - i - 1;
        if (index >= m_results.size()) {
            printf("%ld %ld get over size\n", index, m_results.size());
            return {{}, false};
        }
        return {m_results[index], true};
    }

    size_t matrixSize() const { return m_matrixSize; }
    /// make sures i < j
    void put(const GTResult &result, long i, long j) {
        if (i >= j || j >= m_matrixSize || j < 0) return;
        auto index = (m_matrixSize * i - ((i + 1) * i >> 1)) + j - i - 1;
        if (index >= m_results.size()) {
            printf("%ld %ld put over size\n", index, m_results.size());
            return;
        }
        m_results[index] = result;
    }
private:
    const size_t m_matrixSize;
    std::vector<GTResult> m_results;
};
} // namespace Mode

Mode::Result::ptr computeMode(const Mode::Input &input,
                              const EncryptedArray &ea)
{
    auto results = std::make_shared<Mode::ConcreteResult>(input.slotToProcess);
    auto plainSpace = ea.getContext().alMod.getPPowR();
    std::atomic<long> counter(0);
    std::vector<std::thread> workers;

    for (long wr = 0; wr < NRWORKER; wr++) {
        workers.push_back(std::thread([&]() {
            long i;
            while ((i = counter.fetch_add(1)) < input.slotToProcess) {
                auto X(input.slots);
                replicate(ea, X, i);
                for (long j = i + 1; j < input.slotToProcess; j++) {
                    auto Y(input.slots);
                    replicate(ea, Y, j);
                    GTInput gt{X, Y, input.valueDomain, plainSpace};
                    results->put(GT(gt, ea), i, j);
                }
            } }));
    }

    for (auto &&wr : workers) wr.join();
    return results;
}

long argMode(const Mode::Result::ptr results,
             const FHESecKey &sk,
             const EncryptedArray &ea)
{
    auto size = results->matrixSize();
    Matrix<long> booleanMatrix(size, size);
    std::vector<std::thread> workers;
    std::atomic<long> counter(0);

    for (long wr = 0; wr < NRWORKER; wr++) {
        workers.push_back(std::thread([&]() {
        long i;
        while ((i = counter.fetch_add(1)) < size) {
            for (long j = i + 1; j < size; j++) {
                auto gt = results->get(i, j);
                assert(gt.second == true);
                bool isGt = decrypt_gt_result(gt.first, sk, ea);
                booleanMatrix[i][j] = isGt;
                booleanMatrix[j][i] = !isGt;
            }
        } }));
    }

    for (auto &&wr : workers) wr.join();

    for (long i = 0; i < size; i++) {
        bool flag = true;
        for (long j = 0; j < size; j++) {
            if (j == i) continue;
            if (!booleanMatrix[i][j]) { // i-th <= j-th
                flag = false;
                break;
            }
        }
        if (flag) return i;
    }

    printf("warnning! no mode!?\n");
    return -1;
}
} // namespace MDL
