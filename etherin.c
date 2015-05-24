#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>

#include <stdio.h>
#include "ip.h"

int
ether_in(struct rte_mbuf *mbuf)
{
	struct ether_hdr *eth;

	eth = rte_pktmbuf_mtod(mbuf, struct ether_hdr *);

   printf("in arp %x\n", eth->ether_type);
   switch(ntohs(eth->ether_type)) {
      case ETHER_TYPE_ARP : printf("arp packet\n");
         arp_in(mbuf);
         break;
      case ETHER_TYPE_IPv4 : printf("IP packet\n");
         ip_in(mbuf);
         break; 
      default :
			rte_pktmbuf_free(mbuf);
   }
   rte_pktmbuf_free(mbuf); // don't free here, future work.
}

