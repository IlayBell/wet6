#include "L4.h"
#include <algorithm>
#include <string>

using namespace common;

/**
* @fn L4
* @brief Constructor of the class
* @param packet_str - A string describes the content of the packet.
*		 with the form: "src_port|dst_port|addrs|L5_data"
* @return New L4 packet object.
*/
L4::L4(const std::string packet_str) {
	int bar_cnt = 0;

	std::string src_prt_str = extract_between_delimiters(packet_str,
														'|',
														bar_cnt,
														bar_cnt);

	bar_cnt++;

	std::string dst_prt_str = extract_between_delimiters(packet_str,
														'|',
														bar_cnt,
														bar_cnt);
	bar_cnt++;

	std::string addr_str = extract_between_delimiters(packet_str,
													 '|',
													 bar_cnt,
													 bar_cnt);
	bar_cnt++;

	std::string data_str = extract_between_delimiters(packet_str,
												 '|',
												 bar_cnt);



	this->src_port = std::stoi(src_prt_str);
	this->dst_port = std::stoi(dst_prt_str);
	this->addr = std::stoi(addr_str);
	this->data = new unsigned char[DATA_L5_SIZE];
	data_to_arr(data_str, this->data);
}

/**
* @fn L4
* @brief Copy Constructor of the class
* @param base - L4 packet to copy.
* @return New copy of L4 packet.
*/
L4::L4(const L4 &base) {
	this->src_port = base.src_port;
	this->dst_port = base.dst_port;
	this->addr = base.addr;

	this->data = new unsigned char[DATA_L5_SIZE];
	for(int i = 0; i < DATA_L5_SIZE; i++) {
		this->data[i] = base.data[i];
	}
}

/**
* @fn validate_packet
* @brief Checks if L4 packet is valid.
		 Meaning src_port and dst_port are in NIC.
* @param open_ports - A vector of all NIC's open ports.
* @param ip[] - NIC's IP address, represented via an array.
* @param mask - The mask of the NIC's that determine the local net.
* @param mac - NIC's MAC address, represented via an array.
* @return True upon success, false otherwise. 
*/
bool L4::validate_packet(open_port_vec open_ports,
                        uint8_t ip[IP_V4_SIZE],
                        uint8_t mask,
                        uint8_t mac[MAC_SIZE]) {

	auto port_iter = std::find_if(open_ports.begin(), open_ports.end(), 
								 	[this](const open_port& port) {
								return this->comp_ports(port);
							});

	return (port_iter != open_ports.end() &&
			this->addr <= DATA_ARR_SIZE - DATA_L5_SIZE);
}

/**
* @fn packet_proccess
* @brief Proccess L4 packet as specified.
* @param open_ports - A vector of all NIC's open ports.
* @param ip[] - NIC's IP address, represented via an array.
* @param mask - The mask of the NIC's that determine the local net.
* @param dst[out] - the place where the packet should be written in.
* @return True upon success, false otherwise.
*/
bool L4::proccess_packet(open_port_vec &open_ports,
                     uint8_t ip[IP_V4_SIZE],
                     uint8_t mask,
                     memory_dest &dst) {

	// check if the src_port and dst_port are in NIC.
	auto port_iter = std::find_if(open_ports.begin(), open_ports.end(), 
							[this](const open_port& port) {
								return this->comp_ports(port);
							});

	open_port& port = *port_iter;
	
	// write data in open_port starting from addr.
	for (int i = 0; i < DATA_L5_SIZE; i++) {
		port.data[this->addr + i] = this->data[i];
	}

	return true;
}


/**
* @fn as_string
* @brief Writes packet properties as string:
		"src_port|dst_port|addrs|L5_data"
* @param packet[out] - The packet as a string.
* @return True upon success, false otherwise.
*/
bool L4::as_string(std::string &packet) {
	std::string data_str = arr_dec_to_hex(this->data, DATA_L5_SIZE);

	packet = std::to_string(this->src_port) + "|" + 
			 std::to_string(this->dst_port) + "|" + 
			 std::to_string(this->addr) + "|" + 
			 data_str;

	return true;
}

/**
* @fn ~L4
* @brief Destructs L4 object, 
* deletes all dynamically allocated memory.
* @return None.
*/
L4::~L4() {
	delete[] this->data;
}

/**
* @fn comp_ports
* @brief checks if given port's src and dst are the same to this.
* @param port - reference to port to compare src and dst to.
* @return True if the same, Fase otherwise.
*/
bool L4::comp_ports(const open_port& port) const {
	return (port.src_prt == this->src_port &&
			port.dst_prt == this->dst_port);
}

/**
* @fn calc_sum
* @brief Sums all bytes of each property of the packet.
* @return The calculated sum.
*/
unsigned int L4::calc_sum() const {
	unsigned int src_prt_sum = sum_bytes(this->src_port);
	unsigned int dst_prt_sum = sum_bytes(this->dst_port);
	unsigned int addr_sum = sum_bytes(this->addr);

	unsigned int data_sum = 0;
	for (int i = 0; i < DATA_L5_SIZE; i++) {
		data_sum += this->data[i];
	}

	return src_prt_sum + dst_prt_sum + addr_sum + data_sum;
}

/**
* @fn data_to_arr
* @brief converts the string od data to an array of ints.
		 each byte (two chars) is converted to it's int value.
* @param data_str - string of data to convert.
* @param data_arr[] - array to write to.
* @return NONE.
*/
void L4::data_to_arr(std::string data_str, unsigned char data_arr[]) {
	for (int i = 0; i < DATA_L5_SIZE; i++) {
		// get next byte written in base 16
		int end_idx = (i == DATA_L5_SIZE - 1? -1 : i);
		std::string chunk = extract_between_delimiters(data_str,
														' ',
														i,
														end_idx);
		// convert to int
		unsigned char dec_num = 0;

		for (int dig = 0; dig < HEX_DIG_IN_BYTE; dig++) {
			dec_num *= HEX_BASE;
			dec_num += hex_to_dec(chunk[dig]);
		}

		data_arr[i] = dec_num;
	}
}

/**
* @fn arr_dec_to_hex
* @brief converts an array of chars that is consisted of ints only
         to string in hex base.
* @param arr[] - array of chars to convert.
* @param n - length of array.
* @return the array as a string in hex base.
*/
std::string L4::arr_dec_to_hex(const unsigned char arr[], int n){
    std::string hex = "";

    for (int i = 0; i < n - 1; i++) {
        hex += dec_to_hex(arr[i]) + " ";
    }
    hex += dec_to_hex(arr[n - 1]);

    return hex;
}

/**
* @fn dec_to_hex
* @brief converts number in base 10 to number in base 16.
* @param dec - char containing the number to convert.
* @return number in base 16 as a string.
*/
std::string L4::dec_to_hex(unsigned char dec) {

	int dec_int = static_cast<int>(dec);

	int msb = dec_int / 16;
	int lsb = dec_int % 16;

	char msb_chr = (msb < 10 ? msb + '0' : (msb - DEC_BASE) + 'a'); 
	char lsb_chr = (lsb < 10 ? lsb + '0' : (lsb - DEC_BASE) + 'a'); 

	std::string hex_str = "";
	hex_str += msb_chr;
	hex_str += lsb_chr;

	return hex_str;
}

/**
* @fn sum_bytes
* @brief sums the bytes of the number in decimal
* @param [in] num - the number to sum its bytes
* return the sum
*/
int L4::sum_bytes(unsigned int num) {
    int sum = 0;
    while (num > 0) {
        sum += num & 0xFF; 
        num >>= SIZE_OF_BYTE;         
    }
    return sum;
}

/**
* @fn chr_to_int
* @brief converts a char represent a digit in hex to int in decimal
* @param [in] c - the char
* return the int
*/
uint8_t L4::hex_to_dec(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } 
    return c - 'a' + DEC_BASE;
} 
