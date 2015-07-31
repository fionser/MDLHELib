#ifndef PROTOCOL_MODE_HPP
#define PROTOCOL_MODE_HPP
#include "algebra/EncVector.hpp"
#include "Gt.hpp"
#include "algebra/Matrix.hpp"
#include <vector>
#include <memory>
class EncryptedArray;
class FHESecKey;
namespace MDL {
namespace Mode {
struct Input {
    const EncVector &slots;
	struct {
		long processFrom;
	    long processTo;
	    long maximum;
	} gt;
    long valueDomain;
};
/// helper class to retrieval result of mode.
class Result {
public:
    typedef std::shared_ptr<Result> ptr;
    Result() {}
    /// make sure i < j.
    /// @return the comparison result of the i-th and the j-th slot.
    virtual std::pair<GTResult, bool> get(long i, long j) const = 0;
    virtual void put(const GTResult &result, long i, long j) = 0;
    virtual size_t matrixSize() const = 0;
};
} // namespace Mode

Mode::Result::ptr computeMode(const Mode::Input &input,
                              const EncryptedArray &ea,
							  Mode::Result::ptr &results);

void argMode(const Mode::Input &input,
			 const Mode::Result::ptr,
             const FHESecKey &sk,
             const EncryptedArray &ea,
			 Matrix<long> &mat);

long argMode(const Matrix<long> &mat);
} // namespace MDL
#endif // PROTOCOL_MODE_HPP
