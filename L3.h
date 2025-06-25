#ifndef __L3__
#define __L3__

#include <iostream>
#include <string>
#include "common.hpp"
#include "packets.hpp"
#include "L4.h"

/* Max num of dec digit in one ip entry */
const int MAX_IP_SIZE = 3;


class L3: public L4 {
	uint8_t* src_ip;
	uint8_t* dst_ip;
	unsigned int ttl;
	unsigned int cs;

	public:

		/**
		* @fn L3
		* @brief Constructor of the class
		* @param base - L4 sub-packet to copy that L3 will extend
		* @param packet_str - A string describes the content of the packet.
		*		 with the form: "src_ip|dst_ip|ttl|cs|L4_packet"
		* @return New L3 packet object.
		*/
		L3(L4 &base, std::string packet_str);

		/**
		* @fn L3
		* @brief Copy Constructor of the class
		* @param base - L3 packet to copy.
		* @return New copy of L3 packet.
		*/
		L3(const L3 &base);

		/**
		* @fn validate_packet
		* @brief Checks if L3 packet is valid. Meaning ttl and cs are valid. 
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
		* @brief Proccess L3 packet as specified.
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
		*		 "src_ip|dst_ip|ttl|cs|L4_packet"
		* @param packet[out] - The packet as a string.
		* @return True upon success, false otherwise.
		*/
		bool as_string(std::string &packet);

		/**
		* @fn ~L3
		* @brief Destructs L3 object, 
		*        deletes all dynamically allocated memory.
		* @return None.
		*/
		~L3();


		/**
		* @fn ip_to_arr
		* @brief convert IP address written as string into an array.
		* @param ip - the ip as string to convert.
		* @param ip_arr[] [out] - The destination array to write the ip into.
		* @return void
		*/
		static void ip_to_arr(std::string ip, uint8_t ip_arr[]);


	protected:
		/**
		* @fn calc_sum
		* @brief Sums all bytes of each property of the packet, other then cs.
		* @return The calculated sum.
		*/
		unsigned int calc_sum() const;

		/**
		* @fn get_cs
		* @brief A getter to the L3 cs property of the packet.
		* @return The cs value.
		*/
		unsigned int get_cs() const;

		/**
		* @fn in_local_net
		* @brief checks whether two ip adresses are in the same local net.
		* @param ip1[] - The 1st ip address as an array.
		* @param ip2[] - The 2nd ip address as an array.
		* @param mask - The mask that determine the local net - 
		*		 the local net is defined to be the first 'mask' bits of ip.
		* @return True, if both in the same local net, false otherwise.
		*/
		static bool in_local_net(uint8_t ip1[], uint8_t ip2[], uint8_t mask);

		/**
		* @fn get_masked_ip
		* @brief returns a string with 32 chars, the first 'mask' bits are
		*		 the IP address in binary, the rest are 0.
		* @param ip - The IP that should be masked.
		* @param mask - The number of bits the should be regarded.
		* @return A string with 32 chars that describes the masked IP.
		*/
		static std::string get_masked_ip(uint8_t ip[], uint8_t mask);

		/**
		* @fn ip_to_str
		* @brief converts an IP described by an array to string in format:
		*		 "***.***.***.***"
		* @param ip[] - The IP as an array that should be written as string.
		* @return The IP address as a string.
		*/
		static std::string ip_to_str(uint8_t ip[]);

		/**
		* @fn dec_to_binary
		* @brief converts number in decimal into its binary respresenation.
		* @param num - A num in decimal base.
		* @return A string that contains the number in binary.
		*/
		static std::string dec_to_binary(uint8_t num);

		/**
		* @fn comp_arr
		* @brief compare between two arrays of the same size.
		* @param arr1[] - The 1st array.
		* @param arr2[] - The 2nd array.
		* @param size - The size of both arrays.
		* @return True if identical, false otherwise.
		*/
		static bool comp_arr(uint8_t arr1[], uint8_t arr2[], int size);

};
#endif