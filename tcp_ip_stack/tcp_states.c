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


int
tcp_syn_sent(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
  // //printf("tcp_closed called\n");
// release semphone waiting at connect.
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
   sendack(ptcb);
   //sendsynack(ptcb);
   return 0;

}
int
tcp_syn_rcv(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   printf("tcp syn recv state\n");
   (void) iphdr;
   (void) mbuf;
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
   printf("tcp established state\n");
   if(ptcphdr->tcp_flags & TCP_FLAG_FIN) {
      ptcb->state = TCP_FIN_2;
  //change state to tcpfin1 
   }
   int tcp_len = (ptcphdr->data_off >> 4) * 4;
   char *data =  (char *)(rte_pktmbuf_mtod(mbuf, unsigned char *) + 
                  sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) +  
                            tcp_len);
   int datalen = rte_pktmbuf_pkt_len(mbuf) - (sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) + tcp_len);
//iphdr->total_length;// - ptcphdr->data_off;
   char *data_buffer = (char *) malloc(datalen);
   memcpy(data_buffer, data, datalen);
   printf("ip len %d tcp len %d buf len %d\n", ntohs(iphdr->total_length), tcp_len, rte_pktmbuf_pkt_len(mbuf));
   printf("data len %d tcp message = %s\n",datalen, data_buffer);
   if(datalen) {
      ptcb->read_buffer = (unsigned char *) malloc(datalen);
      ptcb->read_buffer_len = datalen;
      memcpy(ptcb->read_buffer, data_buffer, datalen); 
      PushData(ptcb->read_buffer, ptcphdr, datalen, ptcb);
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
      if((ptcphdr->tcp_flags & TCP_FLAG_SYN) || (ptcphdr->tcp_flags & TCP_FLAG_FIN) ) {
         printf("**********Increasing ack for syn or fin %u\n", ntohl(ptcphdr->sent_seq) + 1);
         ptcb->ack = ntohl(ptcphdr->sent_seq) + 1;  // this should add tcp len also.
         sendack(ptcb);
      }
   }
// also increase ack for data. future work.
   return 0;
}

int
tcp_listen(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
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
   // fix me this sould be send packet normal api.
   struct rte_mbuf *new_mbuf = get_mbuf();
   new_ptcb->tcp_flags = TCP_FLAG_SYN | TCP_FLAG_ACK; 
   // ** addtcpoptiosn.
   //    // add tcpflags.
   sendtcpdata(new_ptcb, new_mbuf, NULL, 0);
   //sendsynack(new_ptcb);
   //sendsynack(ptcb);
   return 0;
}

int
tcp_closed(struct tcb *ptcb, struct tcp_hdr* tcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
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
     sendfin(ptcb);  // future, remove it from here.
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
   printf("In tcp fin1 state.\n");
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
