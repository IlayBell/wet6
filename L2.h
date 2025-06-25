#ifndef __L2__
#define __L2__

#include <iostream>
#include <string>
#include "common.hpp"
#include "packets.hpp"
#include "L3.h"

/* Number of '|' until reaching CS segmant in L2 */
const int CS_L2_BAR_NUM = 10;

class L2: public L3 {
	uint8_t* src_mac;
	uint8_t* dst_mac;
	unsigned int cs;

	public:
		/**
		* @fn L2
		* @brief Constructor of the class
		* @param base - L3 sub-packet to copy that L2 will extend
		* @param packet_str - A string describes the content of the packet.
		*		 with the form: "src_mac|dst_mac|L3_packet|cs"
		* @return New L2 packet object.
		*/
		L2(L3 &base, std::string packet_str);

		/**
		* @fn L2
		* @brief Copy Constructor of the class
		* @param base - L2 packet to copy.
		* @return New copy of L2 packet.
		*/
		L2(const L2 &base);

		/**
		* @fn validate_packet
		* @brief Checks if L2 packet is valid, meaning cs is valid. 
		* @param open_ports - A vector of all NIC's open ports.
		* @param ip - NIC's IP address, represented via an array.
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
		* @brief Proccess L2 packet as specified.
		* @param open_ports - A vector of all NIC's open ports.
		* @param ip - NIC's IP address, represented via an array.
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
		*		 "src_mac|dst_mac|L3_packet|cs"
		* @param packet[out] - The packet as a string.
		* @return True upon success, false otherwise.
		*/
		bool as_string(std::string &packet);

		/**
		* @fn ~L2
		* @brief Destructs L2 object, 
		*        deletes all dynamically allocated memory.
		* @return None.
		*/
		~L2();

		/**
		* @fn mac_to_arr
		* @brief convert MAC address written as string into an array.
		* @param mac - the mac as string to convert.
		* @param mac_arr[out] - The destination array to write the MAC into.
		* @return void
		*/
		static void mac_to_arr(std::string mac, uint8_t mac_arr[]);

	protected:

		/**
		* @fn calc_sum
		* @brief Sums all bytes of each property of the packet, other then cs.
		* @return The calculated sum.
		*/
		unsigned int calc_sum() const;

		/**
		* @fn mac_to_str
		* @brief converts a MAC described by an array to string in format:
		*		 "**:**:**:**:**:**"
		* @param mac - The MAC as an array that should be written as string.
		* @return The MAC address as a string.
		*/
		static std::string mac_to_str(uint8_t mac[]);
};
#endif