#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include "logger.h"

#include <stdio.h>
#include "tcp_in.h"
#include "tcp_tcb.h"
#include "tcp_states.h"

int tcpchecksumerror;
int tcpnopcb;


int tcpok(struct tcb *ptcb, struct rte_mbuf *mbuf)
{
   return 1;
}

void sendack(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   //printf("head room = %d\n", rte_pktmbuf_headroom(mbuf));
  // rte_pktmbuf_adj(mbuf, sizeof(struct tcp_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr));
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
   //printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
      //printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(20);
   ptcphdr->dst_port = htons(208);
   ptcphdr->sent_seq = htonl(10);
   ptcphdr->recv_ack = htonl(200);
   
   //printf(" null\n");
   fflush(stdout);
   ip_out(ptcb, mbuf); 
}

int tcp_in(struct rte_mbuf *mbuf)
{
   struct tcb *ptcb = NULL;
   logger(TCP, NORMAL, "received tcp packet\n");
   //calculate tcp checksum.
   if(0) {
      rte_pktmbuf_free(mbuf);
      ++tcpchecksumerror;
      return -1;
   }  
   struct ipv4_hdr *hdr =  (struct ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, unsigned char *) +
                            sizeof(struct ether_hdr));
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *) ( rte_pktmbuf_mtod(mbuf, unsigned char *) + 
         sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr)); 
   ptcb = findtcb(ptcphdr);
   if(ptcb == NULL) {
      ++tcpnopcb;
      rte_pktmbuf_free(mbuf);
      logger(TCP, CRITICAL, "found null tcb\n");
      return -1;
   }
   ptcb->ipv4_src = ntohl(hdr->src_addr);
   ptcb->sport = ntohs(ptcphdr->src_port);
   printf("sport = %d\n", ptcb->sport);
   ptcb->ack = ntohl(ptcphdr->sent_seq) + 1;
   if(tcpok(ptcb, mbuf)) {
      logger(TCP, NORMAL, "sending tcp packet\n");
      tcpswitch[ptcb->state](ptcb, ptcphdr);
      //(tcpswitch[1])(ptcb, mbuf);
   }
   else {
      logger(TCP, NORMAL, "sending syn-ack packet\n");
      sendsynack(ptcb);
   }
   return 0;
}
