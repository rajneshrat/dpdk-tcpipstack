#ifndef __TCP_TCB_H__
#define __TCP_TCB_H__
#include "tcp_common.h"
#include <pthread.h>
#include "tcp_states.h"

struct tcb
{
   int dport;
   int sport;
   int WaitingOnAccept;
   struct tcb *newpTcbOnAccept;
   char ipv4_dest[4];
   char ipv4_src[4];
   TCP_STATE state;
   pthread_mutex_t mutex;
   pthread_cond_t condAccept; 
   int identifier;
};

struct tcb* alloc_tcb();
#endif
