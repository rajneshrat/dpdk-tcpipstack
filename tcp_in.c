#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>

#include <stdio.h>
#include "tcp_in.h"
#include "tcp_tcb.h"
#define TCP_STATES 1

struct tcb* findtcb(struct rte_mbuf *mbuf)
{

}

typedef int (tcpinstate)(struct tcb *, struct rte_mbuf*);

tcpinstate *tcpswitch[TCP_STATES] = { 



};

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
   ptcb = findtcb(mbuf);
   if(ptcb = NULL) {
      ++tcpnopcb;
      rte_pktmbuf_free(mbuf);
      return -1;
   }
   if(tcpok(ptcb, mbuf)) {
      tcpswitch[ptcb->tcp_state](ptcb, mbuf);
   }
   else {
      sendack(ptcb, mbuf);
   }
   return 0;
}
