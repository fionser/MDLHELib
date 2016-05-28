#include <fhe/FHEContext.h>
#include <fhe/FHE.h>
#include <fhe/NumbTh.h>
#include <fhe/EncryptedArray.h>
#include <fhe/replicate.h>

#include <utils/FileUtils.hpp>
#include <utils/timer.hpp>
#include <utils/encoding.hpp>

#include <protocol/Percentile.hpp>

#include <thread>
#include <atomic>
#ifdef FHE_THREADS
long WORKER_NR = 8;
#else // ifdef FHE_THREADS
long WORKER_NR = 1;
#endif // ifdef FHE_THREADS
std::pair<double, double> mean_std(const std::vector<double> &v) {
	double m = 0;
	for (auto vv : v) m += vv;
	m /= v.size();

	double s = 0;
	for (auto vv : v) {
		s += (vv - m) * (vv - m);
	}

	if (v.size() > 1)
		s = std::sqrt(s / (v.size() - 1));
	else
		s = 0.0;

	return std::make_pair(m, s);
}

MDL::Timer evalTimer, decTimer;

MDL::EncVector sum_ctxts(const std::vector<MDL::EncVector>& ctxts)
{
    std::vector<std::thread>    workers;
    std::vector<MDL::EncVector> partials(WORKER_NR, ctxts[0].getPubKey());
    std::atomic<size_t> counter(WORKER_NR);

    evalTimer.start();
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

    for (auto && wr : workers) wr.join();

    for (long i = 1; i < WORKER_NR; i++) partials[0] += partials[i];
    evalTimer.end();

    printf("Sum %zd ctxts with %ld workers costed %f sec\n", ctxts.size(),
           WORKER_NR, evalTimer.second());

    return partials[0];
}

std::pair<MDL::EncVector, long>load_file(const EncryptedArray& ea,
                                         const FHEPubKey     & pk,
					 const long N)
{
    auto data = load_csv("adult.data", N);
    std::vector<MDL::EncVector> ctxts(data.rows(), pk);
    std::atomic<size_t> counter(0);
    std::vector<std::thread> workers;
    MDL::Timer timer;

    timer.start();

    for (long wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::move(std::thread([&counter, &ea,
                                                 &ctxts, &data]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < data.rows()) {
                auto indicator = MDL::encoding::staircase(data[next][0], ea);
                ctxts[next].pack(indicator, ea);
            }
        })));
    }
    timer.end();

    for (auto && wr : workers) wr.join();
    //printf("Encrypt %ld records with %ld workers costed %f sec.\n", data.rows(), WORKER_NR, timer.second());
    return { sum_ctxts(ctxts), data.rows() };
}

std::vector<MDL::GTResult<void>>
k_percentile(const MDL::EncVector& ctxt,
             const FHEPubKey     & pk,
             const EncryptedArray& ea,
             long                  records_nr,
             long                  domain,
             long                  k)
{
    MDL::Timer timer;

    std::atomic<size_t> counter(0);
    std::atomic<long> countK(1);
    std::vector<std::thread> workers;
    std::vector<MDL::EncVector> replicated(domain, ctxt);
    std::vector<MDL::EncVector> encK(101, pk);

    evalTimer.start();
    /// replicate all positions
    /// and prepare all possible Ks.
    for (long wr = 0; wr < WORKER_NR; wr++) {
	workers.push_back(std::thread([&]() {
			  size_t d;
			  while ((d = counter.fetch_add(1)) < domain){
			      replicate(ea, replicated.at(d), d);
			  }
			  long kk;
			  while ((kk = countK.fetch_add(1)) <= 100) {
			      long _k = static_cast<long>(kk * records_nr / 100.0);
			      MDL::Vector<long> percentile(ea.size(), _k);
			      encK.at(kk).pack(percentile, ea);
			  }
			  }));
    }
    for (auto & wr : workers) wr.join();

    counter.store(0);
    workers.clear();

    std::vector<MDL::PercentileResult> percentileResults(101);
    for (long wr = 0; wr < WORKER_NR; wr++) {
	workers.push_back(std::thread([&]() {
		size_t d = 1;
                while ((d = counter.fetch_add(1)) <= 100) {
			MDL::PercentileParam param = { domain, records_nr, replicated, encK.at(d) };
			if (d == k)
				percentileResults.at(d) = k_percentile(param, ea, pk);
			else
				k_percentile(param, ea, pk);
		}
        }));
    }
    for (auto & wr : workers) wr.join();

    evalTimer.end();
    //printf("Domain %ld, Workers %ld Second %f\n", 
    //	   records_nr, WORKER_NR, timer.second());
    return percentileResults.at(k);
}

void decrypt(const std::vector<MDL::GTResult<void>>& gtresults,
             const FHESecKey& sk,
             const EncryptedArray& ea, long kpercent)
{
    std::vector<bool>   results(gtresults.size());
    std::atomic<size_t> counter(0);
    std::vector<std::thread> workers;

    decTimer.start();

    for (int wr = 0; wr < WORKER_NR; wr++) {
        workers.push_back(std::thread([&gtresults, &ea, &sk,
                                       &results, &counter]() {
            size_t next;

            while ((next = counter.fetch_add(1)) < results.size()) {
                results[next] = MDL::decrypt_gt_result(gtresults[next],
                                                       sk,
                                                       ea);
            }
        }));
    }

    for (auto && wr : workers) wr.join();
    decTimer.end();
    bool prev = true;

    for (size_t i = 0; i < results.size(); i++) {
        if (prev && !results[i]) {
            printf("%ld-percentile is %zd\n", kpercent, i);
            break;
        }
    }
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

    FHEcontext context(m, p, r);
    buildModChain(context, L);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    FHEPubKey pk = sk;

    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);

    for (long N : {-1}) {
	std::vector<double> evalTimes, decTimes;
	for (long trial = 0; trial < 10; trial++) {
		auto data      = load_file(ea, pk, N);
		long kpercent  = 50;
		auto gtresults = k_percentile(data.first,
				pk, ea,
				data.second,
				100,
				kpercent);
		decrypt(gtresults, sk, ea, kpercent);
        	evalTimes.push_back(evalTimer.second());
        	decTimes.push_back(decTimer.second());
		evalTimer.reset();
		decTimer.reset();
	} 
	auto ms1 = mean_std(evalTimes);
	auto ms2 = mean_std(decTimes);
	printf("%ld %f +- %f\n", N, ms1.first, ms1.second);
	printf("%f +- %f\n", ms2.first, ms2.second);
    }
    return 0;
}
