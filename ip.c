#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
//#include <inttypes.h>
#include <stdio.h>
#include <rte_tcp.h>
#include "tcp_tcb.h"
#include "arp.h"
#include "tcp.h"

int
ip_in(struct rte_mbuf *mbuf)
{
	 struct ether_hdr *eth = rte_pktmbuf_mtod(mbuf, struct ether_hdr *);
    struct ipv4_hdr *hdr =  (struct ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, unsigned char *) +
                            sizeof(struct ether_hdr));
    unsigned char mac[6];
    print_arp_table();
    switch(hdr->next_proto_id) {
    case IPPROTO_TCP :
        if(get_mac(ntohl(hdr->src_addr), mac) == 0) { // remove me, should be inside add_mac
            add_mac(ntohl(hdr->src_addr), eth->s_addr.addr_bytes);
        }
        //printf("tcp packet\n");
        tcp_in(mbuf);
        break;
    default:
        break ;
    }

}
// for time being using this for checksum, will change later
uint16_t calculate_checksum(unsigned char *data, int len)
{
    uint8_t *val = data;
    int i;
    uint32_t sum = 0;
    for(i=0; i<len/2; i++) {
        uint16_t val16 = val[0] & 0xff;
        val16 = val16 << 8 | val[1];
        val = val + 2;
        sum = val16 + sum;
    }
    while( sum & 0xffff0000) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return ~sum;
}


int
ip_out(struct tcb *ptcb, struct rte_mbuf *mbuf, struct tcp_hdr *ptcphdr)
{
    unsigned char dest_mac[6];
    //printf("head room3 = %d\n", rte_pktmbuf_headroom(mbuf));
//    struct tcp_hdr *ptcphdr =  rte_pktmbuf_mtod(mbuf, struct tcp_hdr *);  
    struct ipv4_hdr *hdr = (struct ipv4_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct ipv4_hdr));
    //printf("head room4 = %d\n", rte_pktmbuf_headroom(mbuf));
    struct pseudo_tcp_hdr *pseudohdr = malloc (sizeof (struct pseudo_tcp_hdr));
    memset(pseudohdr, 0, sizeof(struct pseudo_tcp_hdr));
    ptcphdr->cksum = 0;
    static uint32_t count = 0;
    if(hdr == NULL) {
        //printf("ip header is null\n");
    //    fflush(stdout);
    }
    hdr->src_addr = ptcb->ipv4_dst;  // for outgoing src will be dest.
    //printf("dst ip is %x\n", ptcb->ipv4_dst);
    hdr->dst_addr = ptcb->ipv4_src;
    hdr->version_ihl = 4 << 4 | 5;
    hdr->next_proto_id = IPPROTO_TCP;
    hdr->hdr_checksum = 0;
    hdr->time_to_live = 127;
  //  hdr->total_length = htons(sizeof(struct ipv4_hdr) + sizeof(struct tcp_hdr) + 4);
    uint8_t tcp_len = (ptcphdr->data_off >> 4) * 4;
    hdr->total_length = htons( 20 + tcp_len);// htons(sizeof(struct ipv4_hdr) + sizeof(struct tcp_hdr) + 4);
    hdr->packet_id = count++;
    hdr->hdr_checksum = htons(calculate_checksum(hdr, sizeof(struct ipv4_hdr)));

    pseudohdr->src_ip = hdr->src_addr;
    pseudohdr->dst_ip = hdr->dst_addr; 
    pseudohdr->reserved = 0; 
    pseudohdr->protocol = (IPPROTO_TCP);
    pseudohdr->len = htons(tcp_len); 
    char *temp = malloc(sizeof(struct pseudo_tcp_hdr) + tcp_len); 
   // memset(temp, 0, sizeof(struct pseudo_tcp_hdr) + 20);
    memcpy(temp, pseudohdr, sizeof(struct pseudo_tcp_hdr));
    memcpy(temp + sizeof(struct pseudo_tcp_hdr), ptcphdr, tcp_len);
    ptcphdr->cksum = htons(calculate_checksum(temp, sizeof(struct pseudo_tcp_hdr) + tcp_len));
//    ptcphdr->cksum = get_ipv4_psd_sum(hdr); 
    get_mac(ptcb->ipv4_src, dest_mac);
    ether_out(dest_mac, NULL, ETHER_TYPE_IPv4, mbuf);
}
