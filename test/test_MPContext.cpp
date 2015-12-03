#include "algebra/NDSS.h"
#include "fhe/FHE.h"
#include "utils/timer.hpp"
#include <NTL/ZZ.h>
#include <iostream>
int main(int argc, char *argv[]) {
	ArgMapping argmap;
	long m, L, p, r, D;
    argmap.arg("m", m, "m");
    argmap.arg("L", L, "L");
    argmap.arg("p", p, "p");
    argmap.arg("r", r, "r");
    argmap.arg("D", D, "D");
    argmap.parse(argc, argv);

    FHEcontext context(m, p, r);
	buildModChain(context, L);
    FHESecKey sk(context);
	sk.GenSecKey(64);
	addSome1DMatrices(sk);
    FHEPubKey pk = sk;
    auto G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);

    MDL::Vector<long> vec(D);
    MDL::Matrix<long> mat(D, D);
    vec[0] = 10; vec[1] = 10; vec[2] = 10;
    mat[0][0] = 1; mat[0][1] = 2; mat[0][2] = 3;
    mat[1][0] = 2; mat[1][1] = 3; mat[1][2] = 4;
    mat[2][0] = 3; mat[2][1] = 4; mat[2][2] = 5;

	MDL::EncVector encVec(pk);
    MDL::EncMatrix encMat(pk), encMat2(pk);
    encVec.pack(vec, ea);
    encMat.pack(mat, ea);
    encMat2.pack(mat, ea);
	{
		MDL::Timer time;
		time.start();
		encMat += encMat2;
		MDL::Matrix<ZZX> result;
		encMat.unpack(result, sk, ea, true);
		time.end();
		printf("X + Y %f\n", time.second());
	}
	{
		MDL::Timer time;
		auto tmp(encMat);
		time.start();
		tmp.dot(encMat2, ea, D);
		MDL::Matrix<ZZX> result;
		tmp.unpack(result, sk, ea, true);
		time.end();
		printf("XY %f\n", time.second());
	}
    /* { */
		/* MDL::Timer time; */
		/* time.start(); */
		/* MDL::Vector<ZZX> result; */
    /*     auto prod = encMat.column_dot(encVec, ea, D); */
    /*     prod.unpack(result, sk, ea, true); */
		/* time.end(); */
		/* printf("Xv %f %zd\n", time.second(), result.dimension()); */
    /* } */
    return 0;
}
