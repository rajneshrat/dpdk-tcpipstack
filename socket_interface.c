#include "tcp_in.h"
#include "tcp_tcb.h"
#include "socket_interface.h"
#include "types.h"
#include "tcp_common.h"
#include <pthread.h>


int 
socket_open(STREAM_TYPE stream)
{
   struct tcb *ptcb;

   ptcb = alloc_tcb();
   return ptcb->identifier; 
}

int
socket_bind(int identifier, struct sock_addr *serv_addr)
{
   struct tcb *ptcb;
   int i;

   ptcb = get_tcb_by_identifier(identifier);
   if(ptcb == NULL) {
      return -1; // use enum here
   }
   ptcb->ipv4_dst = serv_addr->ip;
   ptcb->dport = serv_addr->port;
   ptcb->state = LISTENING;
   return 0;
}

int
socket_listen(int identifier, int queue_len)
{
   struct tcb *ptcb;

   ptcb = get_tcb_by_identifier(identifier);
   // not implemented yet.
   return 0; //SUCCESS
}

int
socket_accept(int ser_id, struct sock_addr *client_addr)
{
   struct tcb *ptcb = NULL;
   struct tcb *new_ptcb = NULL;
   
   ptcb = get_tcb_by_identifier(ser_id);
   if(ptcb->WaitingOnAccept) {
      return 0;
      // don't allow multiple accepts hold on same socket.
   }
   ptcb->state = LISTENING;
   pthread_mutex_lock(&(ptcb->mutex));
   ptcb->WaitingOnAccept = 1;
   pthread_cond_wait(&(ptcb->condAccept), &(ptcb->mutex));
   new_ptcb = ptcb->newpTcbOnAccept;
   ptcb->WaitingOnAccept = 0;
   pthread_mutex_unlock(&(ptcb->mutex));
   
   return new_ptcb->identifier; 
}

int
socket_close(int identifier)
{
   return 0;
}
