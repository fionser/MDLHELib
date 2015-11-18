#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <iostream>
#include <strstream>
#include <cstring>
#include "fhe/FHEContext.h"
#include "fhe/FHE.h"
#include "network/network.hpp"
#include "utils/timer.hpp"

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
    MDL::Timer timer;

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
    timer.start();
    nn_sendmsg(socket, &nn_hdr, 0);
    timer.end();
    printf("sent %zd ctxt %f s\n", ctxts.size(), timer.second());
    MDL::net::free_header(&nn_hdr, true);
}

void receive_ctxt(int socket, const FHEPubKey &pk,
                  std::vector<Ctxt> &ctxts) {
    std::stringstream sstream;
    MDL::Timer timer;
    char *buf;
    // nn_recv(socket, &buf, NN_MSG, 0);
    // nn_send(socket, NULL, 0, 0);
    MDL::net::msg_header *hdr = (MDL::net::msg_header *)buf;

    std::vector<size_t> lens(hdr->msg_ele_sze,
                             hdr->msg_ele_sze + hdr->msg_ele_nr);
    timer.start();
    struct nn_msghdr nn_hdr;
    MDL::net::make_nn_header(&nn_hdr, lens);
    nn_recvmsg(socket, &nn_hdr, 0);
    printf("received\n");
    Ctxt c(pk);
    for (size_t i = 0; i < nn_hdr.msg_iovlen; i++) {
        sstream.str((char *)nn_hdr.msg_iov[i].iov_base);
        sstream >> c;
        ctxts.push_back(c);
    }
    timer.end();
    printf("receive %zd ciphers %fs\n", ctxts.size(), timer.second());
}

long gM, gP, gR, gL;
long gC = 1;
void act_server(int socket) {
    FHEcontext context(gM, gP, gR);
    buildModChain(context, gL);
    FHEPubKey pk(context);
    receive_pk(socket, pk);

    std::vector<Ctxt> ctxts(gC, pk);
    for (long i = 0; i < gC; i++)
        pk.Encrypt(ctxts[i], NTL::to_ZZX(i));
    send_ctxts(socket, ctxts);
    nn_close(socket);
}

void act_client(int socket) {
    FHEcontext context(gM, gP, gR);
    buildModChain(context, gL);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    FHEPubKey &pk = sk;
    send_pk(socket, pk);

    std::vector<Ctxt> ctxts;
    receive_ctxt(socket, pk, ctxts);
    nn_close(socket);
}

int main(int argc, char *argv[]) {
    ArgMapping mapping;
    long role = 0;
    std::string host = "127.0.0.1";
    mapping.arg("m", gM, "m");
    mapping.arg("p", gP, "p");
    mapping.arg("r", gR, "r");
    mapping.arg("L", gL, "L");
    mapping.arg("R", role, "role, 0:server, 1:client");
    mapping.arg("H", host, "host");
    mapping.arg("C", gC, "cipher to send");
    mapping.parse(argc, argv);

    if (role == 0) {
        int sock = nn_socket(AF_SP, NN_REP);
        if (nn_bind(sock, "tcp://*:12345") < 0) {
            printf("%s\n", nn_strerror(errno));
            return -1;
        }
        printf("SID %d\n", sock);
        act_server(sock);
    } else if (role == 1){
        int sock = nn_socket(AF_SP, NN_REQ);
        std::string h = "tcp://" + host + ":12345";
        if (nn_connect(sock, h.c_str()) < 0) {
            printf("%s\nn", nn_strerror(errno));
            return -1;
        }
        printf("SID %d\n", sock);
        act_client(sock);
    }
    return 0;
}
