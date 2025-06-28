#include "NIC_sim.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

using namespace common;

/**
* @fn nic_sim
* @brief Constructor of the class.
* 
* @param param_file - File name containing the NIC's parameters.
*
* @return New simulation object.
*/
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

/**
* @fn nic_flow
* @brief Process and store to relevant location all packets in packet_file.
*
* @param packet_file - Name of file containing packets as strings.
*
* @return None.
*/
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

/**
* @fn nic_print_results
* @brief Prints all data stored in memory to stdout in the following format:
*
*        LOCAL DRAM:
*        [src] [dst]: [data - DATA_ARR_SIZE bytes]
*        [src] [dst]: [data - DATA_ARR_SIZE bytes]
*        ...
*
*        RQ:
*        [each packet in separate line]
*
*        TQ:
*        [each packet in separate line]
*
* @return None.
*/
void nic_sim::nic_print_results() {
	std::cout << "LOCAL DRAM:" << std::endl;

	for (const open_port& prt: this->open_ports) {
		std::cout << prt.src_prt << " ";
		std::cout << prt.dst_prt << ": ";

		const unsigned char* data_arr = prt.data;
		
		std::string data_str = L4::arr_dec_to_hex(data_arr, DATA_ARR_SIZE);
		std::cout << data_str <<  std::endl;
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

/**
* @fn ~nic_sim
* @brief Destructor of the class.
*
* @return None.
*/
nic_sim::~nic_sim() {
	delete[] this->nic_mac;
	delete[] this->nic_ip;
}

/**
* @fn packet_factory
* @brief Gets a string representing a packet, creates the corresponding
*        packet type, and returns a pointer to a generic_packet.
*
* @param packet - String representation of a packet.
*
* @return Pointer to a generic_packet object.
*/
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

/**
* @fn seperate_ip_mask
* @brief takes the string "ip/mask" and seperates them.
* @param ip_mask - the string of ip and mask. format:
                  "ip/mask".
* @param ip_arr = the array to write the ip into.
* @return the mask as an int.
*/
uint8_t nic_sim::seperate_ip_mask(std::string ip_mask, uint8_t* ip_arr) {
	int idx_sep = 0;
	while(ip_mask[idx_sep++] != '/');

	std::string ip = ip_mask.substr(0, idx_sep - 1);

	L3::ip_to_arr(ip, ip_arr);

	std::string mask = ip_mask.substr(idx_sep);

	return std::stoi(mask);
}

/**
* @fn create_L4
* @brief creates an object L4 from string.
* @param packet - string representing the packet. format:
                  "src_port|dst_port|addrs|L5_data".
* @return pointer to L4 packet.
*/
L4* nic_sim::create_L4(std::string &packet) {
	L4* L4_packet = new L4(packet);
	return L4_packet;
}

/**
* @fn create_L3
* @brief creates an object L3 from string.
* @param packet - string representing the packet. format:
                  "src_ip|dst_ip|ttl|cs|L4_packet".
* @return pointer to L3 packet.
*/
L3* nic_sim::create_L3(std::string &packet) {

	int idx_sep = 0;
	int bar_cnt = 0;

	while(bar_cnt < L3_BAR_NUM) { bar_cnt += (packet[idx_sep++] == '|'); };

	std::string L3_str = packet.substr(0, idx_sep - 1);
	std::string L4_str = packet.substr(idx_sep);

	L4* L4_packet = this->create_L4(L4_str);
	L3* L3_packet = new L3(*L4_packet, L3_str);

	delete L4_packet;

	return L3_packet;
}

/**
* @fn create_L2
* @brief creates an object L2 from string.
* @param packet - string representing the packet. format:
                  "src_mac|dst_mac|L3_packet|cs".
* @return pointer to L2 packet.
*/
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

	delete L3_packet;

	return L2_packet;
}