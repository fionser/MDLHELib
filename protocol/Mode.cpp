#include "Mode.hpp"
#include "Gt.hpp"
#include "algebra/Matrix.hpp"
#include "fhe/EncryptedArray.h"
#include "fhe/replicate.h"
#include "fhe/FHE.h"
#include <vector>
#include <thread>

#ifdef FHE_THREADS
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
        if (i >= j || j >= m_matrixSize || j < 0) {
			printf("warnning in get\n"); return {{}, false};
		}
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
        if (i >= j || j >= m_matrixSize || j < 0) {
			printf("warnning in put i:%ld j:%ld max:%ld\n",
				   i, j, m_matrixSize);
			return;
		}
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

static long computeIndex(long row, long cols)
{
	if (row == 0) return 0;
	return (2 * cols - row - 1) * row / 2;
}

static std::pair<long, long> computeCoord(long index, long cols)
{
	long row = 0;
	while ((2 * cols - row - 1) * row / 2 <= index) { row++; }
	row = row - 1;
	long col = (index - computeIndex(row, cols)) + row + 1;
	return {row, col};
}

Mode::Result::ptr computeMode(const Mode::Input &input,
                              const EncryptedArray &ea,
							  Mode::Result::ptr &results)
{
	if (results == nullptr) {
    	results = std::make_shared<Mode::ConcreteResult>(input.gt.maximum + 1);
	}
    auto plainSpace = ea.getContext().alMod.getPPowR();
	auto fromIndex = computeIndex(input.gt.processFrom, input.gt.maximum + 1);
	auto toIndex = computeIndex(input.gt.processTo, input.gt.maximum + 1);
    std::atomic<long> counter(fromIndex);
    std::vector<std::thread> workers;
    for (long wr = 0; wr < NRWORKER; wr++) {
        workers.push_back(std::thread([&]() {
            long i;
            while ((i = counter.fetch_add(1)) < toIndex) {
                auto X(input.slots), Y(input.slots);
				auto coord = computeCoord(i, input.gt.maximum + 1);
                replicate(ea, X, coord.first);
                replicate(ea, Y, coord.second);

                GTInput gt{X, Y, input.valueDomain, plainSpace};
                results->put(GT(gt, ea), coord.first, coord.second);
            } }));
    }

    for (auto &&wr : workers) wr.join();
    return results;
}

void argMode(const Mode::Input &input,
			 const Mode::Result::ptr results,
             const FHESecKey &sk,
             const EncryptedArray &ea,
			 MDL::Matrix<long> &booleanMatrix)
{
    std::vector<std::thread> workers;
	auto fromIndex = computeIndex(input.gt.processFrom, input.gt.maximum + 1);
	auto toIndex = computeIndex(input.gt.processTo, input.gt.maximum + 1);
    std::atomic<long> counter(fromIndex);
	assert(results != nullptr);
    for (long wr = 0; wr < NRWORKER; wr++) {
        workers.push_back(std::thread([&]() {
        long index;
        while ((index = counter.fetch_add(1)) < toIndex) {
			auto coord = computeCoord(index, input.gt.maximum + 1);
			auto gt = results->get(coord.first, coord.second);
			if (!gt.second) { printf("warrning false gt!\n"); abort(); }
			bool isGt = decrypt_gt_result(gt.first, sk, ea);
			booleanMatrix[coord.first][coord.second] = isGt;
			booleanMatrix[coord.second][coord.first] = !isGt;
        } }));
    }

    for (auto &&wr : workers) wr.join();
}

long argMode(const MDL::Matrix<long> &mat)
{
	for (long i = 0; i < mat.rows(); i++) {
        bool flag = true;
        for (long j = 0; j < mat.cols(); j++) {
            if (j == i) continue;
            if (0 == mat[i][j]) { // i-th <= j-th
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
