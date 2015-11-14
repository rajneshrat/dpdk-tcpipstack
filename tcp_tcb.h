#ifndef __TCP_TCB_H__
#define __TCP_TCB_H__
#include "tcp_common.h"
#include "tcp_windows.h"
#include <pthread.h>
#include "tcp_states.h"
#include <sys/types.h>
#include <inttypes.h>
extern int Ntcb;

// make mac address also part of it.
struct tcb
{
   int identifier;
   int dport;
   int sport;
   uint32_t ack;
   uint32_t next_seq;
   int WaitingOnAccept;
   int WaitingOnRead;
   char *read_buffer;
   int read_buffer_len;
   struct tcb *newpTcbOnAccept;
   uint32_t ipv4_dst;
   uint32_t ipv4_src;
   char dest_mac[6];
   char src_mac[6];
   ReceiveWindow *RecvWindow;
   TCP_STATE state;
   pthread_mutex_t mutex;
   pthread_cond_t condAccept; // used for read also 
};

struct tcb* alloc_tcb();
#endif
