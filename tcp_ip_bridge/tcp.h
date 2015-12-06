#ifndef __TCP__
#define __TCP__

//#include <rte_common.h>
//#include <rte_ether.h>
//#include <assert.h>
//#include <rte_ip.h>
//#include <rte_tcp.h>

enum TCP_FLAGS {
   CWR = 0x80,
   ECE = 0x40,
   URG = 0x20,
   ACK = 0x10,
   PSH = 0x08,
   RST = 0x04,
   SYN = 0x02,
   FIN = 0x01,
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

#endif
