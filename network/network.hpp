#ifndef NETWORK_NETWORK_HPP
#define NETWORK_NETWORK_HPP
#include <vector>
#include <nanomsg/nn.h>
namespace MDL {
namespace net {
const size_t MAX_ELEMENT_NR = 100;
struct msg_header {
    size_t msg_ele_nr;
    size_t msg_ele_sze[0];
};

size_t header_size(const msg_header *hdr);

size_t make_header(msg_header **hdr,
                   const std::vector<size_t> &lens);

size_t make_nn_header(struct nn_msghdr *hdr,
                      const std::vector<void *> &data,
                      const std::vector<size_t> &lens);

size_t make_nn_header(struct nn_msghdr *hdr,
                      const std::vector<size_t> &lens);

void free_header(msg_header *hdr);

void free_header(struct nn_msghdr *hdr, bool free_base);

template<class T>
long receive(T &obj, int sock);

template<class T>
long send(const T &obj, int sock);
} // namespace net
}; // namespace MDL
#endif // NETWORK_NETWORK_HPP
