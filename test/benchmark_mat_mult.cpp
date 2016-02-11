#include <fhe/FHEContext.h>
#include <fhe/FHE.h>
#include <fhe/NumbTh.h>
#include <fhe/EncryptedArray.h>
#include <utils/timer.hpp>

#include <algebra/NDSS.h>
#include <vector>
void mean_std(const std::vector<double> &t) {
	double s1, s2;
	s1 = 0.0;
	for (auto tt : t) {
		s1 += tt;
	}
	s1 /= t.size();

	s2 = 0.0;
	for (auto tt : t) {
		s2 += (tt - s1) * (tt - s1);
	}
	s2 = std::sqrt(s2) / (t.size() - 1);
	printf("%f %f\n", s1, s2);
}

double size(const MDL::EncMatrix &mat) {
	std::stringstream str;
	str << mat.front();
	return (double) str.str().size();
}

int main() {
	/* long m = 27893; */
	/* long p = 4139; */
	long m = 5227;
	long p = 67499;
	long L = 32;
	long r = 1;
	FHEcontext context(m, p, r);
	auto G = context.alMod.getFactorsOverZZ()[0];
	EncryptedArray ea(context, G);
	std::cout << ea.size() << "\n";
	buildModChain(context, L);
	FHESecKey sk(context);
	sk.GenSecKey(64);
	addSome1DMatrices(sk);
	FHEPubKey pk = sk;
	MDL::EncMatrix entMat(pk);

	for (long D : {6, 12, 18, 24, 30, 36}) {
		MDL::Matrix<long> mat(D, D);
		for (long i = 0; i < D; i++)
			for (long j = 0; j < D; j++)
				mat[i][j] = i + j;
		entMat.pack(mat, ea);
		MDL::Timer timer;
		std::vector<double> tt;
		std::vector<double> ss;
		for (long i = 0; i < 10; i++) {
			auto tmp(entMat);
			printf("size %zd %zd\n", tmp.size(), entMat.size());
			timer.start();
			tmp.dot(entMat, ea, D);
			timer.end();
			tt.push_back(timer.second());
			timer.reset();
			//ss.push_back(size(tmp));
		}
		printf("%ld ", D);
		mean_std(tt);
		//mean_std(ss);
	}
	return 0;
}

