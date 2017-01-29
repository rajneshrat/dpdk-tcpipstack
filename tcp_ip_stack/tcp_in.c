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
#include "tcp_out.h"
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
   if((ptcb->state == LISTENING) && !(ptcphdr->tcp_flags & TCP_FLAG_SYN)) {
      rte_pktmbuf_free(mbuf);
      send_reset(hdr, ptcphdr);
      logger(LOG_TCP, CRITICAL, "Ignoring non syn flag for listen tcb %u\n", ptcb->identifier);
      return 0;
   }
   if((ptcphdr->tcp_flags & TCP_FLAG_FIN)) {
    //  sendfin(ptcb);  // future, remove it from here.
   }
  
   //printf("tcb identifier = %d\n", ptcb->identifier);
   if(tcpok(ptcb, mbuf)) {
      if(ntohl(ptcphdr->sent_seq) > ptcb->max_seq_received) {
         ptcb->max_seq_received = ntohl(ptcphdr->sent_seq); 
      }
      logger(LOG_TCP, NORMAL, "[RECEIVED TCP PACKET] received tcp packet seq %u ack %u for tcb %u\n", ntohl(ptcphdr->sent_seq), ntohl(ptcphdr->recv_ack), ptcb->identifier);
      // remove the pair from send window and if all are done adjust the rto timer.
      AdjustSendWindow(ptcb, ntohl(ptcphdr->recv_ack));
      tcpswitch[ptcb->state](ptcb, ptcphdr, hdr, mbuf); // this function will take care mbuf, no need to free it now.
         //rte_pktmbuf_free(mbuf) ;// higher layer let decide this
      // need to fix check fro
      //(tcpswitch[1])(ptcb, mbuf);
   }
   else {
        rte_pktmbuf_free(mbuf);
   //   logger(TCP, NORMAL, "sending syn-ack packet\n");
     // sendsynack(ptcb);
   // send ack future work
   }
   return 0;
}
