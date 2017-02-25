#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include "etherout.h"
#include "counters.h"
#include <stdio.h>
#include "arp.h"
#include "main.h"
#include "debug.h"
#include "logger.h"
#include "timer.h"

static const char *_MSG_POOL = "MSG_POOL_IP_ETHER_PACKETS";
static const unsigned int pool_size = 1024;
static const unsigned int ring_size = 1024;
static struct rte_mempool *buffer_message_pool;
struct rte_ring *ip_to_ether_ring_send; // this ring will work only under same process as rte_mbuf pointer is shared here.
struct rte_ring *ip_to_ether_ring_recv;
struct rte_ring *ether_to_ip_ring_send;
struct rte_ring *ether_to_ip_ring_recv;
const char * IP_ETHER_RING_NAME = "IP_ETHER_RING";
const char * ETHER_IP_RING_NAME = "ETHER_IP_RING";

void InitIpToEtherRing(void )
{
 //  socket_tcb_ring_send = rte_ring_create(TCB_TO_SOCKET, socket_tcb_ring_size, SOCKET_ID_ANY, 0);
   int buffer_size = sizeof(struct rte_mbuf *);
   buffer_message_pool = rte_mempool_create(_MSG_POOL, pool_size,
            buffer_size, 32, 0,
            NULL, NULL, NULL, NULL,
            SOCKET_ID_ANY, 0);
   if(buffer_message_pool == NULL) {
      //printf("ERROR **** ip -- ether Message failed\n");
   }
   else {
      //printf("ip - ether message pool OK.\n");
   }
   ip_to_ether_ring_send = rte_ring_create(IP_ETHER_RING_NAME, ring_size, SOCKET_ID_ANY, 0);
   if(ip_to_ether_ring_send) {
      //printf("ip_to_ether_ring_send ring OK\n");
   }
   else {
      //printf("ERROR * ring ip_to_ether_ring_send failed\n");
   }
   ether_to_ip_ring_send = rte_ring_create(ETHER_IP_RING_NAME, ring_size, SOCKET_ID_ANY, 0);
   if(ether_to_ip_ring_send) {
      //printf("ether_to_ip_ring_send ring OK\n");
   }
   else {
      //printf("ERROR * ring ether_to_ip_ring_send failed\n");
   }
   ether_to_ip_ring_recv = rte_ring_lookup(ETHER_IP_RING_NAME);
   if(ether_to_ip_ring_recv) {
      //printf("ether_to_ip_ring_recv ring OK\n");
   }
   else {
      //printf("ERROR * ring ether_to_ip_ring_recv failed\n");
   }
   ip_to_ether_ring_recv = rte_ring_lookup(IP_ETHER_RING_NAME);
   if(ip_to_ether_ring_recv) {
      //printf("ip_to_ether_ring_recv ring OK\n");
   }
   else {
      //printf("ERROR * ring ip_to_ether_ring_recv failed\n");
   }
}

int
InitEtherInterface(void)
{
   InitIpToEtherRing();
   return 0;
}

int
ether_out(unsigned char *dst_mac, char *src_mac, uint16_t ether_type, struct rte_mbuf *mbuf)
{
   int i = 0;
   struct ether_hdr *eth = NULL;
   if(mbuf->udata64 & 0x4) {
      logger(LOG_ETHER, LOG_LEVEL_NORMAL, " got mbuf with ether hdr\n");
	 eth = rte_pktmbuf_mtod(mbuf, struct ether_hdr *);
   }
   else {
       eth = (struct ether_hdr *)rte_pktmbuf_prepend (mbuf, sizeof(struct ether_hdr)); 
   }
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
// fix this this should be automatically detect the network interface id.
   //printf("dumping mbuf to be send\n");
   //DumpMbuf(mbuf);
   send_packet_out(mbuf, 0, 1);
   static int counter_id = -1;
   if(counter_id == -1) {
      counter_id = create_counter("sent_rate");
   }
   int data_len = rte_pktmbuf_data_len(mbuf);

   counter_abs(counter_id, data_len);
   {
      static int counter_id = -1;
      if(counter_id == -1) {
         counter_id = create_counter("wire_sent");
      }
      int data_len = rte_pktmbuf_data_len(mbuf);
 
      counter_inc(counter_id, data_len);
   }
   (void) src_mac; // jusat to avoid warning
   src_mac = NULL;
   return 0;
}

int
EnqueueMBuf(struct rte_mbuf *mbuf)
{
   struct rte_mbuf **Msg;
   if (rte_mempool_get(buffer_message_pool, (void **)&Msg) < 0) {
       //printf ("Failed to get rte_mbuf message buffer\n");
/// / put assert ;
      return -1;
   }
   *Msg = mbuf;
   uint64_t time_u = get_time_usec();
   logger(LOG_TIME, LOG_LEVEL_NORMAL, "Enquing mbuf %p at time %u in enqueue mbuf.\n", mbuf, time_u);
   if (rte_ring_enqueue(ip_to_ether_ring_send, Msg) < 0) {
      //printf("Failed to send rte_mbuf message - message discarded\n");
      rte_mempool_put(buffer_message_pool, Msg);
   }
   else {
      //printf("mbuf enqueue = %p\n", mbuf);
   }
   return 0;
}

int
CheckEtherOutRing(void)
{
   struct rte_mbuf **Msg, *mbuf;
   int mac_status = 0;

   int num = rte_ring_dequeue(ip_to_ether_ring_recv, (void **)&Msg);
   if(num < 0) {
      return 0;
   }
   mbuf = *Msg;
   uint64_t time_u = get_time_usec();
   logger(LOG_TIME, LOG_LEVEL_NORMAL, "Dequeued  mbuf %p at time %u in %s.\n", mbuf, time_u, __FUNCTION__);
   logger(LOG_ETHER, LOG_LEVEL_NORMAL, "Mbuf received = %p\n", mbuf);
   rte_mempool_put(buffer_message_pool, Msg);
   struct ipv4_hdr *hdr = NULL;
   if(mbuf->udata64 & 0x4) {
      logger(LOG_ETHER, LOG_LEVEL_NORMAL, "getting ipv4 hdr, no ether headder in %p\n.", mbuf);
        hdr =  (struct ipv4_hdr *)((rte_pktmbuf_mtod(mbuf, unsigned char *) +
                            sizeof(struct ether_hdr)));
   }
   else {
        logger(LOG_ETHER, LOG_LEVEL_NORMAL, "getting ipv4 hdr, available ether headder in %p\n.", mbuf);
        hdr =  (struct ipv4_hdr *)(rte_pktmbuf_mtod(mbuf, struct ipv4_hdr*));
   }
   //printf("dumping ip hdr at before mac\n");
   //DumpIpHdr(hdr);
   unsigned char dest_mac[6];
   mac_status = get_mac(ntohl(hdr->dst_addr), dest_mac);
   if(mac_status) {
      logger(LOG_ETHER, LOG_LEVEL_NORMAL, "mac found sending packet out for %p\n.", mbuf);
      ether_out(dest_mac, NULL, ETHER_TYPE_IPv4, mbuf);
   }
   else {
      char buf[1024];
      int len = DumpIpHdrToBuf(hdr, buf);
      buf[len] = '\0';
      logger(LOG_ETHER, LOG_LEVEL_NORMAL, "mac not found pushing packet back to queue for %p %s.\n", mbuf, buf);
      // put this mbuf bak in queue.
   }
   return 0;
}

