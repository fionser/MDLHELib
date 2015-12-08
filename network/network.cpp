#include "network.hpp"
#include <nanomsg/nn.h>
#include <stdio.h>
#include <cstring>
namespace MDL {
namespace net {
#ifdef USE_NETWORK
size_t header_size(const msg_header *hdr) {
    return sizeof(hdr->msg_ele_sze[0]) * hdr->msg_ele_nr
        + sizeof(hdr->msg_ele_nr);
}

size_t make_header(struct msg_header **hdr,
                   const std::vector<size_t> &lens) {
    size_t element_nr = lens.size();
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
    size_t msg_nr = lens.size();
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

void send_all(int socket,
              const std::vector<void *> &data,
              const std::vector<size_t> &lens) {
	struct nn_msghdr hdr;
	size_t to_send = data.size();

	std::memset(&hdr, 0, sizeof hdr);
    	hdr.msg_iov = (struct nn_iovec *)nn_allocmsg(sizeof(struct nn_iovec) * MAX_ELEMENT_NR, 0);
	size_t msg_nr = std::min(to_send, MAX_ELEMENT_NR);
	size_t i = 0;
	printf("to send %zd\n", to_send);
	while (to_send > 0) {
		for (int j = 0; j < msg_nr; j++) {
			hdr.msg_iov[j].iov_base = data[i + j];
			hdr.msg_iov[j].iov_len = lens[i + j];
		}
		hdr.msg_iovlen = msg_nr;
		nn_sendmsg(socket, &hdr, 0);
		nn_recv(socket, NULL, 0, 0);
		i += msg_nr;
		to_send -= msg_nr;
		msg_nr = std::min(to_send, MAX_ELEMENT_NR);
	}
	printf("sent all!\n");
	nn_freemsg(hdr.msg_iov);
}

void receive_all(int socket,
                 const std::vector<size_t> &lens) {
	struct nn_msghdr hdr;
	size_t to_receive = lens.size();
	size_t msg_nr = std::min(to_receive, MAX_ELEMENT_NR);

	std::memset(&hdr, 0, sizeof hdr);
    	hdr.msg_iov = (struct nn_iovec *)nn_allocmsg(sizeof(struct nn_iovec) * MAX_ELEMENT_NR, 0);
	for (size_t i = 0; i < MAX_ELEMENT_NR; i++) {
		hdr.msg_iov[i].iov_len = 0;
		hdr.msg_iov[i].iov_base= NULL;
	}

	size_t i = 0;
	printf("to receive %zd\n", to_receive);
	while (to_receive > 0) {
		for (size_t j = 0; j < msg_nr; j++) {
			if (hdr.msg_iov[j].iov_len == 0) {
				hdr.msg_iov[j].iov_base = nn_allocmsg(lens[i + j], 0);
		        } else if (hdr.msg_iov[j].iov_len < lens[i + j]) {
				hdr.msg_iov[j].iov_base = nn_reallocmsg(hdr.msg_iov[j].iov_base,
							                    lens[i + j]);
			}
			hdr.msg_iov[j].iov_len = lens[i + j];
		}
		hdr.msg_iovlen = msg_nr;
		nn_recvmsg(socket, &hdr, 0);
		nn_send(socket, NULL, 0, 0);
		i += msg_nr;
		to_receive -= msg_nr;
		msg_nr = std::min(to_receive, MAX_ELEMENT_NR);
	}
	printf("receive all!\n");
	nn_freemsg(hdr.msg_iov);
}

size_t make_nn_header(struct nn_msghdr *hdr,
                      const std::vector<size_t> &lens) {
    size_t msg_nr = lens.size();
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
#endif
}
};
