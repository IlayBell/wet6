#include "L2.h"

using namespace common;

/**
* @fn L2
* @brief Constructor of the class
* @param base - L3 sub-packet to copy that L2 will extend
* @param packet_str - A string describes the content of the packet.
*		 with the form: "src_mac|dst_mac|L3_packet|cs"
* @return New L2 packet object.
*/
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

/**
* @fn L2
* @brief Copy Constructor of the class
* @param base - L2 packet to copy.
* @return New copy of L2 packet.
*/
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

/**
* @fn validate_packet
* @brief Checks if L2 packet is valid, meaning cs is valid. 
* @param open_ports - A vector of all NIC's open ports.
* @param ip - NIC's IP address, represented via an array.
* @param mask - The mask of the NIC's that determine the local net.
* @param mac - NIC's MAC address, represented via an array.
* @return True upon success, false otherwise. 
*/
bool L2::validate_packet(open_port_vec open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     uint8_t mac[MAC_SIZE]) {

	return L3::comp_arr(mac, this->dst_mac, MAC_SIZE) &&
			this->cs == this->calc_sum();
}

/**
* @fn packet_proccess
* @brief Proccess L2 packet as specified.
* @param open_ports - A vector of all NIC's open ports.
* @param ip - NIC's IP address, represented via an array.
* @param mask - The mask of the NIC's that determine the local net.
* @param dst[out] - the place where the packet should be written in.
* @return True upon success, false otherwise.
*/
bool L2::proccess_packet(open_port_vec &open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     memory_dest &dst) {

	if (this->L3::validate_packet(open_ports, ip, mask, nullptr)) { 
		return this->L3::proccess_packet(open_ports, ip, mask, dst);
	}

	return false;
	
}

/**
* @fn as_string
* @brief Writes packet properties as string:
*		 "src_mac|dst_mac|L3_packet|cs"
* @param packet[out] - The packet as a string.
* @return True upon success, false otherwise.
*/
bool L2::as_string(std::string &packet) {
	std::string L3_str = "";
	if(!this->L3::as_string(L3_str)) {
		return false;
	}

	packet = L3_str;
	
	return true;
}


/**
* @fn ~L2
* @brief Destructs L2 object, 
*        deletes all dynamically allocated memory.
* @return None.
*/
L2::~L2() {
	delete[] this->src_mac;
	delete[] this->dst_mac;
}

/**
* @fn calc_sum
* @brief Sums all bytes of each property of the packet, other then cs.
* @return The calculated sum.
*/
unsigned int L2::calc_sum() const {
	unsigned int L3_sum = this->L3::calc_sum();

	unsigned int macs_sum = 0;
	for (int i = 0; i < MAC_SIZE; i++) {
		macs_sum += this->src_mac[i] + this->dst_mac[i];
	}

	unsigned int cs_L3 = L3::get_cs();

	return L3_sum + macs_sum + L4::sum_bytes(cs_L3);
}

/**
* @fn mac_to_str
* @brief converts a MAC described by an array to string in format:
*		 "**:**:**:**:**:**"
* @param mac - The MAC as an array that should be written as string.
* @return The MAC address as a string.
*/
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

/**
* @fn mac_to_arr
* @brief convert MAC address written as string into an array.
* @param mac - the mac as string to convert.
* @param mac_arr[out] - The destination array to write the MAC into.
* @return void
*/
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
