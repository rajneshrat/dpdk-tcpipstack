#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include "logger.h"
#include "etherin.h"
#include <stdio.h>
#include "ip.h"
#include "arp.h"
#include "main.h"

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
         free_mbuf(mbuf); // don't free here, future work.
         break;
      case ETHER_TYPE_IPv4 : 
         logger(LOG_IP, NORMAL, "seen ip packet\n");
      //printf("IP packet\n");
         ip_in(mbuf);  // no need to free mbuf here, this will be taken here in this function or its calle.
         break; 
      default :
			free_mbuf(mbuf);
   }
   return 0;
}

