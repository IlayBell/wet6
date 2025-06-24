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
		L3(L4 &base, std::string packet_str);

		L3(const L3 &base);

		bool validate_packet(open_port_vec open_ports,
                             uint8_t ip[IP_V4_SIZE],
                             uint8_t mask,
                             uint8_t mac[MAC_SIZE]);

		bool proccess_packet(open_port_vec &open_ports,
                             uint8_t ip[IP_V4_SIZE],
                             uint8_t mask,
                             memory_dest &dst);

		bool as_string(std::string &packet);

		~L3();

		static void ip_to_arr(std::string ip, uint8_t ip_arr[]);


	protected:
		unsigned int calc_sum() const;
		unsigned int get_cs() const;

		static bool in_local_net(uint8_t ip1[], uint8_t ip2[], uint8_t mask);
		static std::string get_masked_ip(uint8_t ip[], uint8_t mask);
		static std::string ip_to_str(uint8_t ip[]);
		static std::string dec_to_binary(uint8_t num);

		static bool comp_arr(uint8_t arr1[], uint8_t arr2[], int size);

};
#endif