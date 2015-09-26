#ifndef __TCP_STATES_H__
#define __TCP_STATES_H__
#include <rte_tcp.h>
//#include "tcp_tcb.h"
struct tcb;

#if 0
int tcp_listen(struct tcb *, struct tcp_hdr *, struct ipv4_hdr *);
int tcp_closed(struct tcb *ptcb, struct tcp_hdr *mbuf, struct ipv4_hdr *);
int tcp_syn_rcv(struct tcb *ptcb, struct tcp_hdr* mbuf, struct ipv4_hdr *);
int tcp_established(struct tcb *ptcb, struct tcp_hdr* mbuf, struct ipv4_hdr *);
#endif
enum TCP_STATE_{
   CLOSED,
   LISTENING,
   SYN_RECV,
   TCP_ESTABLISHED,
   TCP_STATES,
};

typedef int (tcpinstate)(struct tcb *, struct tcp_hdr*, struct ipv4_hdr *, struct rte_mbuf *);

typedef enum TCP_STATE_ TCP_STATE;

extern tcpinstate *tcpswitch[TCP_STATES];
#endif
