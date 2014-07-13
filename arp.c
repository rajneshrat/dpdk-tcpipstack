#include <rte_common.h>
#include <rte_mbuf.h>
#include <assert.h>
#include <rte_ether.h>

#include <stdio.h>

#include "arp.h"

#define ARP_TABLE_SIZE 100

struct arp arp_table[ARP_TABLE_SIZE];

void
swapvalue(char *add1, char *add2, int len)
{
   char temp;
   int i;
   for(i=0;i<len;i++) {
      temp = add1[i];
      add1[i] = add2[i];
      add2[i] = temp;
   }
}

int
arp_reply_queue(struct rte_mbuf *mbuf)
{
   struct arp *arp_pkt;
   arp_pkt  = rte_pktmbuf_mtod(mbuf, char *) + sizeof(struct ether_hdr);
   swapvalue(arp_pkt->src_hw_add, arp_pkt->dst_hw_add, 8);
   swapvalue(arp_pkt->src_pr_add, arp_pkt->dst_pr_add, 8);
   arp_pkt->opcode = htons(2);
}

int
arp_in (struct rte_mbuf *mbuf) 
{
   assert(mbuf->buf_len >= sizeof(struct arp));
   struct arp *arp_pkt;

	struct ether_hdr *eth;

	eth = rte_pktmbuf_mtod(mbuf, struct ether_hdr *);


   assert(rte_pktmbuf_data_len(mbuf) >= (sizeof(struct arp) + sizeof(struct ether_hdr)));
   arp_pkt  = rte_pktmbuf_mtod(mbuf, char *) + sizeof(struct ether_hdr);
   printf("in arp 2 %x\n",  ntohs(arp_pkt->opcode));
   switch(ntohs(arp_pkt->opcode)) {
      case ARP_REQ ://
         arp_reply_queue(mbuf);
         printf("seen arp packet\n");
         break;
   //   default : assert(0);
   }
}
