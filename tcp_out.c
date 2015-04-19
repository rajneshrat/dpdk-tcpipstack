#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <rte_tcp.h>

#include <stdio.h>
#include "tcp_in.h"
#include "tcp_tcb.h"
#include "tcp_states.h"

void sendsynack(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   printf("head room = %d\n", rte_pktmbuf_headroom(mbuf));
  // rte_pktmbuf_adj(mbuf, sizeof(struct tcp_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr));
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
   printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
      printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(20);
   ptcphdr->dst_port = htons(208);
   ptcphdr->sent_seq = htonl(10);
   ptcphdr->recv_ack = htonl(200);
   
   printf(" null\n");
   fflush(stdout);
   ip_out(ptcb, mbuf); 
}

