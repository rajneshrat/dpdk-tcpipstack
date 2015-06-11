#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>

#include <stdio.h>
#include "tcp_tcb.h"

tcpinstate *tcpswitch[TCP_STATES] = { 
   tcp_closed,
   tcp_listen,
//   tcp_syn_sent,
//   tcp_syn_rcv,
};

int
tcp_listen(struct tcb *ptcb, struct rte_mbuf* mbuf)
{
   ////printf("tcp_listen called\n");
   struct tcb *new_ptcb = NULL;
   new_ptcb = alloc_tcb(); 
   memcpy(new_ptcb, ptcb, sizeof(struct tcb));
   new_ptcb->state = SYN_RECV;
   new_ptcb->dport = ptcb->dport;
   new_ptcb->ipv4_dst = ptcb->ipv4_dst;
   new_ptcb->ipv4_src = ptcb->ipv4_src;
   // set src port;
   // set ips.
   ptcb->newpTcbOnAccept = new_ptcb;
   pthread_mutex_lock(&(ptcb->mutex));
   pthread_cond_signal(&(ptcb->condAccept));
   pthread_mutex_unlock(&(ptcb->mutex));
   //printf("sending ack\n");
   //sendack(new_ptcb);
   sendsynack(new_ptcb);
   return 0;
}
int
tcp_closed(struct tcb *ptcb, struct rte_mbuf* mbuf)
{
  // //printf("tcp_closed called\n");

}
