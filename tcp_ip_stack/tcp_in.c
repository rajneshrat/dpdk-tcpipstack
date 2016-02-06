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
#include "tcp.h"
#include "tcp_states.h"
#include "main.h"
#include "ip.h"

int tcpchecksumerror;
int tcpnopcb;


int tcpok(struct tcb *ptcb, struct rte_mbuf *mbuf)
{
   ptcb = NULL;
   mbuf = NULL;
   (void ) mbuf;
   (void) ptcb;
   return 1;
}


void send_reset(struct ipv4_hdr *ip_hdr, struct tcp_hdr *t_hdr)
{
   printf("sending reset\n");
   struct rte_mbuf *mbuf = get_mbuf();
   //printf("head room = %d\n", rte_pktmbuf_headroom(mbuf));
  // rte_pktmbuf_adj(mbuf, sizeof(struct tcp_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr));
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
   //printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
      //printf("tcp header is null\n");
   }


   uint8_t tcp_len = 20 ;
   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->data_off = tcp_len;


   ptcphdr->src_port = t_hdr->dst_port;
   ptcphdr->dst_port = t_hdr->src_port;
   ptcphdr->sent_seq = t_hdr->recv_ack;
   ptcphdr->recv_ack = 0;
   ptcphdr->tcp_flags =  RST;
   ptcphdr->rx_win = 12000;
   ptcphdr->tcp_urp = 0; 
   
  // struct ipv4_hdr *hdr = (struct ipv4_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct ipv4_hdr));
   //printf("head room4 = %d\n", rte_pktmbuf_headroom(mbuf));
       //printf("ip header is null\n");
   //    fflush(stdout);
   struct tcb ptcb;
   ptcb.identifier = 0; // dummy 
   
   ptcb.ipv4_dst = ip_hdr->dst_addr;  // fix it future , why we have htonl only for src
   ptcb.ipv4_src = htonl(ip_hdr->src_addr);
   fflush(stdout);
   ip_out(&ptcb, mbuf, ptcphdr, 0); 
}

int tcp_in(struct rte_mbuf *mbuf)
{
   struct tcb *ptcb = NULL;
   logger(LOG_TCP, NORMAL, "received tcp packet\n");
   //calculate tcp checksum.
   if(0) {
      rte_pktmbuf_free(mbuf);
      ++tcpchecksumerror;
      return -1;
   }  
   struct ipv4_hdr *hdr =  (struct ipv4_hdr *)((rte_pktmbuf_mtod(mbuf, unsigned char *) +
                            sizeof(struct ether_hdr)));
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *) ( rte_pktmbuf_mtod(mbuf, unsigned char *) + 
         sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr)); 
   ptcb = findtcb(ptcphdr, hdr);
   if(ptcb == NULL) {
      ++tcpnopcb;
      rte_pktmbuf_free(mbuf);
      logger(LOG_TCP, CRITICAL, "found null tcb\n");
      send_reset(hdr, ptcphdr);
      return -1;
   }
   if((ptcb->state == LISTENING) && !(ptcphdr->tcp_flags & SYN)) {
      rte_pktmbuf_free(mbuf);
      send_reset(hdr, ptcphdr);
      printf("Ignoring non syn flag for listen tcb\n");
      return 0;
   }
   if((ptcphdr->tcp_flags & FIN)) {
    //  sendfin(ptcb);  // future, remove it from here.
   }
  
   printf("tcb identifier = %d\n", ptcb->identifier);
   if(tcpok(ptcb, mbuf)) {
      logger(LOG_TCP, NORMAL, "sending tcp packet\n");
      tcpswitch[ptcb->state](ptcb, ptcphdr, hdr, mbuf);
      //(tcpswitch[1])(ptcb, mbuf);
   }
   else {
   //   logger(TCP, NORMAL, "sending syn-ack packet\n");
     // sendsynack(ptcb);
   // send ack future work
   }
   return 0;
}
