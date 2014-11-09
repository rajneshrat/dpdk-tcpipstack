#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <rte_tcp.h>

#include <stdio.h>
#include "tcp_tcb.h"

#define TOTAL_TCBS 100

int Ntcb = 0;


struct tcb tcbs[TOTAL_TCBS];
struct tcb* findptcb(struct rte_mbuf *mbuf)
{
   int i;
   struct tcb *ptcb = NULL;
   struct ipv4_hdr *ip_hdr =  (struct ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, unsigned char *) +
         sizeof(struct ether_hdr));
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *) (ip_hdr + sizeof(struct ipv4_hdr)); 
   for(i=0; i<Ntcb; i++) {
      ptcb = &tcbs[i];
      if(ptcb->dport == ptcphdr->dst_port) {
      
      }
   }
   return ptcb;
}

