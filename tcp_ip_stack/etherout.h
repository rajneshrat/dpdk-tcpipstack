#ifndef __ETHEROUT__
#define __ETHEROUT__
int ether_out(unsigned char *dst_mac, char *src_mac, uint16_t ether_type, struct rte_mbuf *mbuf);
int InitEtherInterface(void);
int CheckEtherOutRing(void);
int EnqueueMBuf(struct rte_mbuf *mbuf);
int ether_out(unsigned char *dst_mac, char *src_mac, uint16_t ether_type, struct rte_mbuf *mbuf);
void InitIpToEtherRing(void );
#endif
