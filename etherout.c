#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>

#include <stdio.h>

int
ether_out(char *dst_mac, char *src_mac, struct rte_mbuf *mbuf)
{
   int i = 0;
   struct ether_hdr *eth;
   eth = (struct ether_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct ether_hdr)); 
   eth->ether_type = htons(ETHER_TYPE_IPv4);
   for(i=0;i<6;i++) {
      eth->d_addr.addr_bytes[i] = 0x12;
   }
   for(i=0;i<6;i++) {
      eth->s_addr.addr_bytes[i] = 0x22;
   }
   send_packet_out(mbuf, 0);
}
