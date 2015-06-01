#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <inttypes.h>
#include <stdio.h>
#include <rte_tcp.h>
#include "tcp_tcb.h"
#include "arp.h"

int
ip_in(struct rte_mbuf *mbuf)
{
    struct ipv4_hdr *hdr =  (struct ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, unsigned char *) +
                            sizeof(struct ether_hdr));
    unsigned char mac[6];
    switch(hdr->next_proto_id) {
    case IPPROTO_TCP :
        if(get_mac(ntohl(hdr->src_addr), mac) == 0) {
            add_mac(ntohl(hdr->src_addr), mac);
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
ip_out(struct tcb *ptcb, struct rte_mbuf *mbuf)
{
    unsigned char dest_mac[6];
    //printf("head room3 = %d\n", rte_pktmbuf_headroom(mbuf));
    struct ipv4_hdr *hdr = (struct ipv4_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct ipv4_hdr));
    //printf("head room4 = %d\n", rte_pktmbuf_headroom(mbuf));
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
    hdr->total_length = htons(sizeof(struct ipv4_hdr) + sizeof(struct tcp_hdr) + 0);
    hdr->packet_id = count++;
    hdr->hdr_checksum = htons(calculate_checksum(hdr, sizeof(struct ipv4_hdr)));
    get_mac(ptcb->ipv4_dst, dest_mac);
    ether_out(dest_mac, NULL, ETHER_TYPE_IPv4, mbuf);
}
