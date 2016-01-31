#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include "logger.h"

#include <stdio.h>
#include "ip.h"

int
ether_in(struct rte_mbuf *mbuf)
{
	struct ether_hdr *eth;

	eth = rte_pktmbuf_mtod(mbuf, struct ether_hdr *);

//   int *data = mbuf->userdata;
   
   switch(ntohs(eth->ether_type)) {
      case ETHER_TYPE_ARP : 
         logger(LOG_ARP, NORMAL, "seen arp packet\n");
         //printf("arp packet\n");
         arp_in(mbuf);
         break;
      case ETHER_TYPE_IPv4 : 
         logger(LOG_IP, NORMAL, "seen ip packet\n");
      //printf("IP packet\n");
         ip_in(mbuf);
         break; 
      default :
			rte_pktmbuf_free(mbuf);
   }
   rte_pktmbuf_free(mbuf); // don't free here, future work.
}

