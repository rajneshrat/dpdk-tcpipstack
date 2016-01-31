#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <rte_tcp.h>

#include <stdio.h>
#include "logger.h"
#include "tcp_tcb.h"
#include "tcp_common.h"
#include <pthread.h>
#include "tcp_windows.h"

#define TOTAL_TCBS 10
pthread_mutex_t tcb_alloc_mutex;
extern const unsigned int socket_tcb_ring_size ;
const unsigned int tcb_socket_ring_size = 1024;

int Ntcb = 0;
struct tcb *tcbs[TOTAL_TCBS];

void InitTcpTcb()
{
   InitSocketTcbRing();
   InitSocketInterface();
   int i;
   for(i=0;i<TOTAL_TCBS; i++) {
      tcbs[i] = NULL;
   }
}

struct tcb* alloc_tcb(uint16_t MaxWindSize, uint16_t CurrentWindSize)
{
   static uint16_t IdentifierCount = 0;
   if(IdentifierCount == 0) {
      IdentifierCount = 1;
   }
   struct tcb *ptcb = malloc(sizeof(struct tcb));
   if(ptcb==NULL) {
      assert(0);
      printf("malloc failed\n");
   }
   printf("Allocated a new tcb %p\n", ptcb);
   memset(ptcb, 0, sizeof(struct tcb));
   ptcb->identifier = IdentifierCount++;
   sprintf(ptcb->TCB_TO_SOCKET_RING_NAME,"TtoS%d", ptcb->identifier);
   sprintf(ptcb->SOCKET_TO_TCB_RING_NAME,"StoT%d", ptcb->identifier);
// assert if size crosses of buffer ring name.
   ptcb->socket_tcb_ring_send = rte_ring_create(ptcb->TCB_TO_SOCKET_RING_NAME, socket_tcb_ring_size, SOCKET_ID_ANY, 0);
   ptcb->socket_tcb_ring_recv = rte_ring_lookup(ptcb->TCB_TO_SOCKET_RING_NAME);
   if(ptcb->socket_tcb_ring_recv == NULL) {
      printf ("ERROR **** Failed to set socket to tcb ring recv.\n");
   }
   else {
      printf("Socket tcb ring recv side OK. Ring Name %s\n", ptcb->TCB_TO_SOCKET_RING_NAME);
   }
   if(ptcb->socket_tcb_ring_send == NULL) {
      printf ("ERROR **** Failed to set socket to tcb ring send side.\n");
   }
   else {
      printf("Socket tcb ring send side OK, Ring Name %s\n", ptcb->TCB_TO_SOCKET_RING_NAME);
   }
   ptcb->tcb_socket_ring_send = rte_ring_create(ptcb->SOCKET_TO_TCB_RING_NAME, tcb_socket_ring_size, SOCKET_ID_ANY, 0);
   if(ptcb->tcb_socket_ring_send == NULL) {
      printf ("ERROR **** Failed to set tcb to socket ring send side.\n");
      //ASSERT(0);
   }
   else {
      printf("Socket tcb ring send side OK. Ring name %s\n", ptcb->SOCKET_TO_TCB_RING_NAME);
   }
   ptcb->tcb_socket_ring_recv = rte_ring_lookup(ptcb->SOCKET_TO_TCB_RING_NAME);
   if(ptcb->tcb_socket_ring_recv == NULL) {
      printf ("ERROR **** Failed to set tcb to scoket ring recv side. Ring Name %s\n", ptcb->SOCKET_TO_TCB_RING_NAME );
      //assert(0);
   }
   else {
      printf("Socket tcb ring send side OK\n");
   }
   ptcb->RecvWindow = AllocWindow(MaxWindSize, CurrentWindSize);
   pthread_mutex_lock(&tcb_alloc_mutex); // change name of lock to other.
   // lock is to safe ntcb array as it is accessed from other thread alos.
   assert(Ntcb < TOTAL_TCBS);
   tcbs[Ntcb] = ptcb;
   printf("adding tcb %p at %d with identifier %d\n", ptcb, Ntcb, ptcb->identifier);
   Ntcb++;
   pthread_mutex_unlock(&tcb_alloc_mutex);
   return ptcb;
}

struct tcb* get_tcb_by_identifier(int identifier)
//uint64_t get_tcb_by_identifier(int identifier)
{
   int i;
   //uint64_t value = 0x7fbe80008c0;
   uint64_t value = 0x7f780008c0;
   struct tcb *ptcb = NULL;
   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      ptcb = tcbs[i];
      //logger(TCB, NORMAL,"Finding the tcb\n");
     // logger(TCB, NORMAL,"identifier is  = %d\n",ptcb->identifier); 
      if(ptcb && ptcb->identifier == identifier) {
	 printf("get_tcb returing tcb as %p\n", ptcb);
     //    return value;
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
   //logger(TCB, NORMAL,"Finding the tcb\n");
   //logger(TCB, NORMAL,"dest port = %d\n",dest_port); 
//   struct ipv4_hdr *ip_hdr =  (struct ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, unsigned char *) +
  //       sizeof(struct ether_hdr));
   //struct tcp_hdr *ptcphdr = (struct tcp_hdr *) (ip_hdr + sizeof(struct ipv4_hdr)); 
  // struct tcp_hdr *ptcphdr = (struct tcp_hdr *) (ip_hdr + sizeof(struct ipv4_hdr)); 

   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      ptcb = tcbs[i];
      if(ptcb == NULL) {
         continue;
      }
      logger(TCB, NORMAL,"searching for tcb %u %u %d %d   found %u %u %d %d for %d\n", ntohl(hdr->src_addr), hdr->dst_addr, src_port, dest_port, ptcb->ipv4_src, ptcb->ipv4_dst, ptcb->sport, ptcb->dport, ptcb->identifier);
   //   logger(TCP, NORMAL, "sending syn-ack packet\n");
      if((ptcb->dport == dest_port) && 
         (ptcb->sport == src_port) && 
         (ptcb->ipv4_dst == hdr->dst_addr) &&
         (ptcb->ipv4_src == ntohl(hdr->src_addr))) {
         logger(TCB, NORMAL,"Found stablized port\n");
         return ptcb; 
      }
   }
   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      ptcb = tcbs[i];
      if(ptcb->state == LISTENING) {
         if((ptcb->dport) == dest_port) {
            logger(TCB, NORMAL,"Found a listening tcb. listening port = %d, packet port = %d\n", ptcb->dport, dest_port);
            return ptcb; 
         }
      }
         
   }
   //logger(TCB, NORMAL,"return NULL tcb\n");
   fflush(stdout);
   return NULL;
}

int remove_tcb(int identifier)
{
   struct tcb *ptcb = get_tcb_by_identifier(identifier);
   int i = 0;
   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      if(tcbs[i] == ptcb) {
         tcbs[i] = NULL;
      }
   }
}

int send_data(char *message, int len)
{
//   sendtcppacket(ptcb, mbuf, message, len);
}

