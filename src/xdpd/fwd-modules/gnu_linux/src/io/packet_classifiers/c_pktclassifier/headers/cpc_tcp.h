#ifndef _CPC_TCP_H_
#define _CPC_TCP_H_

#include "../cpc_utils.h"

struct cpc_tcp_hdr_t {
	uint16_t sport;
	uint16_t dport;
	uint32_t seqno;
	uint32_t ackno;
#ifdef __BIG_ENDIAN
	uint16_t offset   : 4; // =5 => 5 32bit words (= 20 bytes), >5 => options appended
	uint16_t reserved : 3;
	uint16_t ns       : 1;
	/* byte */
	uint16_t cwr      : 1;
	uint16_t ece	  : 1;
	uint16_t urg	  : 1;
	uint16_t ack	  : 1;
	uint16_t psh	  : 1;
	uint16_t rst	  : 1;
	uint16_t syn	  : 1;
	uint16_t fin	  : 1;
#elif __LITTLE_ENDIAN
	uint16_t cwr      : 1;
	uint16_t ece	  : 1;
	uint16_t urg	  : 1;
	uint16_t ack	  : 1;
	uint16_t psh	  : 1;
	uint16_t rst	  : 1;
	uint16_t syn	  : 1;
	uint16_t fin	  : 1;
	/* byte */
	uint16_t offset   : 4; // =5 => 5 32bit words (= 20 bytes), >5 => options appended
	uint16_t reserved : 3;
	uint16_t ns       : 1;
#endif
	uint16_t wnd;
	uint16_t checksum;
	uint16_t urgent;
	uint8_t data[0];
} __attribute__((packed));

/* for UDP checksum calculation */
struct cpc_ip_pseudo_hdr_t {
	uint32_t src;
	uint32_t dst;
	uint8_t reserved;
	uint8_t proto;
	uint16_t len;
} __attribute__((packed));

enum tcp_ip_proto_t {
	TCP_IP_PROTO = 6,
};

inline static
void tcp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){
	int wnum;
	uint32_t sum = 0; //sum
	uint16_t* word16;
	
	//Set 0 to checksum
	((cpc_tcp_hdr_t*)hdr)->checksum = 0x0;

	/*
	* part -I- (IPv4 pseudo header)
	*/
	
	word16 = (uint16_t*)(void*)&ip_src; //NOTE ENDIANNESS?
	sum += *(word16+1);
	sum += *(word16);

	word16 = (uint16_t*)(void*)&ip_dst; //NOTE ENDIANNESS??
	sum += *(word16+1);
	sum += *(word16);
	sum += htons(ip_proto);
	
	sum += htons(length); 

	/*
	* part -II- (TCP header + payload)
	*/
	
	// pointer on 16bit words
	// number of 16bit words
	word16 = (uint16_t*)hdr;
	wnum = (length / sizeof(uint16_t));

	for (int i = 0; i < wnum; i++){
		sum += (uint32_t)word16[i];
	}
	
	if(length & 0x1)
		//Last byte
		sum += (uint32_t)( ((uint8_t*)hdr)[length-1]);

	//Fold it
	do{
		sum = (sum & 0xFFFF)+(sum >> 16);
	}while (sum >> 16); 

	((cpc_tcp_hdr_t*)hdr)->checksum =(uint16_t) ~sum;

//	fprintf(stderr," %x \n", tcp_hdr->checksum);
}

inline static
uint16_t get_sport(void *hdr){
	return CPC_BE16TOH(((cpc_tcp_hdr_t*)hdr)->sport);
}

inline static
void set_sport(void *hdr, uint16_t port){
	((cpc_tcp_hdr_t*)hdr)->sport = CPC_HTOBE16(port);
}

inline static
uint16_t get_dport(void *hdr){
	return CPC_BE16TOH(((cpc_tcp_hdr_t*)hdr)->dport);
}

inline static
void set_dport(void *hdr, uint16_t port){
	((cpc_tcp_hdr_t*)hdr)->dport = CPC_HTOBE16(port);
}


#endif //_CPC_TCP_H_