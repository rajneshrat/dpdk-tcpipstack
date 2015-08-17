#ifndef __TCP_STATES_H__
#define __TCP_STATES_H__
#include <rte_tcp.h>
//#include "tcp_tcb.h"
struct tcb;

int tcp_listen(struct tcb *, struct tcp_hdr *);
int tcp_closed(struct tcb *ptcb, struct tcp_hdr *mbuf);
int tcp_syn_rcv(struct tcb *ptcb, struct tcp_hdr* mbuf);

enum TCP_STATE_{
   CLOSED,
   LISTENING,
   SYN_RECV,
   TCP_STATES,
};

typedef int (tcpinstate)(struct tcb *, struct tcp_hdr*);

typedef enum TCP_STATE_ TCP_STATE;

extern tcpinstate *tcpswitch[TCP_STATES];
#endif
