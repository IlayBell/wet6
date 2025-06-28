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

/* Different sizes of counting bases */
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

		/**
		* @fn L4
		* @brief Constructor of the class
		* @param packet_str - A string describes the content of the packet.
		*		 with the form: "src_port|dst_port|addrs|L5_data"
		* @return New L4 packet object.
		*/
		L4(const std::string packet_str);

		/**
		* @fn L4
		* @brief Copy Constructor of the class
		* @param base - L4 packet to copy.
		* @return New copy of L4 packet.
		*/
		L4(const L4 &base);

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
		bool validate_packet(open_port_vec open_ports,
                             uint8_t ip[IP_V4_SIZE],
                             uint8_t mask,
                             uint8_t mac[MAC_SIZE]);

		/**
		* @fn packet_proccess
		* @brief Proccess L4 packet as specified.
		* @param open_ports - A vector of all NIC's open ports.
		* @param ip[] - NIC's IP address, represented via an array.
		* @param mask - The mask of the NIC's that determine the local net.
		* @param dst[out] - the place where the packet should be written in.
		* @return True upon success, false otherwise.
		*/
		bool proccess_packet(open_port_vec &open_ports,
                             uint8_t ip[IP_V4_SIZE],
                             uint8_t mask,
                             memory_dest &dst);

		/**
		* @fn as_string
		* @brief Writes packet properties as string:
		*		 "src_port|dst_port|addrs|L5_data"
		* @param packet[out] - The packet as a string.
		* @return True upon success, false otherwise.
		*/
		bool as_string(std::string &packet);
		
		/**
		* @fn ~L4
		* @brief Destructs L4 object, 
		*        deletes all dynamically allocated memory.
		* @return None.
		*/
		~L4();

		/**
		* @fn arr_dec_to_hex
		* @brief converts an array of chars that is consisted of ints only
		         to string in hex base.
		* @param arr[] - array of chars to convert.
		* @param n - length of array.
		* @return the array as a string in hex base.
		*/
		static std::string arr_dec_to_hex(const unsigned char arr[], int n);


	protected:
		
		/**
		* @fn comp_ports
		* @brief checks if given port's src and dst are the same to this.
		* @param port - reference to port to compare src and dst to.
		* @return True if the same, Fase otherwise.
		*/
		bool comp_ports(const open_port& port) const;

		/**
		* @fn calc_sum
		* @brief Sums all bytes of each property of the packet.
		* @return The calculated sum.
		*/
		unsigned int calc_sum() const;

		/**
		* @fn data_to_arr
		* @brief converts the string od data to an array of ints.
				 each byte (two chars) is converted to it's int value.
		* @param data_str - string of data to convert.
		* @param data_arr[] - array to write to.
		* @return NONE.
		*/
		static void data_to_arr(std::string data_str,
								unsigned char data_arr[]);

		/**
		* @fn dec_to_hex
		* @brief converts number in base 10 to number in base 16.
		* @param dec - char containing the number to convert.
		* @return number in base 16 as a string.
		*/
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