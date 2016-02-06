#ifndef __TCP_IN_H__
#define __TCP_IN_H__
#include <rte_tcp.h>
#include "tcp_tcb.h"
#include "tcp_tcb.h"
#include <rte_tcp.h>
extern int tcpchecksumerror;
int tcp_in(struct rte_mbuf *mbuf);
void send_reset(struct ipv4_hdr *ip_hdr, struct tcp_hdr *t_hdr);
int tcpok(struct tcb *ptcb, struct rte_mbuf *mbuf);

extern int tcpnopcb;

#endif
