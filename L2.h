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
		L2(L3 &base, std::string packet_str);

		L2(const L2 &base);

		bool validate_packet(open_port_vec open_ports,
                             uint8_t ip[IP_V4_SIZE],
                             uint8_t mask,
                             uint8_t mac[MAC_SIZE]);

		bool proccess_packet(open_port_vec &open_ports,
                             uint8_t ip[IP_V4_SIZE],
                             uint8_t mask,
                             memory_dest &dst);

		bool as_string(std::string &packet);

		~L2();

		static void mac_to_arr(std::string mac, uint8_t mac_arr[]);

	protected:
		unsigned int calc_sum() const;

		static std::string mac_to_str(uint8_t mac[]);
};
#endif