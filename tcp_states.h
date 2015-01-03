#ifndef __TCP_STATES_H__
#define __TCP_STATES_H__
//#include "tcp_tcb.h"
struct tcb;

int tcp_listen(struct tcb *, struct rte_mbuf *);
int tcp_closed(struct tcb *ptcb, struct rte_mbuf *mbuf);

enum TCP_STATE_{
   CLOSED,
   LISTENING,
   SYN_RECV,
   TCP_STATES,
};

typedef int (tcpinstate)(struct tcb *, struct rte_mbuf*);

typedef enum TCP_STATE_ TCP_STATE;

extern tcpinstate *tcpswitch[TCP_STATES];
#endif
