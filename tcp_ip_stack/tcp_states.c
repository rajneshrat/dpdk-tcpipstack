#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>

#include <stdio.h>
#include "tcp_tcb.h"
#include "tcp.h"
#include "tcp_out.h"
#include "main.h"
#include "logger.h"


int
tcp_syn_sent(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
  // //printf("tcp_closed called\n");
// release semphone waiting at connect.
// we will come here only when we received syn-ack for our syn.
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp syn sent state for tcb %u.\n", ptcb->identifier);
   (void) mbuf;
   ptcb->state = TCP_ESTABLISHED;
//   ptcb->dport = ptcb->dport;
   ptcb->dport = htons(ptcphdr->dst_port);
//   ptcb->ipv4_dst = ptcb->ipv4_dst;
   ptcb->ipv4_dst = ntohl(iphdr->dst_addr);
//   ptcb->sport = ntohs(ptcphdr->src_port);
   ptcb->ack = ntohl(ptcphdr->sent_seq) + 1;
   ptcb->next_seq = 1;
   // set src port;
   // set ips.
   pthread_mutex_lock(&(ptcb->mutex));
   printf("signaling connect mutex.\n");
   pthread_cond_signal(&(ptcb->condAccept));
   pthread_mutex_unlock(&(ptcb->mutex));
   //printf("sending ack\n");
   //sendack(new_ptcb);
   sendack(ptcb); // this should not be here, use senddata api only but with 0 data.
   //sendsynack(ptcb);
   return 0;
}

int
tcp_syn_rcv(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp syn rcv state for tcb %u.\n", ptcb->identifier);
   printf("tcp syn recv state\n");
   int tcp_len = (ptcphdr->data_off >> 4) * 4;
   int datalen = rte_pktmbuf_pkt_len(mbuf) - (sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) + tcp_len);
   (void) iphdr;
   (void) mbuf;
   if(datalen != 0) {
       logger(LOG_TCP, LOG_LEVEL_CRITICAL, "seen data of len %d in handshake ack packet for tcb %u.\n", datalen, ptcb->identifier);
       ptcb->need_ack_now = 1; // we need to ack this later.
   }
   if((ptcphdr->tcp_flags & TCP_FLAG_SYN) || (ptcphdr->tcp_flags & TCP_FLAG_FIN) ) {
      ptcb->ack = ntohl(ptcphdr->sent_seq) + 1;  // this should add tcp len also. but not needed for syn-ack.
   }
   ptcb->state = TCP_ESTABLISHED;
// also increase ack for data. future work.
   return 0;
}

int
tcp_established(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp established state for tcb %u.\n", ptcb->identifier);
   printf("tcp established state\n");
   if(ptcphdr->tcp_flags & TCP_FLAG_FIN) {
      ptcb->need_ack_now = 1; // we need to ack this later.
      ptcb->state = TCP_FIN_2;
  //change state to tcpfin1 
   }
   int tcp_len = (ptcphdr->data_off >> 4) * 4;
#if 0
   char *data =  (char *)(rte_pktmbuf_mtod(mbuf, unsigned char *) + 
                  sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) +  
                            tcp_len);
#endif
   int datalen = rte_pktmbuf_pkt_len(mbuf) - (sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) + tcp_len);
//iphdr->total_length;// - ptcphdr->data_off;
 //  char *data_buffer = (char *) malloc(datalen);
 //  memcpy(data_buffer, data, datalen);
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "ip len %d tcp len %d buf len %d data len %u for tcb %u\n", ntohs(iphdr->total_length), tcp_len, rte_pktmbuf_pkt_len(mbuf), datalen, ptcb->identifier);
   if(datalen) {
      ptcb->need_ack_now = 1; // we need to ack this data, remember it.
     // ptcb->read_buffer = (unsigned char *) malloc(datalen);
     // ptcb->read_buffer_len = datalen;
      //memcpy(ptcb->read_buffer, data_buffer, datalen); 
      PushData(mbuf, ptcphdr, datalen, ptcb);
      pthread_mutex_lock(&(ptcb->mutex));
      if(ptcb->WaitingOnRead) {
         printf("signaling accept mutex.\n");
         pthread_cond_signal(&(ptcb->condAccept));
         ptcb->WaitingOnRead = 0;
      }
      pthread_mutex_unlock(&(ptcb->mutex));
    //  free(ptcb->read_buffer);
   }
   else {
      if(ptcphdr->tcp_flags & TCP_FLAG_FIN) {
         logger(LOG_TCP, LOG_LEVEL_NORMAL, "**********Increasing ack for syn or fin  to %u for tcb %u\n", ntohl(ptcphdr->sent_seq) + 1, ptcb->identifier);
      //   ptcb->ack = ntohl(ptcphdr->sent_seq) + 1;  // this should add tcp len also.
       //  sendack(ptcb);
      }
   }
// also increase ack for data. future work.
   return 0;
}

int
tcp_listen(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp listen state for tcb %u.\n", ptcb->identifier);
   struct tcb *new_ptcb = NULL;
   (void) mbuf;
   new_ptcb = alloc_tcb(2000, 2000); 
   if(new_ptcb == NULL) {
      printf("Null tcb'\n");
      return 0;
   }
   printf("Tcp listen state\n");
//   new_ptcb->identifier = 10;  identifier will be given by alloc.
   new_ptcb->state = SYN_RECV;
   new_ptcb->RecvWindow->CurrentSequenceNumber = ntohl(ptcphdr->sent_seq) + 1;
   logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL,  "Setting the CurrentSequenceNumber for ptcb %u to %u\n", ptcb->identifier, new_ptcb->RecvWindow->CurrentSequenceNumber);
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
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "signaling accept mutex for tcb %u.\n", ptcb->identifier);
   pthread_cond_signal(&(ptcb->condAccept));
   pthread_mutex_unlock(&(ptcb->mutex));
   //printf("sending ack\n");
   //sendack(new_ptcb);
   new_ptcb->tcp_flags = TCP_FLAG_SYN | TCP_FLAG_ACK; 
   // ** addtcpoptiosn.
   //    // add tcpflags.
   printf("Next seq number is %u flags %u\n", new_ptcb->next_seq, new_ptcb->tcp_flags);
   sendtcpdata(new_ptcb,  NULL, 0);
   //sendsynack(new_ptcb);
   //sendsynack(ptcb);
   return 0;
}

int
tcp_closed(struct tcb *ptcb, struct tcp_hdr* tcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp closed state for tcb %u.\n", ptcb->identifier);
   remove_tcb(ptcb->identifier);
  // //printf("tcp_closed called\n");
   (void) tcphdr;
   (void) iphdr;
   (void) mbuf;
   return 0;
}

int
tcp_fin2(struct tcb *ptcb, struct tcp_hdr* tcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   printf("In tcp fin state.\n");
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp fin2 state for tcb %u.\n", ptcb->identifier);
     //sendfin(ptcb);  // future, remove it from here.
   if(ptcb->state == TCP_FIN_2) {
      ptcb->state = TCP_STATE_CLOSED;
   }
   else {
      ptcb->state = TCP_STATE_FIN_1;
   }
   (void) mbuf;
   (void) iphdr;
   (void) tcphdr;
   return 0;
}

int
tcp_fin1(struct tcb *ptcb, struct tcp_hdr* tcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp fin1 state for tcb %u.\n", ptcb->identifier);
   if(ptcb->state == TCP_STATE_FIN_1) {
      ptcb->state = TCP_STATE_CLOSED;
   }
   else {
      ptcb->state = TCP_FIN_2;
   }
   (void) mbuf;
   (void) iphdr;
   (void) tcphdr;
   return 0;
}
      
  // //printf("tcp_closed called\n");

tcpinstate *tcpswitch[TCP_STATES] = { // the order of function must match with tcp states order. future work add assert if they differ 
   tcp_closed,
   tcp_listen,
   tcp_syn_sent,
   tcp_syn_rcv,
   tcp_established,
   tcp_fin1,
   tcp_fin2  // responder (syn-ack side) sent fin.
};
