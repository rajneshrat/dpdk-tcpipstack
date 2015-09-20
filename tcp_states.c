#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>

#include <stdio.h>
#include "tcp_tcb.h"
#include "tcp.h"

tcpinstate *tcpswitch[TCP_STATES] = { 
   tcp_closed,
   tcp_listen,
//   tcp_syn_sent,
   tcp_syn_rcv,
};

int
tcp_syn_rcv(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr)
{
   printf("tcp syn recv state\n");
   if((ptcphdr->tcp_flags & SYN) || (ptcphdr->tcp_flags & FIN) ) {
      ptcb->ack = ntohl(ptcphdr->sent_seq) + 1;  // this should add tcp len also.
   }
// also increase ack for data. future work.
   return 0;
}

int
tcp_listen(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr)
{
   struct tcb *new_ptcb = NULL;
   new_ptcb = alloc_tcb(); 
   if(new_ptcb == NULL) {
      printf("Null tcb'\n");
      return 0;
   }
   printf("Tcp listen state\n");
   memcpy(new_ptcb, ptcb, sizeof(struct tcb));
   new_ptcb->identifier = 10;
   new_ptcb->state = SYN_RECV;
   new_ptcb->dport = ptcb->dport;
   new_ptcb->sport = htons(ptcphdr->src_port);
   new_ptcb->ipv4_dst = ptcb->ipv4_dst;
   new_ptcb->ipv4_src = ntohl(iphdr->src_addr);
//   ptcb->sport = ntohs(ptcphdr->src_port);
   new_ptcb->ack = ntohl(ptcphdr->sent_seq) + 1;
   new_ptcb->next_seq = 1;
   // set src port;
   // set ips.
   ptcb->newpTcbOnAccept = new_ptcb;
   pthread_mutex_lock(&(ptcb->mutex));
printf("signaling accept mutex.\n");
   pthread_cond_signal(&(ptcb->condAccept));
   pthread_mutex_unlock(&(ptcb->mutex));
   //printf("sending ack\n");
   //sendack(new_ptcb);
   sendsynack(new_ptcb);
   //sendsynack(ptcb);
   return 0;
}
int
tcp_closed(struct tcb *ptcb, struct tcp_hdr* mbuf, struct ipv4_hdr *iphdr)
{
  // //printf("tcp_closed called\n");

}
