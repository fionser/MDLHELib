//

// Created by riku on 5/5/15.
//
#include "FHEUtils.hpp"
#include <fstream>

static void process_in_log(Ctxt& res, const std::deque<Ctxt>& input,
                           functor func) {
    const size_t ele_nr = input.size();

    if (ele_nr > 1) {
        std::deque<Ctxt> tmp((ele_nr + 1) / 2, Ctxt(input[0].getPubKey()));
        const size_t     sze = tmp.size();

        for (size_t i = 0; i < sze; i++) {
            tmp[i] = input[i];
        }

        for (size_t i = 0; i < sze && (sze + i) < ele_nr; i++) {
            func(tmp[i], input[i + sze]);
        }
        process_in_log(res, tmp, func);
    } else {
        res = input[0];
    }
}

static void process_in_log(Ctxt& res, const std::vector<Ctxt>& input,
                           functor func) {
    const size_t ele_nr = input.size();

    if (ele_nr > 1) {
        std::vector<Ctxt> tmp((ele_nr + 1) / 2, Ctxt(input[0].getPubKey()));
        const size_t sze = tmp.size();

        for (size_t i = 0; i < sze; i++) {
            tmp[i] = input[i];
        }

        for (size_t i = 0; i < sze && (sze + i) < ele_nr; i++) {
            func(tmp[i], input[i + sze]);
        }
        process_in_log(res, tmp, func);
    } else {
        res = input[0];
    }
}

void add_with_log_noise(Ctxt& res, const std::vector<Ctxt>& input) {
    process_in_log(res, input,
                   [](Ctxt& op1, const Ctxt& op2) {
        op1 += op2;
    });
}

void add_with_log_noise(Ctxt& res, const std::deque<Ctxt>& input) {
    process_in_log(res, input,
                   [](Ctxt& op1, const Ctxt& op2) {
        op1 += op2;
    });
}

void mul_with_log_noise(Ctxt& res, const std::vector<Ctxt>& input) {
    process_in_log(res, input,
                   [](Ctxt& op1, const Ctxt& op2) {
        op1.multiplyBy(op2);
    });
}

void mul_with_log_noise(Ctxt& res, const std::deque<Ctxt>& input) {
    process_in_log(res, input,
                   [](Ctxt& op1, const Ctxt& op2) {
        op1.multiplyBy(op2);
    });
}

/*!
 * @param n : phi(m)
 */
void pack_into_coeff(NTL::ZZX& plain, const std::vector<long>& input,
                     long n, bool reverse) {
    long sze = static_cast<long>(input.size());

    assert(sze > 0);
    assert(sze <= n);

    plain.SetLength(sze);

    if (reverse) {
        NTL::SetCoeff(plain, 0, input.back());

        for (long i = 0; i < sze - 1; i++) {
            NTL::SetCoeff(plain, n - i - 1, -input[i]);
        }
    } else {
        for (long i = 0; i < sze; i++) {
            NTL::SetCoeff(plain, i, input[i]);
        }
    }
}

void add_noise_to_coeff(Ctxt& res, long n, long p, long except) {
    NTL::ZZX noise;

    for (long i = 0; i < n; i++) {
        NTL::SetCoeff(noise, i, NTL::RandomBnd(p));
    }
    NTL::SetCoeff(noise, except, 0);
    res.addConstant(noise);
}

void dump_FHE_setting_to_file(const std::string& file, long k,
                              long m, long p,
                              long r, long L)
{
    FHEcontext context(m, p, r);

    FindM(k, L, 2, p, 0, 1, m, true);
    buildModChain(context, L);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    addSome1DMatrices(sk);
    std::fstream out;
    out.open(file, std::ios::out);

    if (!out.is_open()) {
        std::cerr << "Can not open file: " << file << std::endl;
        return;
    }
    out << context;
    out << sk;
    out.close();
}

void load_FHE_setting(const std::string& file,
                      FHEcontext& context, FHESecKey& sk)
{
    std::ifstream in;
    std::stringstream sstream;

    in.open(file);

    if (!in.is_open()) {
        std::cerr << "Can not open file: " << file << std::endl;
        return;
    }
    sstream << in.rdbuf();
    sstream >> context;
    sstream >> sk;

    // std::cout << str << std::endl;
    in.close();
}
