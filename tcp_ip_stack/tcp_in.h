#ifndef __TCP_IN_H__
#define __TCP_IN_H__
#include <rte_tcp.h>
#include "tcp_tcb.h"
#include "tcp_tcb.h"
#include <rte_tcp.h>
extern int tcpchecksumerror;
int tcp_in(struct rte_mbuf *mbuf);
int tcpok(struct tcb *ptcb, struct rte_mbuf *mbuf);

extern int tcpnopcb;

#endif
