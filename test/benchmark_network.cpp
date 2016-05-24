#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <cmath>
#include "utils/timer.hpp"
#include "network/network.hpp"
std::pair<double, double> mean_std(const std::vector<double> &v) {
	double m = 0;
	for (auto vv : v) m += vv;
	m /= v.size();

	double s = 0;
	for (auto vv : v) {
		s += (vv - m) * (vv - m);
	}

	if (v.size() > 1)
		s = std::sqrt(s / (v.size() - 1));
	else
		s = 0.0;

	return std::make_pair(m, s);
}

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

	std::vector<double> times;
	MDL::Timer timer;
	nn_send(sock, NULL, 0, 0);
	nn_recv(sock, NULL, 0, 0);
   	for (int i = 0; i < 10; i++) {
		timer.start();
		MDL::net::send_all(sock, data, lens);
		timer.end();
		times.push_back(timer.second());
		//printf("send %zd %f\n", sze, timer.second());
        }
	nn_close(sock);
	auto ms = mean_std(times);
 	printf("send %zd %f +- %f\n", sze, ms.first, ms.second);
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

	std::vector<size_t> lens;
	make_package(sze, lens);

	nn_recv(sock, NULL, 0, 0);
	nn_send(sock, NULL, 0, 0);

	MDL::Timer timer;
	std::vector<double> times;
	for (int i = 0; i < 10; i++) {
		timer.start();
		MDL::net::receive_all(sock, lens);
		timer.end();
		times.push_back(timer.second());
	}
	auto ms = mean_std(times);
 	printf("receive %zd %f +- %f\n", sze, ms.first, ms.second);
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
			char *end;
			sze = static_cast<size_t>(1024 * 1024 * std::strtod(optarg, &end));
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
