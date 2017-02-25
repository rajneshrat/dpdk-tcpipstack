#ifndef __IP__
#define __IP__

#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <stdio.h>
#include "logger.h"
#include "tcp_tcb.h"
#include <rte_tcp.h>
#include <pthread.h>

int
ip_in(struct rte_mbuf *mbuf);

int
InitIPQueue(void);

struct _ip_send_queue_ {
   struct rte_mbuf *mbuf;
   struct ip_send_queue *Next;
};

struct _ip_send_queue_ ip_send_queue;
int ip_out(struct tcb *ptcb, struct rte_mbuf *mbuf, struct tcp_hdr *ptcphdr, int data_len);
uint16_t calculate_checksum(unsigned char *data, int len);
int ip_in(struct rte_mbuf *mbuf);

#endif
