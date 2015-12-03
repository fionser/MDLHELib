#include "MPContext.hpp"
#include "fhe/NumbTh.h"
#include <set>
#include <NTL/ZZ.h>
#include <cmath>
#include <vector>
#include <thread>
#ifdef FHE_THREADS
const long WORKER_NR = 8;
#else
const long WORKER_NR = 1;
#endif

static long getSlots(long m, long p)
{
    return phi_N(m) / multOrd(p, m);
}

static long gcd(long a, long b) {
    if (a > b) std::swap(a, b);

    auto r = b % a;
    while (r > 0) {
        a = r;
        b = a;
        r = b % a;
    }
    return a;
}

bool checkSlots(long required, long check) {
    if (check < required) return false;
    return gcd(required, check) == required;
}

static std::set<long> FindPrimes(long m, long p, long parts)
{
    auto slots = getSlots(m, p);
    auto bits = static_cast<long>(std::ceil(std::log2(static_cast<double>(p))));
    std::set<long> primes;
	// {
	// 	std::vector<long> pprimes = {4139, 7321, 5381, 5783, 4231, 4937, 5279, 6679, 6323, 7459, 6791};
	// 	auto len = pprimes.size();
	// 	for (long pp = 0; pp < parts; pp++) primes.insert(pprimes[len - 1 - pp]);
	// 	return primes;
	// }
    primes.insert(p);
	long generated = 1;
	long trial = 0;
	while (generated < parts) {

		auto prime = NTL::RandomPrime_long(bits);
        auto s = getSlots(m, prime);
		if (checkSlots(slots, s)) {
			auto ok = primes.insert(prime);
			if (ok.second) {
				generated += 1;
			}
        }

		if (trial++ > 1000) {
			printf("Error: Can not find enough primes, only found %ld\n",
				   generated);
			break;
		}
	}

	return primes;
}

	MPContext::MPContext(long m, long p, long r, long parts)
: m_r(r)
{
	contexts.reserve(parts);
	auto primesSet = FindPrimes(m, p, parts);

	for (auto prime : primesSet) {
		m_plainSpace *= std::pow(prime, r);
		m_primes.push_back(prime);
	}

	std::vector<std::thread> worker;
	std::atomic<size_t> counter(0);
	const size_t num = m_primes.size();
	auto job = [this, &counter, &m, &r, &num]() {
		size_t i;
		while ((i = counter.fetch_add(1)) < num) {
			contexts[i] = std::make_shared<FHEcontext>(m, m_primes[i], r);
		}
	};

	contexts.resize(num);

	for (long wr = 0; wr < WORKER_NR; wr++) worker.push_back(std::thread(job));

	for (auto &&wr : worker) wr.join();
}

void MPContext::buildModChain(long L)
{
	std::vector<std::thread> worker;
	std::atomic<size_t> counter(0);
	const size_t num = contexts.size();
	auto job = [this, &counter, &num, &L]() {
		size_t i;
		while ((i = counter.fetch_add(1)) < num) {
			::buildModChain(*contexts[i], L);
		}
	};

	contexts.resize(num);

	for (long wr = 0; wr < WORKER_NR; wr++) worker.push_back(std::thread(job));

	for (auto &&wr : worker) wr.join();
}

double MPContext::precision() const
{
	return NTL::log(plainSpace()) / NTL::log(NTL::to_ZZ(2));
}
