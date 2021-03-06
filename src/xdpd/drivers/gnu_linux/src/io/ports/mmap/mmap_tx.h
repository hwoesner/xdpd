/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MMAP_TX_H
#define MMAP_TX_H 

#include <rofl_datapath.h>
#include <string>

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include "../../../util/likely.h"
#include "../../../config.h"
#include <utils/c_logger.h>

/**
* @file mmap_tx.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief MMAP TX internals 
*
*/

namespace xdpd {
namespace gnu_linux {

class eConstructorMmapTx : public std::exception {};

/**
* @brief MMAP TX internals (v2)
*
* @ingroup driver_gnu_linux_io_ports
*/
class mmap_tx{

private:

	void* map;
	int block_size;
	int n_blocks;
	int frame_size;
	
	std::string devname; // device name e.g. "eth0"
	
	int sd; // socket descriptor
	//rofl::caddress ll_addr; // link layer sockaddr
	struct sockaddr_ll ll_addr;
	struct tpacket_req req; // ring buffer

	//Circular buffer pointer
	unsigned int tpos; // current position within ring buffer

public:
	/**
	 *
	 */
	mmap_tx(std::string devname,
		int block_size,
		int n_blocks,
		int frame_size);

	~mmap_tx(void);

	/**
	 *
	 */
	inline struct tpacket2_hdr* get_free_slot(){
		struct tpacket2_hdr *hdr = (struct tpacket2_hdr*)((uint8_t*)map + tpos * req.tp_frame_size);
	
		if ( likely(hdr->tp_status == TP_STATUS_AVAILABLE) ) {
		
			//Increment and return
			tpos++;
			if (tpos == req.tp_frame_nr) {
				tpos = 0;
			}
			
			return hdr;
		}else
			return NULL;
	};

	inline rofl_result_t send(void){
		int rv;
		XDPD_DEBUG_VERBOSE(DRIVER_NAME" %s() on socket descriptor %d\n", __FUNCTION__, sd);

MMAP_SEND_RETRY:
		rv = ::sendto(sd, NULL, 0, MSG_DONTWAIT, NULL, 0);
		if(rv < 0){
			if(likely(errno == EAGAIN)){
				sched_yield(); //Preempt
				goto MMAP_SEND_RETRY;
			}

			XDPD_ERR(DRIVER_NAME"[%s:mmap_tx]: Error in port's sendto(), errno:%d, %s\n", devname.c_str(), errno, strerror(errno));
				return ROFL_FAILURE;	
		}
		return ROFL_SUCCESS;	
	};
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* MMAP_TX_H_ */
