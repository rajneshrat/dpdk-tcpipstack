#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>

#include <stdio.h>

int
ip_in(struct rte_mbuf *mbuf)
{
   struct ipv4_hdr *hdr =  (struct ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, unsigned char *) +
         sizeof(struct ether_hdr));
   switch(hdr->next_proto_id) {
      case IPPROTO_TCP : printf("tcp packet\n");
         tcp_in(mbuf);
         break;
      default: break ;
   } 

}

int
ip_out(struct rte_mbuf *mbuf)
{

}
