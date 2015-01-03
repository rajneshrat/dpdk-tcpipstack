#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>

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

void sendack(struct tcb *ptcb, struct rte_mbuf *mbuf)
{

}

int tcp_in(struct rte_mbuf *mbuf)
{
   struct tcb *ptcb = NULL;
   //calculate tcp checksum.
   if(0) {
      rte_pktmbuf_free(mbuf);
      ++tcpchecksumerror;
      return -1;
   }  
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *) ( rte_pktmbuf_mtod(mbuf, unsigned char *) + 
         sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr)); 
   ptcb = findtcb(ptcphdr);
   if(ptcb == NULL) {
      ++tcpnopcb;
      rte_pktmbuf_free(mbuf);
      return -1;
   }
   if(tcpok(ptcb, mbuf)) {
      tcpswitch[ptcb->state](ptcb, mbuf);
      //(tcpswitch[1])(ptcb, mbuf);
   }
   else {
      sendack(ptcb, mbuf);
   }
   return 0;
}
