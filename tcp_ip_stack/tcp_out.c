#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <rte_tcp.h>

#include <stdio.h>
#include "tcp_in.h"
#include "tcp_tcb.h"
#include "tcp.h"
#include "tcp_states.h"
#include "logger.h"
#include "main.h"
#include "tcp_out.h"
#include "ip.h"

uint8_t add_mss_option(struct rte_mbuf *mbuf, uint16_t mss_value)
{
   struct tcp_mss_option *mss_option = (struct tcp_mss_option *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_mss_option));
   mss_option->type = 2;
   mss_option->len = 4;
   mss_option->value = htons(mss_value);
   return 4; // 1 * 4 bytes
}

uint8_t add_winscale_option(struct rte_mbuf *mbuf, uint8_t value)
{
   struct tcp_winscale_option *option = (struct tcp_winscale_option *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_winscale_option));
   option->type = 3;
   option->len = 3;
   option->value = value;
   return 3;
}

uint8_t add_tcp_data(struct rte_mbuf *mbuf, unsigned char *data, uint8_t len)
{
   char *src = (char *)rte_pktmbuf_prepend (mbuf, len);
   memcpy(src, data, len);
   return len;
}

uint8_t add_timestamp_option(struct rte_mbuf *mbuf, uint32_t value, uint32_t echo)
{
   struct tcp_timestamp_option *option = (struct tcp_timestamp_option *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_timestamp_option));
   option->type = 8;
   option->len = 10;
   option->value = value;
   option->echo = echo;
   return 10;
}

int
RetransmitPacket(struct rte_mbuf* mbuf, struct tcb *ptcb, int data_len)
{
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_mtod(mbuf, struct tcp_hdr *);
   ip_out(ptcb, mbuf, ptcphdr, data_len); 
   return 0;
}

void reflect_reset(struct ipv4_hdr *ip_hdr, struct tcp_hdr *t_hdr)
{
   send_reset(ip_hdr, t_hdr);
}
// this should be renamed as reflect reset as it sends the reset for incoing packet passwd as argument.

void send_reset(struct ipv4_hdr *ip_hdr, struct tcp_hdr *t_hdr)
{
   printf("sending reset\n");
   logger(LOG_TCP, LOG_LEVEL_CRITICAL, "sending reset\n");
   struct rte_mbuf *mbuf = get_mbuf();
   //printf("head room = %d\n", rte_pktmbuf_headroom(mbuf));
  // rte_pktmbuf_adj(mbuf, sizeof(struct tcp_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr));
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
   //printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
      //printf("tcp header is null\n");
   }


   uint8_t tcp_len = 20 ;
   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->data_off = tcp_len;


   ptcphdr->src_port = t_hdr->dst_port;
   ptcphdr->dst_port = t_hdr->src_port;
   ptcphdr->sent_seq = t_hdr->recv_ack;
   ptcphdr->recv_ack = 0;
   ptcphdr->tcp_flags =  TCP_FLAG_RST;
   ptcphdr->rx_win = 12000;
   ptcphdr->tcp_urp = 0; 
   
  // struct ipv4_hdr *hdr = (struct ipv4_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct ipv4_hdr));
   //printf("head room4 = %d\n", rte_pktmbuf_headroom(mbuf));
       //printf("ip header is null\n");
   //    fflush(stdout);
   struct tcb ptcb;
   ptcb.identifier = 0; // dummy 
   
   ptcb.ipv4_dst = ip_hdr->dst_addr;  // fix it future , why we have htonl only for src
   ptcb.ipv4_src = htonl(ip_hdr->src_addr);
   fflush(stdout);
   ip_out(&ptcb, mbuf, ptcphdr, 0); 
}

void sendtcpdata(struct tcb *ptcb, unsigned char *data, int len)
{
   //uint8_t tcp_len = 0x50 + add_mss_option(mbuf, 1300);// + add_winscale_option(mbuf, 7);
   struct rte_mbuf *mbuf = get_mbuf(); // remove mbuf from parameters.
   uint8_t data_len = add_tcp_data(mbuf, data, len);
   uint8_t option_len = 0;//add_winscale_option(mbuf, 7) + add_mss_option(mbuf, 1300) + add_timestamp_option(mbuf, 203032, 0);
   uint8_t tcp_len = 20 + option_len;
   uint8_t pad = (tcp_len%4) ? 4 - (tcp_len % 4): 0;
   tcp_len += pad;
   logger(LOG_TCP, NORMAL, "padding option %d for tcb %u\n",  pad, ptcb->identifier); 
   char *nop = rte_pktmbuf_append (mbuf, pad); // always pad the option to make total size multiple of 4.
   memset(nop, 0, pad);
   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(LOG_TCP, NORMAL, "sending tcp data of len %d total %d for tcb %u\n", data_len, tcp_len, ptcb->identifier);
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(ptcb->dport);
   ptcphdr->dst_port = htons(ptcb->sport);
   ptcphdr->sent_seq = htonl(ptcb->next_seq);
   ptcb->next_seq += data_len;
   if(((ptcb->tcp_flags & TCP_FLAG_SYN) == TCP_FLAG_SYN) || ((ptcb->tcp_flags & TCP_FLAG_FIN) == TCP_FLAG_FIN)) {
      logger(LOG_TCP, NORMAL, "Increasing seq number by one for flag syn or fin for tcb %u\n", ptcb->identifier);
      ptcb->next_seq += 1;
   }
   logger(LOG_TCP, LOG_LEVEL_NORMAL, "Next seq number is %u flags %u for tcb %u\n", ptcb->next_seq, ptcb->tcp_flags, ptcb->identifier);
   ptcphdr->recv_ack = htonl(ptcb->ack);
   ptcphdr->data_off = tcp_len;
   ptcphdr->tcp_flags =  ptcb->tcp_flags;
   ptcphdr->rx_win = 12000;
   ptcphdr->cksum = 0x0000;
   ptcphdr->tcp_urp = 0; 

   ptcb->tcp_flags = 0; //reset the flags as we have sent them in packet now.
   //mbuf->ol_flags |=  PKT_TX_IP_CKSUM; // someday will calclate checkum here only.
   
 //  printf(" null\n");
  // fflush(stdout);
   logger(LOG_TCP, NORMAL, "[SENDING TCP PACKET] sending tcp packet seq %u ack %u and datalen %u for tcb %u\n", ntohl(ptcphdr->sent_seq), ntohl(ptcphdr->recv_ack), data_len, ptcb->identifier);
   // push the mbuf in send_window unacknowledged data and increase the refrence count of this segment also start the rto timer for this tcb.
   PushDataToSendWindow(ptcb, mbuf, ntohl(ptcphdr->sent_seq), ptcb->next_seq, data_len);  
   ip_out(ptcb, mbuf, ptcphdr, data_len); 
}
/*
void sendtcppacket(struct tcb *ptcb, struct rte_mbuf *mbuf, unsigned char *data, int len)
{
   //uint8_t tcp_len = 0x50 + add_mss_option(mbuf, 1300);// + add_winscale_option(mbuf, 7);
   uint8_t data_len = add_tcp_data(mbuf, data, len);
   uint8_t option_len = add_winscale_option(mbuf, 7) + add_mss_option(mbuf, 1300) + add_timestamp_option(mbuf, 203032, 0);

   uint8_t tcp_len = 20 + option_len;
   uint8_t pad = (tcp_len%4) ? 4 - (tcp_len % 4): 0;
   tcp_len += pad;
   logger(LOG_TCP, NORMAL, "padding option %d\n",  pad); 
   char *nop = rte_pktmbuf_append (mbuf, pad); // always pad the option to make total size multiple of 4.
   memset(nop, 0, pad);

   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(LOG_TCP, NORMAL, "sending tcp packet\n");
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(ptcb->dport);
   ptcphdr->dst_port = htons(ptcb->sport);
   ptcphdr->sent_seq = htonl(ptcb->next_seq);
   ptcb->next_seq += data_len;
   ptcb->next_seq ++;  // for syn 
   ptcphdr->recv_ack = htonl(ptcb->ack);
   ptcphdr->data_off = tcp_len;
   ptcphdr->tcp_flags =  SYN|ACK;
   ptcphdr->rx_win = 12000;
//   ptcphdr->cksum = 0x0001;
   ptcphdr->tcp_urp = 0; 
   //mbuf->ol_flags |=  PKT_TX_IP_CKSUM; // someday will calclate checkum here only.
   
 //  printf(" null\n");
  // fflush(stdout);
   ip_out(ptcb, mbuf, ptcphdr, data_len); 

}
*/

void sendtcpack(struct tcb *ptcb, struct rte_mbuf *mbuf, unsigned char *data, int len)
{
   //uint8_t tcp_len = 0x50 + add_mss_option(mbuf, 1300);// + add_winscale_option(mbuf, 7);
   uint8_t data_len = add_tcp_data(mbuf, data, len);
   uint8_t option_len = add_winscale_option(mbuf, 7) + add_mss_option(mbuf, 1300) + add_timestamp_option(mbuf, 203032, 0);

   uint8_t tcp_len = 20 + option_len;
   uint8_t pad = (tcp_len%4) ? 4 - (tcp_len % 4): 0;
   tcp_len += pad;
   logger(LOG_TCP, NORMAL, "padding option %d\n",  pad); 
   char *nop = rte_pktmbuf_append (mbuf, pad); // always pad the option to make total size multiple of 4.
   memset(nop, 0, pad);

   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(LOG_TCP, NORMAL, "sending tcp packet\n");
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(ptcb->dport);
   ptcphdr->dst_port = htons(ptcb->sport);
   ptcphdr->sent_seq = htonl(ptcb->next_seq);
   ptcb->next_seq += data_len;
  // ptcb->next_seq ++;  // for syn 
   ptcphdr->recv_ack = htonl(ptcb->ack);
   ptcphdr->data_off = tcp_len;
   ptcphdr->tcp_flags =  TCP_FLAG_ACK;
   ptcphdr->rx_win = 12000;
//   ptcphdr->cksum = 0x0001;
   ptcphdr->tcp_urp = 0; 
   //mbuf->ol_flags |=  PKT_TX_IP_CKSUM; // someday will calclate checkum here only.
   
 //  printf(" null\n");
  // fflush(stdout);
   ip_out(ptcb, mbuf, ptcphdr, data_len); 

}

void sendsyn(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   uint8_t tcp_len = 20 ;
   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(LOG_TCP, NORMAL, "sending tcp packet\n");
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(ptcb->dport);
   ptcphdr->dst_port = htons(ptcb->sport);
   ptcphdr->sent_seq = htonl(ptcb->next_seq);
   ptcb->next_seq ++; // for fin
   ptcphdr->recv_ack = htonl(ptcb->ack);
   ptcphdr->data_off = tcp_len;
   ptcphdr->tcp_flags =  TCP_FLAG_SYN;
   ptcphdr->rx_win = 12000;
//   ptcphdr->cksum = 0x0001;
   ptcphdr->tcp_urp = 0; 
   //mbuf->ol_flags |=  PKT_TX_IP_CKSUM; // someday will calclate checkum here only.
   
 //  printf(" null\n");
  // fflush(stdout);
   ip_out(ptcb, mbuf, ptcphdr, 0); 
}

// remove this api. not used now
void sendfin(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   uint8_t tcp_len = 20 ;
   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(LOG_TCP, NORMAL, "sending tcp packet\n");
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(ptcb->dport);
   ptcphdr->dst_port = htons(ptcb->sport);
   ptcphdr->sent_seq = htonl(ptcb->next_seq);
   ptcb->next_seq ++; // for fin
   ptcphdr->recv_ack = htonl(ptcb->ack);
   ptcphdr->data_off = tcp_len;
   ptcphdr->tcp_flags =  ptcb->tcp_flags | TCP_FLAG_FIN;
   ptcphdr->rx_win = 12000;
//   ptcphdr->cksum = 0x0001;
   ptcphdr->tcp_urp = 0; 
   //mbuf->ol_flags |=  PKT_TX_IP_CKSUM; // someday will calclate checkum here only.
   
 //  printf(" null\n");
  // fflush(stdout);
   ip_out(ptcb, mbuf, ptcphdr, 0); 
}

void sendack(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   sendtcpack(ptcb, mbuf, NULL, 0);
}

/*
void sendsynack(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   sendtcppacket(ptcb, mbuf, NULL, 0);
   //printf("head room = %d\n", rte_pktmbuf_headroom(mbuf));
  // rte_pktmbuf_adj(mbuf, sizeof(struct tcp_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr));
}
*/

