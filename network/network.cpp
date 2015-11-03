#include "network.hpp"
#include <nanomsg/nn.h>
namespace MDL {
namespace net {
size_t header_size(const msg_header *hdr) {
    return sizeof(hdr->msg_ele_sze[0]) * hdr->msg_ele_nr
        + sizeof(hdr->msg_ele_nr);
}

size_t make_header(struct msg_header **hdr,
                   const std::vector<size_t> &lens) {
    size_t element_nr = 0;
    element_nr = std::min(lens.size(), MAX_ELEMENT_NR);
    (*hdr) = (msg_header *)nn_allocmsg(sizeof(*hdr) * (1 + element_nr), 0);
    struct msg_header *T = *hdr;
    T->msg_ele_nr = element_nr;
    std::memcpy(T->msg_ele_sze, lens.data(), sizeof(lens[0]) * element_nr);
    return element_nr;
}

size_t make_nn_header(struct nn_msghdr *hdr,
                      const std::vector<void *> &data,
                      const std::vector<size_t> &lens) {
    size_t msg_nr = 0;
    msg_nr = std::min(lens.size(), MAX_ELEMENT_NR);
    std::memset(hdr, 0, sizeof *hdr);
    hdr->msg_iov = (struct nn_iovec *)nn_allocmsg(sizeof(struct nn_iovec)
                                                  * msg_nr, 0);
    hdr->msg_iovlen = msg_nr;
    for (size_t i = 0; i < msg_nr; i++) {
        hdr->msg_iov[i].iov_base = data[i];
        hdr->msg_iov[i].iov_len = lens[i];
    }
    return msg_nr;
}

size_t make_nn_header(struct nn_msghdr *hdr,
                      const std::vector<size_t> &lens) {
    size_t msg_nr = 0;
    std::vector<void *>data;
    msg_nr = std::min(lens.size(), MAX_ELEMENT_NR);
    for (size_t i = 0; i < msg_nr; i++) {
        data.push_back(nn_allocmsg(lens[i] + 1, 0));
    }
    return make_nn_header(hdr, data, lens);
}

void free_header(struct nn_msghdr *hdr, bool free_base) {
    if (free_base) {
        for (size_t i = 0; i < hdr->msg_iovlen; i++) {
            nn_freemsg(hdr->msg_iov[i].iov_base);
        }
    }
    nn_freemsg(hdr->msg_iov);
}

void free_header(struct msg_header *hdr) {
    nn_freemsg(hdr);
}
}
};
