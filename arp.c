#include <rte_common.h>
#include <rte_mbuf.h>
#include <assert.h>
#include <rte_ether.h>

#include <stdio.h>
#include <string.h>

#include "arp.h"

#define ARP_TABLE_SIZE 100

struct arp arp_table[ARP_TABLE_SIZE];

static struct arp_map *arp_map_list = NULL;


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
arp_reply_queue(unsigned char *src_pr_add, unsigned char *dst_pr_add)
{
#if 0
   struct arp *arp_pkt;
   arp_pkt  = rte_pktmbuf_mtod(mbuf, char *) + sizeof(struct ether_hdr);
   swapvalue(arp_pkt->src_hw_add, arp_pkt->dst_hw_add, 8);
   swapvalue(arp_pkt->src_pr_add, arp_pkt->dst_pr_add, 8);
   arp_pkt->opcode = htons(2);
#endif
   struct rte_mbuf *new_mbuf = get_mbuf();
   struct arp *arp_reply = (struct arp *)rte_pktmbuf_prepend (new_mbuf, sizeof(struct arp));

   char mac[6];   
//   memcpy(arp_reply, arp_pkt, sizeof(struct arp));
// http://www.tcpipguide.com/free/t_ARPMessageFormat.htm
   arp_reply->hw_type = htons(HW_TYPE_ETHERNET); 
   arp_reply->pr_type = htons(SW_TYPE_IPV4);  
   arp_reply->hw_len = HW_LEN_ETHER;  
   arp_reply->pr_len = PR_LEN_IPV4;  
   arp_reply->opcode = htons(2);
   unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // don't use broadcast address for arp reply.
   int status = get_mac(src_pr_add, mac);
   printf("arp reply status = %d\n", status);
   memcpy(arp_reply->src_hw_add, mac, HW_LEN_ETHER); 
   memcpy(arp_reply->dst_hw_add, dest_mac, HW_LEN_ETHER); 
   memcpy(arp_reply->src_pr_add, src_pr_add, PR_LEN_IPV4); 
   memcpy(arp_reply->dst_pr_add, dst_pr_add, PR_LEN_IPV4); 
   ether_out(dest_mac, NULL, ETHER_TYPE_ARP, new_mbuf);
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
         arp_reply_queue(arp_pkt->dst_pr_add, arp_pkt->src_pr_add );
         printf("seen arp packet\n");
         break;
   //   default : assert(0);
   }
}

int
get_mac(uint32_t ipv4_addr, unsigned char *mac_addr) 
{
   struct arp_map *temp = NULL;

   printf("Getting mac for %x\n", ipv4_addr);
   temp = arp_map_list;
   while(temp) {
      if(temp->ipv4_addr == ipv4_addr) {
         strncpy(mac_addr, temp->mac_addr, 6);
         return 1;
      }
      temp = temp->next;
   }
   return 0;
}

int
add_mac(uint32_t ipv4_addr, unsigned char *mac_addr) 
{
   struct arp_map *temp = NULL;
   struct arp_map *last = NULL;

   printf("Adding mac for %x\n", ipv4_addr);
   temp = arp_map_list;
   while(temp) {
      temp = temp->next;
      last = temp;
   }
   temp = malloc(sizeof(struct arp_map));
   temp->next = NULL;
   if(last) {
      last->next = temp;
   }
   temp->ipv4_addr == ipv4_addr;
   strncpy(temp->mac_addr, mac_addr, 6);
   int i;
   for(i=0;i<6;i++) {
      printf("%x ", mac_addr);
   }
   printf("\n");
   return 1;
}
   
