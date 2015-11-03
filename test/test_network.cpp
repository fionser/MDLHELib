#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <iostream>
#include <strstream>
#include "fhe/FHEcontext.h"
#include "fhe/FHE.h"
struct package {
    long total_byte;
    const char *buf;
};

void server(int sock) {
    FHEcontext context(64, 1031, 1);
    buildModChain(context, 3);

    package pkg;
    nn_recv (sock, &(pkg.total_byte), sizeof(pkg.total_byte), 0);
    char *buf = new char[pkg.total_byte];
    pkg.buf = buf;
    int byte = 0;
    char *p = buf;
    auto need_to_read = pkg.total_byte;
    do {
        int read = nn_recv(sock, p, need_to_read, 0);
        if (read < 0) {
            printf("%s\n", nn_strerror(errno));
            exit(-1);
        }
        byte += read;
        p += read;
        need_to_read -= read;
    } while (need_to_read > 0);

    FHEPubKey pk(context);
    {
        std::stringstream sstream(pkg.buf, pkg.total_byte);
        sstream >> pk;
    }

    Ctxt c1(pk);
    pk.Encrypt(c1, NTL::to_ZZX(2));
    {
        std::cout << c1 << std::endl;
        std::stringstream sstream;
        sstream << c1;
        const auto &str = sstream.str();
        package pkg = {(long)str.size(), str.c_str()};
        nn_send(sock, &pkg.total_byte, sizeof(pkg.total_byte), 0);
        printf("ctxt size %ld\n", pkg.total_byte);
        nn_send(sock, pkg.buf, pkg.total_byte, 0);
    }
    delete []buf;
}

void client(int sock) {
    FHEcontext context(64, 1031, 1);
    buildModChain(context, 3);
    FHESecKey sk(context);
    sk.GenSecKey(64);
    const FHEPubKey &pk = sk;

    std::stringstream sstream;
    sstream << pk;
    const auto &str = sstream.str();
    package pkg = { (long)str.size(), str.c_str() };
    long *tp = &pkg.total_byte;
    printf("going to send %ld\n", *tp);
    nn_send(sock, tp, sizeof(pkg.total_byte), 0);
    nn_send(sock, pkg.buf, pkg.total_byte, 0);
    {
        nn_recv(sock, &pkg.total_byte, sizeof(pkg.total_byte), 0);
        auto need_to_read = pkg.total_byte;
        printf("need to read %ld\n", need_to_read);
        char *buf = new char[pkg.total_byte];
        char *p = buf;
        do {
            int read = nn_recv(sock, p, need_to_read, 0);
            if (read < 0) {
                printf("recv error: %s\n", nn_strerror(errno));
                exit(-1);
            }
            need_to_read -= read;
            p += read;
        } while (need_to_read > 0);

        printf("read %ld : %s\n", pkg.total_byte - need_to_read, buf);
        std::stringstream sstream(buf, pkg.total_byte);
        Ctxt c1(pk);
        sstream >> c1;
        NTL::ZZX plain;
        sk.Decrypt(plain, c1);
        std::cout << plain << std::endl;
        delete []buf;
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        int sock = nn_socket(AF_SP, NN_REP);
        if (nn_bind(sock, "tcp://127.0.0.1:12345") < 0) {
            printf("%s\nn", nn_strerror(errno));
            return -1;
        }
        printf("SID %d\n", sock);
        server(sock);
    } else {
        int sock = nn_socket(AF_SP, NN_REQ);
        if (nn_connect(sock, "tcp://127.0.0.1:12345") < 0) {
            printf("%s\nn", nn_strerror(errno));
            return -1;
        }
        printf("SID %d\n", sock);
        client(sock);
    }
    return 0;
}
