#include "NIC_sim.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

using namespace common;


nic_sim::nic_sim(std::string param_file) {
	std::ifstream file(param_file);

	if(!file.is_open()) {
		throw std::invalid_argument("Could not open the file.");
	}

	std::string mac;
	if(!std::getline(file, mac)) {
		throw std::invalid_argument("No MAC address in file");
	}

	std::string ip_mask;
	if(!std::getline(file, ip_mask)) {
		throw std::invalid_argument("No IP address in file");
	}

	uint8_t* mac_arr = new uint8_t[MAC_SIZE];
	L2::mac_to_arr(mac, mac_arr);
	this->nic_mac = mac_arr;

	uint8_t* ip_arr = new uint8_t[IP_V4_SIZE];
	uint8_t mask = seperate_ip_mask(ip_mask, ip_arr);

	this->nic_ip = ip_arr;
	this->nic_mask = mask;

	std::string port_str;

	while (std::getline(file, port_str)) {
		int curr_idx = 1;
		while(port_str[curr_idx - 1] != ':') {
			curr_idx++;
		}

		int cnt_length = 0;
		while(port_str[curr_idx + cnt_length] != ',') {
			cnt_length++;
		}

		std::string src_str = port_str.substr(curr_idx, cnt_length);

		curr_idx += cnt_length + 1;
		while(port_str[curr_idx - 1] != ':') {
			curr_idx++;
		}

		cnt_length = 0;
		while(port_str[curr_idx + cnt_length] != '\0') {
			cnt_length++;
		}

		std::string dst_str = port_str.substr(curr_idx, cnt_length);

		struct open_port prt;

		prt = open_port(std::stoi(dst_str), std::stoi(src_str));
		
		this->open_ports.push_back(prt);
	}
}

void nic_sim::nic_flow(std::string packet_file) {
	std::ifstream file(packet_file);

	if(!file.is_open()) {
		throw std::invalid_argument("Could not open the file.");
	}

	std::string packet_str;
	while(std::getline(file, packet_str)) {
		generic_packet* packet = this->packet_factory(packet_str);

		if (packet->validate_packet(this->open_ports, 
									this->nic_ip,
									this->nic_mask,
									this->nic_mac)) {

			memory_dest dst = LOCAL_DRAM;

			packet->proccess_packet(this->open_ports, 
									this->nic_ip,
									this->nic_mask,
									dst);

			std::string packet_str;
			packet->as_string(packet_str);

			switch (dst) {
				case memory_dest::RQ:
					this->RQ.push_back(packet_str);
					break;

				case memory_dest::TQ:
					this->TQ.push_back(packet_str);
					break;

				case memory_dest::LOCAL_DRAM:
					break;
			}
		}

		delete packet;
	}
}

void nic_sim::nic_print_results() {
	std::cout << "LOCAL_DRAM" << std::endl;

	for (const open_port& prt: this->open_ports) {
		std::cout << prt.src_prt << " ";
		std::cout << prt.dst_prt << ": ";

		const unsigned char* data_arr = prt.data;
		
		std::string data_str = L4::arr_dec_to_hex(data_arr, DATA_ARR_SIZE);
		std::cout << data_str << std::endl;
	}

	std::cout << "\nRQ:" << std::endl;
	for (std::string entry: this->RQ) {
		std::cout << entry << std::endl;
	}

	std::cout << "\nTQ:" << std::endl;

	for (std::string entry: this->TQ) {
		std::cout << entry << std::endl;
	}
}

nic_sim::~nic_sim() {
	delete[] this->nic_mac;
	delete[] this->nic_ip;
}

generic_packet* nic_sim::packet_factory(std::string &packet) {
	/* L2 is distinct because of MAC address at first, 
	   which each entry has fixed size */
	if (packet[MAC_CLASSIFIER] == ':') {
		return this->create_L2(packet);
	}

	/* L3 is distinct because of IP address at first,
	   which each entry has max size (0-255 so 3) */
	bool is_L3 = false;
	for (int i = 0; i <= MAX_IP_SIZE; i++) {
		if (packet[i] == '.') {
			is_L3 = true;
		}
	}

	if(is_L3) {
		return this->create_L3(packet);
	}

	return this->create_L4(packet);
}

uint8_t nic_sim::seperate_ip_mask(std::string ip_mask, uint8_t* ip_arr) {
	int idx_sep = 0;
	while(ip_mask[idx_sep++] != '/');

	std::string ip = ip_mask.substr(0, idx_sep - 1);

	L3::ip_to_arr(ip, ip_arr);

	std::string mask = ip_mask.substr(idx_sep);

	return std::stoi(mask);
}

L4* nic_sim::create_L4(std::string &packet) {
	L4* L4_packet = new L4(packet);
	return L4_packet;
}

L3* nic_sim::create_L3(std::string &packet) {

	int idx_sep = 0;
	int bar_cnt = 0;

	while(bar_cnt < L3_BAR_NUM) { bar_cnt += (packet[idx_sep++] == '|'); };

	std::string L3_str = packet.substr(0, idx_sep - 1);
	std::string L4_str = packet.substr(idx_sep);

	L4* L4_packet = this->create_L4(L4_str);
	L3* L3_packet = new L3(*L4_packet, L3_str);

	return L3_packet;
}

L2* nic_sim::create_L2(std::string &packet) {
	int idx_sep = 0;
	int bar_cnt = 0;

	while(bar_cnt < L2_BAR_NUM) { bar_cnt += (packet[idx_sep++] == '|'); };

	std::string L3_str = packet.substr(idx_sep);

	int cs_l2_length = 0;
	while (packet[packet.length() - cs_l2_length] != '|') { cs_l2_length++; };

	L3_str = L3_str.substr(0, L3_str.length() - cs_l2_length);


	L3* L3_packet = this->create_L3(L3_str);
	L2* L2_packet = new L2(*L3_packet, packet);

	return L2_packet;
}