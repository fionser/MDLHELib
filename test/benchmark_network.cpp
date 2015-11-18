#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <unistd.h>
#include <string>
#include "utils/timer.hpp"
#include "network/network.hpp"
void act_client(std::string &addr, size_t sze) {
	int sock = nn_socket(AF_SP, NN_REQ);
	std::string host = "tcp://" + addr + ":12345";
	printf("host: %s\n", host.c_str());
	if (nn_connect(sock, host.c_str()) < 0) {
            printf("%s\nn", nn_strerror(errno));
            return;
	}

	std::vector<void *> data;	
	std::vector<int> _data(sze / sizeof(int));
	data.push_back((void *)_data.data());
	std::vector<size_t> lens = {sze};
	struct nn_msghdr nn_hdr;
	MDL::net::make_nn_header(&nn_hdr, data, lens);
	MDL::Timer timer;
	nn_send(sock, NULL, 0, 0);
	nn_recv(sock, NULL, 0, 0);
	timer.start();
	nn_sendmsg(sock, &nn_hdr, 0);
	nn_recv(sock, NULL, 0, 0);
	timer.end();
	printf("send %f\n", timer.second());
	MDL::net::free_header(&nn_hdr, false);
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
	std::vector<size_t> lens = {sze};	
	struct nn_msghdr nn_hdr;
	MDL::Timer timer;
	timer.start();
	MDL::net::make_nn_header(&nn_hdr, lens);
	nn_recvmsg(sock, &nn_hdr, 0);
	timer.end();
	nn_send(sock, NULL, 0, 0);
	MDL::net::free_header(&nn_hdr, true);
	printf("receive %f\n", timer.second());
}

int main(int argc, char *argv[]) {
	int oc;
	std::string addr;
	int role, sze;
	while((oc = getopt(argc, argv, "r:a:c:")) != -1) {
			switch (oc) {
			case 'a':
			addr = std::string(optarg);
			break;
			case 'c':
			sze = std::atoi(optarg);
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
