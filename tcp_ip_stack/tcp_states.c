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
   rte_pktmbuf_free(mbuf);
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
   logger(LOG_TCP_STATE, LOG_LEVEL_NORMAL, "signaling connect mutex.\n");
   pthread_cond_signal(&(ptcb->condAccept));
   pthread_mutex_unlock(&(ptcb->mutex));
   //printf("sending ack\n");
   //sendack(new_ptcb);
   sendack(ptcb); // this should not be here, use senddata api only but with 0 data.
   //sendsynack(ptcb);
   return 0;
}

// we have already recived the syn from remote but handhshake is not complete. last ack from remote is still pending.
int
tcp_syn_rcv(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   int tcp_len = (ptcphdr->data_off >> 4) * 4;
   int ip_header_len = ((iphdr->version_ihl) & (0x0f)) * 4;
   int datalen = ntohs(iphdr->total_length) - ip_header_len - tcp_len;
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp syn rcv state for tcb %u.\n", ptcb->identifier);
   logger(LOG_TCP_STATE, LOG_LEVEL_NORMAL, "tcp syn recv state\n");
   if(ntohl(ptcphdr->recv_ack) != ptcb->next_seq) {
      logger(LOG_TCP, LOG_LEVEL_CRITICAL, "dropping this packet, the ack %u not matching with our syn ack seq %u.\n", ntohl(ptcphdr->recv_ack), ptcb->next_seq);
      rte_pktmbuf_free(mbuf);
      reflect_reset(iphdr, ptcphdr);
      //clean the tcb also. future work.
      return -1;
   }

   struct tcb *pTcbOnAccept = ptcb->m_TcbWaitingOnAccept;
   // signal the tcb waiting on accpet.
   pthread_mutex_lock(&(pTcbOnAccept->mutex));
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "signaling accept mutex for tcb %u.\n", pTcbOnAccept->identifier);
   pthread_cond_signal(&(pTcbOnAccept->condAccept));
   pthread_mutex_unlock(&(pTcbOnAccept->mutex));
   //int datalen = rte_pktmbuf_pkt_len(mbuf) - (sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) + tcp_len);
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
   if(datalen != 0) {
      // this may be logical that we should support this case where the ack of handshake is lost but we are reciving the next packet.
      logger(LOG_TCP, LOG_LEVEL_CRITICAL, "seen non zero len on ack of threeway handshake. Data len = %u\n", datalen);
      tcp_established(ptcb, ptcphdr, iphdr, mbuf);
   //   logger(LOG_TCP, LOG_LEVEL_CRITICAL, "dropping this packet as we do not expect data with non zero len on ack of threeway handshake. Data len = %u\n", datalen);
   //   rte_pktmbuf_free(mbuf);
   //   reflect_reset(iphdr, ptcphdr);
   }
   else {
        rte_pktmbuf_free(mbuf);
   }
// also increase ack for data. future work.
   return 0;
}

int
tcp_established(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp established state for tcb %u.\n", ptcb->identifier);
   logger(LOG_TCP_STATE, LOG_LEVEL_NORMAL, "tcp established state\n");
   if(ptcphdr->tcp_flags & TCP_FLAG_FIN) {
      ptcb->need_ack_now = 1; // we need to ack this later.
      ptcb->state = TCP_FIN_2;
  //change state to tcpfin1 
   }
   int tcp_len = (ptcphdr->data_off >> 4) * 4;
   int ip_header_len = ((iphdr->version_ihl) & (0x0f)) * 4;
#if 0
   char *data =  (char *)(rte_pktmbuf_mtod(mbuf, unsigned char *) + 
                  sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) +  
                            tcp_len);
#endif
  // int datalen = rte_pktmbuf_pkt_len(mbuf) - (sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) + tcp_len);
   int datalen = ntohs(iphdr->total_length) - ip_header_len - tcp_len;
//iphdr->total_length;// - ptcphdr->data_off;
 //  char *data_buffer = (char *) malloc(datalen);
 //  memcpy(data_buffer, data, datalen);
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "ip header len %d total len % d tcp len %d buf len %d data len %u for tcb %u\n", ip_header_len,  ntohs(iphdr->total_length), tcp_len, rte_pktmbuf_pkt_len(mbuf), datalen, ptcb->identifier);
   if(ptcphdr->tcp_flags & TCP_FLAG_FIN) {
         logger(LOG_TCP, LOG_LEVEL_NORMAL, "got FIN flag for tcb %d\n", ptcb->identifier);
   }
   if(datalen || (ptcphdr->tcp_flags & TCP_FLAG_FIN)) {
      ptcb->need_ack_now = 1; // we need to ack this data, remember it.
     // ptcb->read_buffer = (unsigned char *) malloc(datalen);
     // ptcb->read_buffer_len = datalen;
      //memcpy(ptcb->read_buffer, data_buffer, datalen); 
      if(PushData(mbuf, ptcphdr, datalen, ptcb) < 0) {
           rte_pktmbuf_free(mbuf);
           mbuf = NULL;
      }
      // this is not needed now. we use queue now.
      pthread_mutex_lock(&(ptcb->mutex));
      if(ptcb->WaitingOnRead) {
         // we are using the same mutes for tcp recv. future change it to another cond wait.
         logger(LOG_TCP_STATE, LOG_LEVEL_NORMAL, "signaling accept mutex for recv data.\n");
         pthread_cond_signal(&(ptcb->condAccept));
         ptcb->WaitingOnRead = 0;
      }
      pthread_mutex_unlock(&(ptcb->mutex));
    //  free(ptcb->read_buffer);
   }
   else {
      if(ptcphdr->tcp_flags & TCP_FLAG_FIN) {  // this we dont need remove it.
         logger(LOG_TCP, LOG_LEVEL_NORMAL, "got FIN flag for tcb %d\n", ptcb->identifier);
         logger(LOG_TCP, LOG_LEVEL_NORMAL, "**********Increasing ack for fin  to %u for tcb %u\n", ntohl(ptcphdr->sent_seq) + 1, ptcb->identifier);
      //   ptcb->ack = ntohl(ptcphdr->sent_seq) + 1;  // this should add tcp len also.
       //  sendack(ptcb);
      }
   }
   return 0;
}

int
tcp_listen(struct tcb *ptcb, struct tcp_hdr* ptcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp listen state for tcb %u.\n", ptcb->identifier);
   struct tcb *new_ptcb = NULL;
   new_ptcb = alloc_tcb(2000, 2000); 
   if(new_ptcb == NULL) {
      logger(LOG_TCP_STATE, LOG_LEVEL_NORMAL, "Null tcb'\n");
      return 0;
   }
   logger(LOG_TCP_STATE, LOG_LEVEL_NORMAL, "Tcp listen state\n");
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
   new_ptcb->m_TcbWaitingOnAccept = ptcb;
   //printf("sending ack\n");
   //sendack(new_ptcb);
   new_ptcb->tcp_flags = TCP_FLAG_SYN | TCP_FLAG_ACK; 
   // ** addtcpoptiosn.
   //    // add tcpflags.
   logger(LOG_TCP_STATE, LOG_LEVEL_NORMAL, "Next seq number is %u flags %u\n", new_ptcb->next_seq, new_ptcb->tcp_flags);
   sendtcpdata(new_ptcb,  NULL, 0);
   rte_pktmbuf_free(mbuf);
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
   rte_pktmbuf_free(mbuf);
   return 0;
}

int
tcp_fin2(struct tcb *ptcb, struct tcp_hdr* tcphdr, struct ipv4_hdr *iphdr, struct rte_mbuf *mbuf)
{
   logger(LOG_TCP_STATE, LOG_LEVEL_NORMAL, "In tcp fin state.\n");
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "In tcp fin2 state for tcb %u.\n", ptcb->identifier);
     //sendfin(ptcb);  // future, remove it from here.
   if(ptcb->state == TCP_FIN_2) {
      ptcb->state = TCP_STATE_CLOSED;
   }
   else {
      ptcb->state = TCP_STATE_FIN_1;
   }
   rte_pktmbuf_free(mbuf);
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
   rte_pktmbuf_free(mbuf);
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
