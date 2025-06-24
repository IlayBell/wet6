#include "L3.h"

using namespace common;


L3::L3(L4 &base, std::string packet_str): L4(base) {
	int bar_cnt = 0;

	std::string src_ip_str = extract_between_delimiters(packet_str,
														'|',
														bar_cnt,
														bar_cnt);
	bar_cnt++;

	std::string dst_ip_str = extract_between_delimiters(packet_str,
														'|',
														bar_cnt,
														bar_cnt);
	bar_cnt++;

	std::string ttl_str = extract_between_delimiters(packet_str,
													'|',
													bar_cnt,
													bar_cnt);
	bar_cnt++;

	std::string cs_str = extract_between_delimiters(packet_str,
													'|',
													bar_cnt);

	uint8_t* src_ip_arr = new uint8_t[IP_V4_SIZE];
	ip_to_arr(src_ip_str, src_ip_arr);
	this->src_ip = src_ip_arr;

	uint8_t* dst_ip_arr = new uint8_t[IP_V4_SIZE];
	ip_to_arr(dst_ip_str, dst_ip_arr);
	this->dst_ip = dst_ip_arr;

	this->ttl = std::stoi(ttl_str);
	this->cs = std::stoi(cs_str);
}

L3::L3(const L3 &base): L4(base) {
	uint8_t* src_ip_arr = new uint8_t[IP_V4_SIZE];
	uint8_t* dst_ip_arr = new uint8_t[IP_V4_SIZE];

	for (int i = 0; i < IP_V4_SIZE; i++) {
		src_ip_arr[i] = base.src_ip[i];
		dst_ip_arr[i] = base.dst_ip[i];
	}

	this->src_ip = src_ip_arr;
	this->dst_ip = dst_ip_arr;

	
	this->ttl = base.ttl;
	this->cs = base.cs;
}


bool L3::validate_packet(open_port_vec open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     uint8_t mac[MAC_SIZE]) {

	return this->ttl > 0 && this->cs == L3::calc_sum();
	
}

bool L3::proccess_packet(open_port_vec &open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     memory_dest &dst) {
	this->ttl--;
	this->cs = calc_sum();

	// Packet invalid
	if (ttl == 0) {
		return false;
	}

	/* dst ip same as NIC's => L4 (2.4) */
	if (comp_arr(ip, this->dst_ip, IP_V4_SIZE)) {
		
		dst = LOCAL_DRAM;	
		
		bool L4_flag = true;

		if (this->L4::validate_packet(open_ports, ip, mask, ip)) { // should not be ip pls rememeber to change!!!

			L4_flag = this->L4::proccess_packet(open_ports,
												ip,
												mask,
												dst);
		} else {
			L4_flag = false;
		}
		

		return this->ttl > 0 && L4_flag;
	}

	/* both belongs to local net => ignore (2.5) */
	if (in_local_net(ip, this->src_ip, mask) &&
		in_local_net(ip, this->dst_ip, mask)) {
		return false;
	}

	/* dst belongs, src doesnt => in (2.1)*/
	if (in_local_net(ip, this->dst_ip, mask)) {
		dst = RQ;

		return true;
	}

	/* src belongs, dst doesnt => out (2.2)*/
	if (in_local_net(ip, this->src_ip, mask)) {
		for (int i = 0; i < IP_V4_SIZE; i++) {
			this->src_ip[i] = ip[i];
		}
		
		this->cs = calc_sum();
		dst = TQ;
		return true;
	}

	
	/* none belongs to local => (2.3) */
	dst = TQ;
	return true;
}

bool L3::as_string(std::string &packet) {
	std::string L4_str = "";
	if (!this->L4::as_string(L4_str)) {
		return false;
	}

	packet = ip_to_str(this->src_ip) + "|" +
			 ip_to_str(this->dst_ip) + "|" +
			 std::to_string(this->ttl) + "|" +
			 std::to_string(this->cs) + "|" +
			 L4_str;

	return true;
}

L3::~L3() {
	delete[] this->src_ip;
	delete[] this->dst_ip;
}


unsigned int L3::calc_sum() const {
	unsigned int L4_sum = this->L4::calc_sum();

	unsigned int ips_sum = 0;
	for (int i = 0; i < IP_V4_SIZE; i++) {
		ips_sum += this->src_ip[i] + this->dst_ip[i];
	}

	unsigned int ttl_sum = L4::sum_bytes(this->ttl);

	return L4_sum + ips_sum + ttl_sum;
}


bool L3::in_local_net(uint8_t ip1[], uint8_t ip2[], uint8_t mask) {

	return get_masked_ip(ip1, mask) == get_masked_ip(ip2, mask);
}

std::string L3::get_masked_ip(uint8_t ip[], uint8_t mask) {
	std::string ip_str = "";

	for (int i = 0; i < IP_V4_SIZE; i++) {
		std::string bin_entry = dec_to_binary(ip[i]);
		ip_str += bin_entry;
	}

	std::string ip_masked = ip_str.substr(0, mask);

	return ip_masked;

}

std::string L3::ip_to_str(uint8_t ip[]) {
	std::string str = "";
	for (int i = 0; i < IP_V4_SIZE - 1; i++) {
		str += std::to_string(ip[i]) + ".";
	}

	str += std::to_string(ip[IP_V4_SIZE - 1]);

	return str;
}

void L3::ip_to_arr(std::string ip, uint8_t ip_arr[]){ 
	std::string chunk;
    for (int i = 0; i < IP_V4_SIZE - 1; i++) {
        chunk = extract_between_delimiters(ip, '.', i, i);

        ip_arr[i] = std::stoi(chunk);
    }

    chunk = extract_between_delimiters(ip, '.', IP_V4_SIZE - 1);
    ip_arr[IP_V4_SIZE - 1] = std::stoi(chunk);
}  

std::string L3::dec_to_binary(uint8_t num) {
	std::string bin_num = "00000000";

	for (int i = SIZE_OF_BYTE - 1; i >= 0; i--) {

		bin_num[i] = ((num % 2 == 1) ? '1' : '0');

		num /= 2;
	}

	return bin_num;
}

unsigned int L3::get_cs() const {
	return this->cs;
}

bool L3::comp_arr(uint8_t arr1[], uint8_t arr2[], int size){
    for (int i = 0; i < size; i++) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }

    return true;
}
