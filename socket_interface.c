#include "tcp_in.h"
#include "tcp_tcb.h"
#include "socket_interface.h"
#include "types.h"
#include "tcp_common.h"
#include <pthread.h>
#include <rte_mempool.h>


static struct rte_ring *socket_tcb_ring_recv = NULL;
static const char *TCB_TO_SOCKET = "TCB_TO_SOCKET";
static const char *_MSG_POOL = "MSG_POOL";
static struct rte_mempool *buffer_message_pool;
void InitSocketInterface()
{
   socket_tcb_ring_recv = rte_ring_lookup(TCB_TO_SOCKET);
   buffer_message_pool = rte_mempool_lookup(_MSG_POOL);
   if(socket_tcb_ring_recv == NULL) {
      printf ("ERROR **** Failed to set scoket tcb ring.\n");
   }
   else {
      printf("Socket tcb ring recv side OK\n");
   }
   if(buffer_message_pool == NULL) {
      printf("ERROR **** socket tcb Message pool failed\n");
   }
   else {
      printf("socket tcb recv side OK.\n");
   }
}

int 
socket_open(STREAM_TYPE stream)
{
   struct tcb *ptcb;
// future set this as default instead of 2000.
   ptcb = alloc_tcb(2000, 2000);
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
   ptcb->sport = 0;
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
socket_send(int ser_id, char *message, int len)
{
   struct tcb *ptcb = NULL;
   struct rte_mbuf *mbuf = get_mbuf();
   ptcb = get_tcb_by_identifier(ser_id);
   sendtcpdata(ptcb, mbuf, message, len);
  // sendtcppacket(ptcb, mbuf, message, len);
  // ptcb->send_data(message, len); 
}

int
socket_read(int ser_id, char *buffer, int len)
{
// check all corner case before going further
//   assert(ptcb->WaitingOnRead == 0);
   struct tcb *ptcb = NULL;
   ptcb = get_tcb_by_identifier(ser_id);
 //  if(ptcb->WaitingOnAccept) {
   //   return 0;
      // don't allow multiple accepts hold on same socket.
  // }
   printf("scoket read called for identifier %d\n", ser_id);
   pthread_mutex_lock(&(ptcb->mutex));
   ptcb->WaitingOnRead = 1;
   pthread_cond_wait(&(ptcb->condAccept), &(ptcb->mutex));
   if(len < ptcb->read_buffer_len) {
      printf("ERROR: Failed to get all buffer data from read.\n");
      memcpy(buffer, ptcb->read_buffer, len); 
   }   
   else 
      memcpy(buffer, ptcb->read_buffer, ptcb->read_buffer_len); 
   ptcb->WaitingOnRead = 0;
   pthread_mutex_unlock(&(ptcb->mutex));
   
   return 10; 
}

int socket_read_nonblock(int ser_id, unsigned char *buffer)
{
   void *msg;
   while (rte_ring_dequeue(socket_tcb_ring_recv, &msg) < 0){
      usleep(5);
      continue;
   }
   printf("Received %s and len %d\n",(char *)msg, strlen(msg));
   memcpy(buffer, msg, strlen(msg));
   rte_mempool_put(buffer_message_pool, msg);
 
   return strlen(msg);//GetData(ser_id, buffer);
}

int
socket_close(int identifier)
{
   printf("closing tcb\n");
 //  remove_tcb(identifier);
   return 0;
}
