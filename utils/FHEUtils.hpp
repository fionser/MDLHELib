//

// Created by riku on 5/5/15.
//

#ifndef CCS2015_FHEUTILS_HPP
#define CCS2015_FHEUTILS_HPP
#include <NTL/ZZX.h>
#include "fhe/Ctxt.h"
#include "fhe/FHEcontext.h"
#include "fhe/FHE.h"
#include <cstring>
#include <vector>
#include <deque>
#include <strstream>
typedef void (*functor)(Ctxt      & op1,
                        const Ctxt& op2);

void add_with_log_noise(Ctxt                   & res,
                        const std::vector<Ctxt>& input);

void mul_with_log_noise(Ctxt                   & res,
                        const std::vector<Ctxt>& input);

/*!
 * @param plain: packing into this polynomial
 * @param n : phi(m)
 */
void pack_into_coeff(NTL::ZZX               & plain,
                     const std::vector<long>& input,
                     long                     n,
                     bool                     reverse = false);

/*!
 * @param n: phi(m)
 * @param p: plaintext range
 * @param except: no noise of this coefficient
 */
void add_noise_to_coeff(Ctxt& res,
                        long  n,
                        long  p,
                        long  except);

template<class T>
std::vector<unsigned char>fhe_convert(const T& v)
{
    std::stringstream sstream;

    sstream << v;
    const auto& str = sstream.str();
    const unsigned char *p = (const unsigned char *)str.c_str();
    std::vector<unsigned char> cpy(str.size());
    std::memcpy(cpy.data(), p, str.size());
    return cpy;
}

template<class T>
void fhe_convert(T& v, const std::vector<unsigned char>& bytes)
{
    const char *p = (const char *)bytes.data();
    std::string str(p, bytes.size());
    std::stringstream sstream(str);
    sstream >> v;
}

void dump_FHE_setting_to_file(const std::string& file,
                              long               k,
                              long               m,
                              long               p,
                              long               r,
                              long               L);

void load_FHE_setting(const std::string& file,
                      FHEcontext       & context,
                      FHESecKey        & sk);
#endif // CCS2015_FHEUTILS_HPP
