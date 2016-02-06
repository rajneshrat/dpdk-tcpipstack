#ifndef __TCP__
#define __TCP__

//#include <rte_common.h>
//#include <rte_ether.h>
//#include <assert.h>
//#include <rte_ip.h>
//#include <rte_tcp.h>
#include "tcp_tcb.h"


enum TCP_FLAGS {
   TCP_FLAG_CWR = 0x80,
   TCP_FLAG_ECE = 0x40,
   TCP_FLAG_URG = 0x20,
   TCP_FLAG_ACK = 0x10,
   TCP_FLAG_PSH = 0x08,
   TCP_FLAG_RST = 0x04,
   TCP_FLAG_SYN = 0x02,
   TCP_FLAG_FIN = 0x01,
}; 

struct pseudo_tcp_hdr {
   uint32_t src_ip;
   uint32_t dst_ip;
   uint8_t reserved;
   uint8_t protocol;
   uint16_t len;
} __attribute__((__packed__));

struct tcp_option {
   uint8_t type;
   uint8_t len;
   char *data;
};

struct tcp_mss_option {
   uint8_t type;
   uint8_t len;
   uint16_t value;
} __attribute__((__packed__));

struct tcp_winscale_option {
   uint8_t type;
   uint8_t len;
   uint8_t value;
} __attribute__((__packed__));

struct tcp_timestamp_option {
   uint8_t type;
   uint8_t len;
   uint32_t value;
   uint32_t echo;
} __attribute__((__packed__));

//void sendack(struct tcb *ptcb);
void sendack(struct tcb *);

void sendsyn(struct tcb *ptcb);

#endif
