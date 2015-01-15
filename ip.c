#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <inttypes.h>
#include <stdio.h>
#include <rte_tcp.h>
#include "tcp_tcb.h"

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
ip_out(struct tcb *ptcb, struct rte_mbuf *mbuf)
{
   printf("head room3 = %d\n", rte_pktmbuf_headroom(mbuf));
   struct ipv4_hdr *hdr = (struct ipv4_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct ipv4_hdr)); 
   printf("head room4 = %d\n", rte_pktmbuf_headroom(mbuf));
   static uint32_t count = 0;
   if(hdr == NULL) {
      printf("ip header is null\n");
      fflush(stdout);
   }
   hdr->dst_addr = 0x33333333;//ptcb->ipv4_dst;
   hdr->src_addr = 0x44444444;//ptcb->ipv4_src;
   hdr->version_ihl = 4 << 4 | 5; 
   hdr->next_proto_id = IPPROTO_TCP;
   hdr->hdr_checksum = 0;
   hdr->time_to_live = 127;
   hdr->total_length = htons(sizeof(struct ipv4_hdr) + sizeof(struct tcp_hdr));
   hdr->packet_id = count++;
   ether_out(NULL, NULL, mbuf);
}
