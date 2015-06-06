#ifndef __IP__
#define __IP__

#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <assert.h>
#include <rte_ip.h>
#include <stdio.h>
#include "logger.h"
#include <pthread.h>

int
ip_in(struct rte_mbuf *mbuf);

int
InitIPQueue()
{
   logger(IP, NORMAL, "Initializing IP out queue.\n");
   
}

struct _ip_send_queue_ {
   struct rte_mbuf *mbuf;
   struct ip_send_queue *Next;
};

struct _ip_send_queue_ ip_send_queue;

#endif
