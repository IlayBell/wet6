#include "L2.h"

using namespace common;


L2::L2(L3 &base, std::string packet_str): L3(base) {
	int bar_cnt = 0;

	std::string src_mac_str = extract_between_delimiters(packet_str,
														'|',
														bar_cnt,
														bar_cnt);
	bar_cnt++;

	std::string dst_mac_str = extract_between_delimiters(packet_str,
														'|',
														bar_cnt,
														bar_cnt);
	bar_cnt++;

	std::string cs_str = extract_between_delimiters(packet_str,
													'|',
													CS_L2_BAR_NUM);

	uint8_t* src_mac_arr = new uint8_t[MAC_SIZE];
	mac_to_arr(src_mac_str, src_mac_arr);
	this->src_mac = src_mac_arr;

	uint8_t* dst_mac_arr = new uint8_t[MAC_SIZE];
	mac_to_arr(dst_mac_str, dst_mac_arr);
	this->dst_mac = dst_mac_arr;

	this->cs = std::stoi(cs_str);
}

L2::L2(const L2 &base): L3(base) {
	uint8_t* src_mac_arr = new uint8_t[MAC_SIZE];
	uint8_t* dst_mac_arr = new uint8_t[MAC_SIZE];

	for (int i = 0; i < MAC_SIZE; i++) {
		src_mac_arr[i] = base.src_mac[i];
		dst_mac_arr[i] = base.dst_mac[i];
	}

	this->src_mac = src_mac_arr;
	this->dst_mac = dst_mac_arr;
	this->cs = base.cs;
}

bool L2::validate_packet(open_port_vec open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     uint8_t mac[MAC_SIZE]) {

	return L3::comp_arr(mac, this->dst_mac, MAC_SIZE) &&
			this->cs == this->calc_sum();
}

bool L2::proccess_packet(open_port_vec &open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     memory_dest &dst) {

	if (this->L3::validate_packet(open_ports, ip, mask, this->src_mac)) { 
		return this->L3::proccess_packet(open_ports, ip, mask, dst);
	}

	return false;
	
}

bool L2::as_string(std::string &packet) {
	std::string L3_str = "";
	if(!this->L3::as_string(L3_str)) {
		return false;
	}

	packet = mac_to_str(this->src_mac) + "|" +
			 mac_to_str(this->dst_mac) + "|" +
			 L3_str + "|" +
			 std::to_string(cs); 

	return true;
}

L2::~L2() {
	delete[] this->src_mac;
	delete[] this->dst_mac;
}

unsigned int L2::calc_sum() const {
	unsigned int L3_sum = this->L3::calc_sum();

	unsigned int macs_sum = 0;
	for (int i = 0; i < MAC_SIZE; i++) {
		macs_sum += this->src_mac[i] + this->dst_mac[i];
	}

	unsigned int cs_L3 = L3::get_cs();

	return L3_sum + macs_sum + L4::sum_bytes(cs_L3);
}

std::string L2::mac_to_str(uint8_t mac[]) {
	std::string mac_str = "";

	for (int i = 0; i < MAC_SIZE; i++) {
		uint8_t elem = mac[i];
		for (int digit = 0; digit < HEX_DIG_IN_BYTE; digit++) {

			int d = elem % HEX_BASE;
			if (d > DEC_BASE - 1) {
				mac_str += (d - DEC_BASE + 'a');
			} else {
				mac_str += std::to_string(d);
			}

			elem /= HEX_BASE;
		}

		if (i < MAC_SIZE - 1) {
			mac_str += ':';
		}
	}
	
	return mac_str;
}

void L2::mac_to_arr(std::string mac, uint8_t mac_arr[]) {
	std::string chunk;
    for (int i = 0; i < MAC_SIZE - 1; i++) {

        chunk = extract_between_delimiters(mac, ':', i, i);

        int num = 0;
        for (int j = 0; j < HEX_DIG_IN_BYTE; j++) {
            num *= HEX_BASE;
            num += L4::hex_to_dec(chunk[j]);
        }

        mac_arr[i] = num;
    }

    chunk = extract_between_delimiters(mac, ':', MAC_SIZE - 1);

    int num = 0;
    for (int j = 0; j < HEX_DIG_IN_BYTE; j++) {
        num *= HEX_BASE;
        num += L4::hex_to_dec(chunk[j]);
    }

    mac_arr[MAC_SIZE - 1] = num;

}
