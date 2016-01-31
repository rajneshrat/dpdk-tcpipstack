#ifndef __ETHERIN__
#define __ETHERIN___
#if 0
typedef enum {
   ETHER_TYPE_ARP = 0x0806,
   ETHER_TYPE_IPV4 = 0x8000,
};
#endif
int
ether_in(struct rte_mbuf *mbuf);
#endif
