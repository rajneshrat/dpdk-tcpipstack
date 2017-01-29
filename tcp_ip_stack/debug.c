#include <rte_tcp.h>
#include "debug.h"
#include "arp.h"
#include "tcp_tcb.h"

void DumpTcpHdr(struct tcp_hdr *hdr)
{
    printf(" source port %u\n", ntohs(hdr->src_port));
    printf(" dest port %u\n", ntohs(hdr->dst_port));
}

int DumpTcpHdrToBuf(struct tcp_hdr *hdr, char *buf)
{
    int len = sprintf(buf, " source port %u\n", ntohs(hdr->src_port));
    len += sprintf(buf + len, " dest port %u\n", ntohs(hdr->dst_port));
    len += sprintf(buf + len, " seq no %u\n", ntohl(hdr->sent_seq));
    return len;
}

void DumpIpHdr(struct ipv4_hdr *hdr)
{
    char ip_add[1024];
    print_add_in_buf(ntohl(hdr->src_addr), ip_add);
    printf(" source ip %u %s\n", ntohl(hdr->src_addr), ip_add);
    print_add_in_buf(ntohl(hdr->dst_addr), ip_add);
    printf(" dest ip %u %s\n", ntohl(hdr->dst_addr), ip_add);
} 

int DumpIpHdrToBuf(struct ipv4_hdr *hdr, char *buf)
{
    char ip_add[1024];
    print_add_in_buf(ntohl(hdr->src_addr), ip_add);
    int len = sprintf(buf, " source ip %u %s\n", ntohl(hdr->src_addr), ip_add);
    print_add_in_buf(ntohl(hdr->dst_addr), ip_add);
    len += sprintf(buf + len, " dest ip %u %s\n", ntohl(hdr->dst_addr), ip_add);
    return len;
}

int DumpMbufInBuf(struct rte_mbuf *mbuf, char *buf)
{
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *) ( rte_pktmbuf_mtod(mbuf, unsigned char *) + 
         sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr)); 
   struct ipv4_hdr *iphdr =  (struct ipv4_hdr *)((rte_pktmbuf_mtod(mbuf, unsigned char *) +
                            sizeof(struct ether_hdr)));
   int len = DumpTcpHdrToBuf(ptcphdr, buf);
   len += DumpIpHdrToBuf(iphdr, buf + len);
   buf[len] = '\0';
   return len;
}
void DumpMbuf(struct rte_mbuf *mbuf)
{
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *) ( rte_pktmbuf_mtod(mbuf, unsigned char *) + 
         sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr)); 
   struct ipv4_hdr *iphdr =  (struct ipv4_hdr *)((rte_pktmbuf_mtod(mbuf, unsigned char *) +
                            sizeof(struct ether_hdr)));
   DumpTcpHdr(ptcphdr);
   DumpIpHdr(iphdr);
}

void DumpMbufToBuf(struct rte_mbuf *mbuf, char *buf, size_t len)
{
    FILE *fp = fmemopen(buf, len, "w");
    rte_pktmbuf_dump (fp, mbuf, 0 );   
    fclose(fp);
}


uint32_t GetSeqFromMbuf(struct rte_mbuf *mbuf)
{
   //do not use it only for debugging when mbuf is not complete.
   struct tcp_hdr *ptcphdr = (struct tcp_hdr *) ( rte_pktmbuf_mtod(mbuf, unsigned char *));
        // sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr)); 
   return ntohl(ptcphdr->sent_seq);
}


void DumpTcb(struct tcb *ptcb)
{
   printf("src port = %u\n", ptcb->sport);
   printf("dst port = %u\n", ptcb->dport);
    char ip_add[1024];
    print_add_in_buf(ntohl(ptcb->ipv4_src), ip_add);
    printf(" source ip %u %s\n", ntohl(ptcb->ipv4_src), ip_add);
    print_add_in_buf(ntohl(ptcb->ipv4_dst), ip_add);
    printf(" dest ip %u %s\n", ntohl(ptcb->ipv4_dst), ip_add);
}

