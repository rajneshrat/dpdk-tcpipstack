// arp data structure 

#include <rte_common.h>

typedef enum {
   ARP_REQ = 1,
   ARP_REPLY,
   RARP_REQ,
   RARP_REPLY,
} arp_type;

struct arp {
   uint16_t hw_type;
   uint16_t pr_type;
   uint8_t hw_len;
   uint8_t pr_len;
   uint16_t opcode;
   unsigned char src_hw_add[6];
   unsigned char src_pr_add[4];
   unsigned char dst_hw_add[6];
   unsigned char dst_pr_add[4];
}__packed__;
   
typedef enum {
   FREE = 0,
   PENDING,
   RESOLVED,
} arp_state;
  
struct rte_mbuf_queue {
	struct rte_mbuf *m;
   struct rte_mbuf_queue *next;
};

struct arp_entry {
   unsigned char pr_address[4];
   unsigned char hw_address[6];
   uint16_t ttl;
   uint8_t tries;
   arp_state state;
   struct rte_mbuf_queue *queue;
};


