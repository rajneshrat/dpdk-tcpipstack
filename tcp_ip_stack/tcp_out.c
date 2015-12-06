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

uint8_t add_mss_option(struct rte_mbuf *mbuf, uint16_t mss_value)
{
   struct tcp_mss_option *mss_option = (struct tcp_option *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_mss_option));
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

uint8_t add_tcp_data(struct rte_mbuf *mbuf, char *data, uint8_t len)
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

void sendtcpdata(struct tcb *ptcb, struct rte_mbuf *mbuf, char *data, int len)
{
   //uint8_t tcp_len = 0x50 + add_mss_option(mbuf, 1300);// + add_winscale_option(mbuf, 7);
   uint8_t data_len = add_tcp_data(mbuf, data, len);
   uint8_t option_len = 0;//add_winscale_option(mbuf, 7) + add_mss_option(mbuf, 1300) + add_timestamp_option(mbuf, 203032, 0);
   uint8_t tcp_len = 20 + option_len;
   uint8_t pad = (tcp_len%4) ? 4 - (tcp_len % 4): 0;
   tcp_len += pad;
   logger(TCP, NORMAL, "padding option %d\n",  pad); 
   char *nop = rte_pktmbuf_append (mbuf, pad); // always pad the option to make total size multiple of 4.
   memset(nop, 0, pad);
   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(TCP, NORMAL, "sending tcp data of len %d total %d\n", data_len, tcp_len);
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct tcp_hdr));
  // printf("head room2 = %d\n", rte_pktmbuf_headroom(mbuf));
   if(ptcphdr == NULL) {
    //  printf("tcp header is null\n");
   }
   ptcphdr->src_port = htons(ptcb->dport);
   ptcphdr->dst_port = htons(ptcb->sport);
   ptcphdr->sent_seq = htonl(ptcb->next_seq);
   ptcb->next_seq += data_len;
   ptcphdr->recv_ack = htonl(ptcb->ack);
   ptcphdr->data_off = tcp_len;
   ptcphdr->tcp_flags =  ACK;
   ptcphdr->rx_win = 12000;
   ptcphdr->cksum = 0x0000;
   ptcphdr->tcp_urp = 0; 
   //mbuf->ol_flags |=  PKT_TX_IP_CKSUM; // someday will calclate checkum here only.
   
 //  printf(" null\n");
  // fflush(stdout);
printf("ok sending tcp data\n");
   ip_out(ptcb, mbuf, ptcphdr, data_len); 
}

void sendtcppacket(struct tcb *ptcb, struct rte_mbuf *mbuf, char *data, int len)
{
   //uint8_t tcp_len = 0x50 + add_mss_option(mbuf, 1300);// + add_winscale_option(mbuf, 7);
   uint8_t data_len = add_tcp_data(mbuf, data, len);
   uint8_t option_len = add_winscale_option(mbuf, 7) + add_mss_option(mbuf, 1300) + add_timestamp_option(mbuf, 203032, 0);

   uint8_t tcp_len = 20 + option_len;
   uint8_t pad = (tcp_len%4) ? 4 - (tcp_len % 4): 0;
   tcp_len += pad;
   logger(TCP, NORMAL, "padding option %d\n",  pad); 
   char *nop = rte_pktmbuf_append (mbuf, pad); // always pad the option to make total size multiple of 4.
   memset(nop, 0, pad);

   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(TCP, NORMAL, "sending tcp packet\n");
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

void sendtcpack(struct tcb *ptcb, struct rte_mbuf *mbuf, char *data, int len)
{
   //uint8_t tcp_len = 0x50 + add_mss_option(mbuf, 1300);// + add_winscale_option(mbuf, 7);
   uint8_t data_len = add_tcp_data(mbuf, data, len);
   uint8_t option_len = add_winscale_option(mbuf, 7) + add_mss_option(mbuf, 1300) + add_timestamp_option(mbuf, 203032, 0);

   uint8_t tcp_len = 20 + option_len;
   uint8_t pad = (tcp_len%4) ? 4 - (tcp_len % 4): 0;
   tcp_len += pad;
   logger(TCP, NORMAL, "padding option %d\n",  pad); 
   char *nop = rte_pktmbuf_append (mbuf, pad); // always pad the option to make total size multiple of 4.
   memset(nop, 0, pad);

   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(TCP, NORMAL, "sending tcp packet\n");
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
   ptcphdr->tcp_flags =  ACK;
   ptcphdr->rx_win = 12000;
//   ptcphdr->cksum = 0x0001;
   ptcphdr->tcp_urp = 0; 
   //mbuf->ol_flags |=  PKT_TX_IP_CKSUM; // someday will calclate checkum here only.
   
 //  printf(" null\n");
  // fflush(stdout);
   ip_out(ptcb, mbuf, ptcphdr, data_len); 

}
void sendfin(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   uint8_t tcp_len = 20 ;
   tcp_len = (tcp_len + 3) / 4;  // len is in multiple of 4 bytes;  20  will be 5
   tcp_len = tcp_len << 4; // len has upper 4 bits position in tcp header.
   logger(TCP, NORMAL, "sending tcp packet\n");
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
   ptcphdr->tcp_flags =  FIN;
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

void sendsynack(struct tcb *ptcb)
{
   struct rte_mbuf *mbuf = get_mbuf();
   sendtcppacket(ptcb, mbuf, NULL, 0);
   //printf("head room = %d\n", rte_pktmbuf_headroom(mbuf));
  // rte_pktmbuf_adj(mbuf, sizeof(struct tcp_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr));
}

