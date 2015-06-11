#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <rte_tcp.h>

#include <stdio.h>
#include "tcp_in.h"
#include "tcp_tcb.h"
#include "tcp.h"
#include "tcp_states.h"
#include "logger.h"

void sendtcppacket(struct tcb *ptcb, struct rte_mbuf *mbuf)
{
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(ptcb->dport);
   ptcphdr->dst_port = htons(ptcb->sport);
   ptcphdr->sent_seq = htonl(10);
   ptcphdr->recv_ack = htonl(ptcb->ack);
   ptcphdr->data_off = 0x50;
   ptcphdr->tcp_flags =  SYN | ACK;
   ptcphdr->rx_win = 12000;
   ptcphdr->cksum = 0x0001;
   ptcphdr->tcp_urp = 0; 
   //mbuf->ol_flags |=  PKT_TX_IP_CKSUM; // someday will caluclate checkum here only.
   
 //  printf(" null\n");
  // fflush(stdout);
   
   logger(TCP, NORMAL, "sending tcp packet\n");
   ip_out(ptcb, mbuf); 
}

void sendsynack(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   sendtcppacket(ptcb, mbuf);
   //printf("head room = %d\n", rte_pktmbuf_headroom(mbuf));
  // rte_pktmbuf_adj(mbuf, sizeof(struct tcp_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr));
}

