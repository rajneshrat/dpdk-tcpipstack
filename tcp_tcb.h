#ifndef __TCP_TCB_H__
#define __TCP_TCB_H__
#include "tcp_common.h"
#include <pthread.h>
#include "tcp_states.h"
#include <sys/types.h>
#include <inttypes.h>

// make mac address also part of it.
struct tcb
{
   int dport;
   int sport;
   int WaitingOnAccept;
   struct tcb *newpTcbOnAccept;
   uint32_t ipv4_dst;
   uint32_t ipv4_src;
   TCP_STATE state;
   pthread_mutex_t mutex;
   pthread_cond_t condAccept; 
   int identifier;
};

struct tcb* alloc_tcb();
#endif
