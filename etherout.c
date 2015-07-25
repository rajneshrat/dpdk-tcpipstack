#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>

#include <stdio.h>

int
ether_out(char *dst_mac, char *src_mac, uint16_t ether_type, struct rte_mbuf *mbuf)
{
   int i = 0;
   struct ether_hdr *eth;
   eth = (struct ether_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct ether_hdr)); 
   eth->ether_type = htons(ether_type);
   for(i=0;i<6;i++) {
      eth->d_addr.addr_bytes[i] = dst_mac[i];
   }
//   for(i=0;i<6;i++) {
   eth->s_addr.addr_bytes[0] = 0x6a;
   eth->s_addr.addr_bytes[1] = 0x9c;
   eth->s_addr.addr_bytes[2] = 0xba;
   eth->s_addr.addr_bytes[3] = 0xa0;
   eth->s_addr.addr_bytes[4] = 0x96;
   eth->s_addr.addr_bytes[5] = 0x24;
//   }
   send_packet_out(mbuf, 0);
}
