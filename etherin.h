typedef enum {
   ARP = 0x0806,
   IPV4 = 0x8000,
};

int
ether_in(struct rte_mbuf *mbuf);
