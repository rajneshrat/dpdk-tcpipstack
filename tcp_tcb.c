#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <rte_tcp.h>

#include <stdio.h>
#include "tcp_tcb.h"
#include "tcp_common.h"
#include <pthread.h>

#define TOTAL_TCBS 10
pthread_mutex_t tcb_alloc_mutex;

int Ntcb = 0;


struct tcb tcbs[TOTAL_TCBS];
struct tcb* alloc_tcb()
{
   int i = 0;
   pthread_mutex_lock(&tcb_alloc_mutex);
   i = Ntcb;
   Ntcb++;
   assert(Ntcb < TOTAL_TCBS);
   pthread_mutex_unlock(&tcb_alloc_mutex);
   memset(&tcbs[i], 0, sizeof(struct tcb));
   return &tcbs[i];
}

struct tcb* get_tcb_by_identifier(int identifier)
{
   int i;
   struct tcb *ptcb = NULL;
   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      ptcb = &tcbs[i];
      //printf("Finding the tcb\n");
     // printf("identifier is  = %d\n",ptcb->identifier); 
      if(ptcb->identifier == identifier) {
         return ptcb;
      }
   }
}

struct tcb* findtcb(struct tcp_hdr *ptcphdr, struct ipv4_hdr *hdr)
{
   int i;
   struct tcb *ptcb = NULL;
   uint16_t dest_port = 0;
   uint16_t src_port = 0;

   dest_port = ntohs(ptcphdr->dst_port);
   src_port = ntohs(ptcphdr->src_port);
  // src_port = (ptcphdr->src_port);
   //dest_port = (ptcphdr->dst_port);
   //printf("Finding the tcb\n");
   //printf("dest port = %d\n",dest_port); 
//   struct ipv4_hdr *ip_hdr =  (struct ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, unsigned char *) +
  //       sizeof(struct ether_hdr));
   //struct tcp_hdr *ptcphdr = (struct tcp_hdr *) (ip_hdr + sizeof(struct ipv4_hdr)); 
  // struct tcp_hdr *ptcphdr = (struct tcp_hdr *) (ip_hdr + sizeof(struct ipv4_hdr)); 

   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      ptcb = &tcbs[i];
      printf("searching for tcb %u %u %d %d   found %u %u %d %d for %d\n", ntohl(hdr->src_addr), hdr->dst_addr, src_port, dest_port, ptcb->ipv4_src, ptcb->ipv4_dst, ptcb->sport, ptcb->dport, ptcb->identifier);
      if((ptcb->dport == dest_port) && 
         (ptcb->sport == src_port) && 
         (ptcb->ipv4_dst == hdr->dst_addr) &&
         (ptcb->ipv4_src == ntohl(hdr->src_addr))) {
         printf("Found stablized port\n");
         return ptcb; 
      }
   }
   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      ptcb = &tcbs[i];
      if(ptcb->state == LISTENING) {
         if((ptcb->dport) == dest_port) {
            printf("Found a listening tcb. listening port = %d, packet port = %d\n", ptcb->dport, dest_port);
            return ptcb; 
         }
      }
         
   }
   //printf("return NULL tcb\n");
   fflush(stdout);
   return NULL;
}

