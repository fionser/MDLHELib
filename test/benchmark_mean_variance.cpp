#include <fhe/FHEContext.h>
#include <fhe/FHE.h>
#include <fhe/NumbTh.h>
#include <fhe/EncryptedArray.h>
#include <utils/FileUtils.hpp>
#include <utils/timer.hpp>
#include <algebra/NDSS.h>
#include <thread>
#include <atomic>
#include <vector>
#include <iostream>

#ifdef FHE_THREADS
long WORKER_NR = 8;
#else // ifdef FHE_THREADS
long WORKER_NR = 1;
#endif // ifdef FHE_THREAD
std::pair<double, double> mean_and_std(const std::vector<double> &v) {
	double m = 0;
	for (auto vv : v) m += vv;
	m /= v.size();
	double s = 0;
	for (auto vv : v) s += (vv - m) * (vv - m);
	return {m, std::sqrt(s / v.size())};
}

std::vector<MDL::EncVector>encrypt(const MDL::Matrix<long>& data,
                                   const FHEPubKey        & pk,
                                   const EncryptedArray   & ea,
                                   long                     from = 0,
                                   long                     to = 0)
{
    to = to == 0 ? data.rows() : to;
    MDL::Timer timer;
    std::vector<MDL::EncVector> ctxts(to - from, pk);
    std::vector<std::thread>    workers;
    std::atomic<size_t> counter(from);

    timer.start();

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&data, &ea, &to, &from,
                                                 &counter, &ctxts]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < to) {
                ctxts[next - from].pack(data[next], ea);
            }
        })));
    }

    for (auto && wr : workers) wr.join();
    timer.end();
    //printf("Encrypt %ld data with %ld workers costed %f sec\n", to - from,
    //       WORKER_NR, timer.second());
    return ctxts;
}

MDL::EncVector mean(const std::vector<MDL::EncVector>& ctxts)
{
    std::vector<std::thread>    workers;
    std::vector<MDL::EncVector> partials(WORKER_NR, ctxts[0].getPubKey());
    std::atomic<size_t> counter(WORKER_NR);
    MDL::Timer timer;

    timer.start();

    for (long i = 0; i < WORKER_NR; i++) {
        partials[i] = ctxts[i];
        workers.push_back(std::move(std::thread([&counter, &ctxts]
                                                    (MDL::EncVector& ct) {
            size_t next;

            while ((next = counter.fetch_add(1)) < ctxts.size()) {
                ct += ctxts[next];
            }
        }, std::ref(partials[i]))));
    }

    for (auto && wr : workers) {
        wr.join();
    }

    for (long i = 1; i < WORKER_NR; i++) {
        partials[0] += partials[i];
    }
    timer.end();
    //printf("Sum %zd data with %ld workers costed %f sec\n", ctxts.size(),
    //       WORKER_NR, timer.second());
    return partials[0];
}

MDL::EncVector squareSum(std::vector<MDL::EncVector>& ctxts)
{
    MDL::Timer timer;
    std::atomic<size_t> counter(0);
    std::vector<std::thread> workers;

    timer.start();

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&ctxts, &counter]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < ctxts.size()) {
                //ctxts[next].square();
				ctxts[next] *= ctxts[next];
            }
        })));
    }

    for (auto && wr : workers) wr.join();
    auto sq_sum = mean(ctxts);
	sq_sum.reLinearize();
    timer.end();
    //printf("Square Sum costed %f sec\n", timer.second());
    return sq_sum;
}

MDL::EncVector variance(const MDL::Matrix<long>& data,
                        const EncryptedArray   & ea,
                        const FHEPubKey        & pk)
{
    MDL::EncVector square_sum(pk), sum_square(pk);
    NTL::ZZX   N(data.rows());
    const long BATCH_SIZE = 2500;
    MDL::Timer totalTimer, encTimer, evalTimer;

    totalTimer.start();
    for (long part = 0; part *BATCH_SIZE < data.rows(); part++) {
        long from  = std::min<long>(part * BATCH_SIZE, data.rows());
        long to    = std::min<long>(from + BATCH_SIZE, data.rows());
        encTimer.start();
        auto ctxts = encrypt(data, pk, ea, from, to);
        encTimer.end();
        evalTimer.start();

        if (part > 0) {
            sum_square += mean(ctxts);
            square_sum += squareSum(ctxts);
        } else {
            sum_square = mean(ctxts);
            square_sum = squareSum(ctxts);
        }
        evalTimer.end();
    }

    evalTimer.start();
    sum_square.square();
    square_sum.multByConstant(N);
    square_sum -= sum_square;
    evalTimer.end();
    totalTimer.end();
    printf("Varaice of %zd data with %ld workers used %f %f %f\n",
           data.rows(), WORKER_NR,
           encTimer.second(),
           evalTimer.second(),
           totalTimer.second());
    return square_sum;
}

MDL::EncVector average(const MDL::Matrix<long>& data,
                       const EncryptedArray &ea,
                       const FHEPubKey &pk)
{
    MDL::EncVector result(pk);
    const long BATCH_SIZE = 2500;
    MDL::Timer encTimer, evalTimer, totalTimer;
    totalTimer.start();
    for (long part = 0; part *BATCH_SIZE < data.rows(); part++) {
        long from  = std::min<long>(part * BATCH_SIZE, data.rows());
        long to    = std::min<long>(from + BATCH_SIZE, data.rows());
        encTimer.start();
        auto ctxts = encrypt(data, pk, ea, from, to);
        encTimer.end();
        evalTimer.start();
        if (part > 0) {
            result += mean(ctxts);
		} else {
			result = mean(ctxts);
        }
		evalTimer.end();
    }

    totalTimer.end();
    printf("Mean of %zd data with %ld workders used %f %f %f\n",
           data.rows(), WORKER_NR, encTimer.second(),
		   evalTimer.second(), totalTimer.second());
    return result;
}

void mean_run_std(const MDL::Matrix<long>&data, const EncryptedArray &ea,
				  const FHEPubKey &pk) {
	auto ctxts = encrypt(data, pk, ea, 0, 1000);
	for (long L : {5, 10, 20, 32}) {
		MDL::Timer timer;
		std::vector<double> times;
		for (long trial = 0; trial < 10; trial ++) {
			timer.start();
			MDL::EncVector result(pk);
			for (long part = 0; part < L; part ++) {
				if (part > 0) {
					result += mean(ctxts);
				} else {
					result = mean(ctxts);
				}
			}
			timer.end();
			times.push_back(timer.second());
			timer.reset();
		}
		auto ms = mean_and_std(times);
		printf("Mean %zd %f +- %f\n", L * 1000, ms.first, ms.second);
	}
}

void  var_run_std(const MDL::Matrix<long>& data,
				  const EncryptedArray   & ea,
				  const FHEPubKey        & pk) {
	auto ctxts = encrypt(data, pk, ea, 0, 1000);
	printf("workers %ld\n", WORKER_NR);
	for (long L : {5, 10, 20, 32}) {
		MDL::Timer timer;
		std::vector<double> times;
		for (long trial = 0; trial < 10; trial ++) {
    		MDL::EncVector square_sum(pk), sum_square(pk);
			for (long part = 0; part < L; part ++) {
				auto tmp(ctxts);
				timer.start();
				if (part > 0) {
					sum_square += mean(ctxts);
					square_sum += squareSum(tmp);
				} else {
					sum_square = mean(ctxts);
					square_sum = squareSum(tmp);
				}
				timer.end();
			}
			timer.start();
    		sum_square.square();
			square_sum.multByConstant(NTL::ZZX(L * 1000L));
			square_sum -= sum_square;
			timer.end();

			times.push_back(timer.second());
			timer.reset();
		}

		auto ms = mean_and_std(times);
		printf("Var %zd %f +- %f\n", L * 1000, ms.first, ms.second);
	}
}

int main(int argc, char *argv[]) {
    long m, p, r, L;
    ArgMapping argmap;

    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.parse(argc, argv);

    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;

    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);
	printf("slots %ld\n", ea.size());
    auto data = load_csv("adult.data", 5000);
	/* mean_run_std(data, ea, pk); */
	var_run_std(data, ea, pk);
    //MDL::Vector<long> ret;
    //for (long R : {5000, 10000, 15000, 20000, 25000, 0}) {
    //	auto data   = load_csv("adult.data", R);
    //auto var = variance(data, ea, pk);
    // var.unpack(ret, sk, ea);
    //}
    return 0;
}
