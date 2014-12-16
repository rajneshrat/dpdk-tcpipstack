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
   for(i=0;i<4;i++) {
      ptcb->ipv4_dest[i] = serv_addr->ip[i];
   }
   ptcb->dport = serv_addr->port;
   ptcb->state = TCP_LISTENING;
   return 0;
}

int
socket_listen(int identifier, int queue_len)
{
   // not implemented yet.
   return 0; //SUCCESS
}

int
socket_accept(int ser_id, struct sock_addr *client_addr)
{
   struct tcb *ptcb;
   struct tcb *new_ptcb;
   
   if(ptcb->WaitingOnAccept) {
      return 0;
      // don't allow multiple accepts hold on same socket.
   }
   ptcb = get_tcb_by_identifier(ser_id);
   new_ptcb = alloc_tcb();
   pthread_mutex_lock(&(ptcb->mutex));
   ptcb->WaitingOnAccept = 1;
   ptcb->newpTcbOnAccept = new_ptcb;
   pthread_cond_wait(&(ptcb->condAccept), &(ptcb->mutex));
   ptcb->WaitingOnAccept = 0;
   pthread_mutex_unlock(&(ptcb->mutex));
   
   return new_ptcb->identifier; 
}

int
socket_close(int identifier)
{
   return 0;
}
