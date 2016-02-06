#ifndef __TCP_OUT__
#define __TCP_OUT__

//void sendsynack(struct tcb *ptcb);
void sendack(struct tcb *ptcb);
void sendfin(struct tcb *ptcb);
void sendsyn(struct tcb *ptcb);
void sendtcpack(struct tcb *ptcb, struct rte_mbuf *mbuf, unsigned char *data, int len);
//void sendtcppacket(struct tcb *ptcb, struct rte_mbuf *mbuf, unsigned char *data, int len);
void sendtcpdata(struct tcb *ptcb, struct rte_mbuf *mbuf, unsigned char *data, int len);
uint8_t add_timestamp_option(struct rte_mbuf *mbuf, uint32_t value, uint32_t echo);
uint8_t add_tcp_data(struct rte_mbuf *mbuf, unsigned char *data, uint8_t len);
uint8_t add_winscale_option(struct rte_mbuf *mbuf, uint8_t value);
uint8_t add_mss_option(struct rte_mbuf *mbuf, uint16_t mss_value);
#endif
