#ifndef __L4__
#define __L4__

#include <iostream>
#include <string>
#include "common.hpp"
#include "packets.hpp"

/* Size of data in L5 packet */
const int DATA_L5_SIZE = 32;
/* Size of byte in bits */
const int SIZE_OF_BYTE = 8;
/* Num of hex digit in one byte */
const int HEX_DIG_IN_BYTE = 2;

enum bases_size {
    HEX_BASE = 16,
    DEC_BASE = 10
};

class L4: public generic_packet {
	unsigned short src_port;
	unsigned short dst_port;
	unsigned int addr;
	unsigned char* data;

	public:
		L4(const std::string packet_str);

		L4(const L4 &base);

		bool validate_packet(open_port_vec open_ports,
                             uint8_t ip[IP_V4_SIZE],
                             uint8_t mask,
                             uint8_t mac[MAC_SIZE]);

		bool proccess_packet(open_port_vec &open_ports,
                             uint8_t ip[IP_V4_SIZE],
                             uint8_t mask,
                             memory_dest &dst);

		bool as_string(std::string &packet);

		~L4();

		static std::string arr_dec_to_hex(const unsigned char arr[], int n);


	protected:
		bool comp_ports(const open_port& port) const;

		unsigned int calc_sum() const;

		static void data_to_arr(std::string data_str,
								unsigned char data_arr[]);

		static std::string dec_to_hex(unsigned char dec);

		/**
	    * @fn sum_bytes
	    * @brief sums the bytes of the number in decimal
	    * @param [in] num - the number to sum its bytes
	    * return the sum
	    */
		static int sum_bytes(unsigned int num);

		/**
		* @fn chr_to_int
		* @brief converts a char represent a digit in hex to int in decimal
		* @param [in] c - the char
		* return the int
		*/
		static uint8_t hex_to_dec(char c);

};
#endif