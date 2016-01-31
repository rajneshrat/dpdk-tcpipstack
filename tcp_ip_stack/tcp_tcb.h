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
   uint16_t identifier;
   int dport;
   int sport;
   uint32_t ack;
   uint32_t next_seq;
   int WaitingOnAccept;
   int WaitingOnConnect;
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
   int m_IsSocketTcbRingIntialized;
   struct rte_ring *socket_tcb_ring_send;
   struct rte_ring *socket_tcb_ring_recv;
   struct rte_ring *tcb_socket_ring_recv;
   struct rte_ring *tcb_socket_ring_send;
   char TCB_TO_SOCKET_RING_NAME[100];
   char SOCKET_TO_TCB_RING_NAME[100];
   pthread_cond_t condAccept; // used for read also 
};

struct tcb* alloc_tcb(uint16_t, uint16_t);
struct tcb* findtcb(struct tcp_hdr *ptcphdr, struct ipv4_hdr *hdr);
struct tcb* get_tcb_by_identifier(int identifier);
#endif
