#include "L3.h"

using namespace common;

/**
* @fn L3
* @brief Constructor of the class
* @param base - L4 sub-packet to copy that L3 will extend
* @param packet_str - A string describes the content of the packet.
*		 with the form: "src_ip|dst_ip|ttl|cs|L4_packet"
* @return New L3 packet object.
*/
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

/**
* @fn L3
* @brief Copy Constructor of the class
* @param base - L3 packet to copy.
* @return New copy of L3 packet.
*/
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

/**
* @fn validate_packet
* @brief Checks if L3 packet is valid. Meaning ttl and cs are valid. 
* @param open_ports - A vector of all NIC's open ports.
* @param ip[] - NIC's IP address, represented via an array.
* @param mask - The mask of the NIC's that determine the local net.
* @param mac[] - NIC's MAC address, represented via an array.
* @return True upon success, false otherwise. 
*/
bool L3::validate_packet(open_port_vec open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     uint8_t mac[MAC_SIZE]) {

	return this->ttl > 0 && this->cs == L3::calc_sum();
	
}

/**
* @fn packet_proccess
* @brief Proccess L3 packet as specified.
* @param open_ports - A vector of all NIC's open ports.
* @param ip[] - NIC's IP address, represented via an array.
* @param mask - The mask of the NIC's that determine the local net.
* @param dst[out] - the place where the packet should be written in.
* @return True upon success, false otherwise.
*/
bool L3::proccess_packet(open_port_vec &open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     memory_dest &dst) {

	/* dst ip same as NIC's => L4 (2.4) */
	if (comp_arr(ip, this->dst_ip, IP_V4_SIZE)) {
		
		dst = LOCAL_DRAM;	
		
		bool L4_flag = true;

		if (this->L4::validate_packet(open_ports, ip, mask, nullptr)) {

			L4_flag = this->L4::proccess_packet(open_ports,
												ip,
												mask,
												dst);
		} else {
			L4_flag = false;
		}
		

		return this->ttl > 0 && L4_flag;
	}

	this->ttl--;
	this->cs = calc_sum();

	/* Packet invalid */
	if (ttl == 0) {
		return false;
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

/**
* @fn as_string
* @brief Writes packet properties as string:
*		 "src_ip|dst_ip|ttl|cs|L4_packet"
* @param packet[out] - The packet as a string.
* @return True upon success, false otherwise.
*/
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

/**
* @fn ~L3
* @brief Destructs L3 object, 
*        deletes all dynamically allocated memory.
* @return None.
*/
L3::~L3() {
	delete[] this->src_ip;
	delete[] this->dst_ip;
}


/**
* @fn calc_sum
* @brief Sums all bytes of each property of the packet, other then cs.
* @return The calculated sum.
*/
unsigned int L3::calc_sum() const {
	unsigned int L4_sum = this->L4::calc_sum();

	unsigned int ips_sum = 0;
	for (int i = 0; i < IP_V4_SIZE; i++) {
		ips_sum += this->src_ip[i] + this->dst_ip[i];
	}

	unsigned int ttl_sum = L4::sum_bytes(this->ttl);

	return L4_sum + ips_sum + ttl_sum;
}

/**
* @fn in_local_net
* @brief checks whether two ip adresses are in the same local net.
* @param ip1[] - The 1st ip address as an array.
* @param ip2[] - The 2nd ip address as an array.
* @param mask - The mask that determine the local net - 
*		 the local net is defined to be the first 'mask' bits of ip.
* @return True, if both in the same local net, false otherwise.
*/
bool L3::in_local_net(uint8_t ip1[], uint8_t ip2[], uint8_t mask) {

	return get_masked_ip(ip1, mask) == get_masked_ip(ip2, mask);
}

/**
* @fn get_masked_ip
* @brief returns a string with 32 chars, the first 'mask' bits are
*		 the IP address in binary, the rest are 0.
* @param ip[] - The IP that should be masked.
* @param mask - The number of bits the should be regarded.
* @return A string with 32 chars that describes the masked IP.
*/
std::string L3::get_masked_ip(uint8_t ip[], uint8_t mask) {
	std::string ip_str = "";

	for (int i = 0; i < IP_V4_SIZE; i++) {
		std::string bin_entry = dec_to_binary(ip[i]);
		ip_str += bin_entry;
	}

	std::string ip_masked = ip_str.substr(0, mask);

	return ip_masked;

}

/**
* @fn ip_to_str
* @brief converts an IP described by an array to string in format:
*		 "***.***.***.***"
* @param ip[] - The IP as an array, that should be written as string.
* @return The IP address as a string.
*/
std::string L3::ip_to_str(uint8_t ip[]) {
	std::string str = "";
	for (int i = 0; i < IP_V4_SIZE - 1; i++) {
		str += std::to_string(ip[i]) + ".";
	}

	str += std::to_string(ip[IP_V4_SIZE - 1]);

	return str;
}

/**
* @fn ip_to_arr
* @brief convert IP address written as string into an array.
* @param ip - the IP as string to convert.
* @param ip_arr[] [out] - The destination array to write the IP into.
* @return void
*/
void L3::ip_to_arr(std::string ip, uint8_t ip_arr[]){ 
	std::string chunk;
    for (int i = 0; i < IP_V4_SIZE - 1; i++) {
        chunk = extract_between_delimiters(ip, '.', i, i);

        ip_arr[i] = std::stoi(chunk);
    }

    chunk = extract_between_delimiters(ip, '.', IP_V4_SIZE - 1);
    ip_arr[IP_V4_SIZE - 1] = std::stoi(chunk);
}  

/**
* @fn dec_to_binary
* @brief converts number in decimal into its binary respresenation.
* @param num - A num in decimal base.
* @return A string that contains the number in binary.
*/
std::string L3::dec_to_binary(uint8_t num) {
	std::string bin_num = "00000000";

	for (int i = SIZE_OF_BYTE - 1; i >= 0; i--) {

		bin_num[i] = ((num % 2 == 1) ? '1' : '0');

		num /= 2;
	}

	return bin_num;
}

/**
* @fn get_cs
* @brief A getter to the L3 cs property of the packet.
* @return The cs value.
*/
unsigned int L3::get_cs() const {
	return this->cs;
}

/**
* @fn comp_arr
* @brief compare between two arrays of the same size.
* @param arr1[] - The 1st array.
* @param arr2[] - The 2nd array.
* @param size - The size of both arrays.
* @return True if identical, false otherwise.
*/
bool L3::comp_arr(uint8_t arr1[], uint8_t arr2[], int size){
    for (int i = 0; i < size; i++) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }

    return true;
}
