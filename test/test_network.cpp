#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <iostream>
#include <strstream>
#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "network/network.hpp"
#include <cstring>
void receive_pk(int socket, FHEPubKey &pk) {
    char *buf;
    int read;
    if ((read = nn_recv(socket, &buf, NN_MSG, 0)) < 0) {
        printf("receive pk error %s\n", nn_strerror(errno));
        exit(-1);
    }
    std::stringstream sstream;
    sstream.str(buf);
    sstream >> pk;
    nn_freemsg(buf);
}

void send_pk(int socket, const FHEPubKey &pk) {
    std::stringstream sstream;
    sstream << pk;
    auto str = sstream.str();
    if (nn_send(socket, str.c_str(), str.size(), 0) < 0) {
        printf("send pk error %s\n", nn_strerror(errno));
        exit(-1);
    }
}

void send_ctxts(int socket, const std::vector<Ctxt> &ctxts) {
    std::vector<void *> data;
    std::vector<size_t> lens;
    std::stringstream sstream;

    for (auto &ctxt : ctxts) {
        sstream.str("");
        sstream << ctxt;
        auto str = sstream.str();
        auto len = str.size();
        auto tmp = nn_allocmsg(len, 0);
        std::memcpy(tmp, str.c_str(), len);
        data.push_back(tmp);
        lens.push_back(len);
    }

    MDL::net::msg_header *hdr;
    MDL::net::make_header(&hdr, lens);
    nn_send(socket, hdr, MDL::net::header_size(hdr), 0);
    nn_recv(socket, NULL, 0, 0);

    struct nn_msghdr nn_hdr;
    MDL::net::make_nn_header(&nn_hdr, data, lens);
    nn_sendmsg(socket, &nn_hdr, 0);
    MDL::net::free_header(&nn_hdr, true);
}

void receive_ctxt(int socket, const FHEPubKey &pk,
                  std::vector<Ctxt> &ctxts) {
    std::stringstream sstream;
    char *buf;
    nn_recv(socket, &buf, NN_MSG, 0);
    nn_send(socket, NULL, 0, 0);
    MDL::net::msg_header *hdr = (MDL::net::msg_header *)buf;

    std::vector<size_t> lens(hdr->msg_ele_sze,
                             hdr->msg_ele_sze + hdr->msg_ele_nr);
    struct nn_msghdr nn_hdr;
    MDL::net::make_nn_header(&nn_hdr, lens);
    nn_recvmsg(socket, &nn_hdr, 0);

    Ctxt c(pk);
    for (size_t i = 0; i < nn_hdr.msg_iovlen; i++) {
        sstream.str((char *)nn_hdr.msg_iov[i].iov_base);
        sstream >> c;
        ctxts.push_back(c);
    }
}

void act_server(int socket) {
    FHEcontext context(1024, 1031, 1);
    buildModChain(context, 3);
    FHEPubKey pk(context);
    receive_pk(socket, pk);
    std::vector<Ctxt> ctxts(2, pk);
    pk.Encrypt(ctxts[0], NTL::to_ZZX(3));
    pk.Encrypt(ctxts[1], NTL::to_ZZX(3));
    ctxts[1].multiplyBy(ctxts[0]);
    send_ctxts(socket, ctxts);
    nn_close(socket);
}

void act_client(int socket) {
    FHEcontext context(1024, 1031, 1);
    buildModChain(context, 3);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    FHEPubKey &pk = sk;
    send_pk(socket, pk);
    std::vector<Ctxt> ctxts;
    receive_ctxt(socket, pk, ctxts);
    NTL::ZZX plain;
    for (auto &ctxt : ctxts) {
        sk.Decrypt(plain, ctxt);
        std::cout << plain << "\n";
    }
    nn_close(socket);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        int sock = nn_socket(AF_SP, NN_REP);
        if (nn_bind(sock, "ipc:///tmp/reqrep.ipc") < 0) {
            printf("%s\nn", nn_strerror(errno));
            return -1;
        }
        printf("SID %d\n", sock);
        act_server(sock);
    } else {
        int sock = nn_socket(AF_SP, NN_REQ);
        if (nn_connect(sock, "ipc:///tmp/reqrep.ipc") < 0) {
            printf("%s\nn", nn_strerror(errno));
            return -1;
        }
        printf("SID %d\n", sock);
        act_client(sock);
    }
    return 0;
}
