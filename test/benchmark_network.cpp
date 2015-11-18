#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include "utils/timer.hpp"
#include "network/network.hpp"
void make_package(size_t sze, std::vector<size_t> &lens) {
	const size_t _1KB = 1024;
	while (sze >= _1KB) {
		lens.push_back(_1KB);
		sze -= _1KB;
	}
	if (sze > 0)
		lens.push_back(sze);
}

size_t check(struct nn_msghdr *nn_hdr, size_t sze) {
	size_t count = 0;
	for (size_t i = 0; i < nn_hdr->msg_iovlen; i++) {
		count += nn_hdr->msg_iov[i].iov_len;
	}
	return count;
}

void act_client(std::string &addr, size_t sze) {
	int sock = nn_socket(AF_SP, NN_REQ);
	std::string host = "tcp://" + addr + ":12345";
	printf("host: %s\n", host.c_str());
	if (nn_connect(sock, host.c_str()) < 0) {
            printf("%s\nn", nn_strerror(errno));
            return;
	}

	std::vector<size_t> lens;
	make_package(sze, lens);
	std::vector<void *> data;	
	for (auto len : lens)
		data.push_back((void *)new char[len]);
	
	MDL::Timer timer;
	nn_send(sock, NULL, 0, 0);
	nn_recv(sock, NULL, 0, 0);

	timer.start();
	MDL::net::send_all(sock, data, lens);
	timer.end();
	printf("send %f\n", timer.second());
	nn_close(sock);

	for (auto p : data) {
		char *pp = (char *)p;
		delete []pp;
	}
}

void act_server(size_t sze) {
	int sock = nn_socket(AF_SP, NN_REP);
	std::string host = "tcp://*:12345";
	if (nn_bind(sock, host.c_str()) < 0) {
            printf("Error: %s\n", nn_strerror(errno));
            return;
	}

	nn_recv(sock, NULL, 0, 0);
	nn_send(sock, NULL, 0, 0);

	std::vector<size_t> lens;
	make_package(sze, lens);

	MDL::Timer timer;
	timer.start();
	MDL::net::receive_all(sock, lens);
	timer.end();

	printf("receive %f\n", timer.second());
	nn_close(sock);
}

int main(int argc, char *argv[]) {
	int oc, role;
	std::string addr;
	size_t sze;
	while((oc = getopt(argc, argv, "r:a:c:")) != -1) {
			switch (oc) {
			case 'a':
			addr = std::string(optarg);
			break;
			case 'c':
			sze = std::strtol(optarg, NULL, 10);
			break;
			case 'r':
			role = std::atoi(optarg);
			break;
			}
	}

	switch (role) {
	case 0:
	act_client(addr, sze);
	break;
	case 1:
	act_server(sze);
	break;
	}	
	return 0;	
}
