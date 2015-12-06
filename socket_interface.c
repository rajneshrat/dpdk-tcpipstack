#include "tcp_in.h"
#include "tcp_tcb.h"
#include "socket_interface.h"
#include "types.h"
#include "tcp_common.h"
#include <pthread.h>
#include <rte_mempool.h>
enum Msg_Type {
   SOCKET_CLOSE,
   SEND_DATA,
};

typedef struct Socket_Send_Msg_ {
   int m_Msg_Type;
   int m_Identifier; // tcb identifier
   int m_Len; // data len;
   unsigned char m_Data[1400]; // data pointer
}Socket_Send_Msg;

//static struct rte_ring *socket_tcb_ring_recv = NULL;
static const char *TCB_TO_SOCKET = "TCB_TO_SOCKET";
static const char *SOCKET_TO_TCB = "SOCKET_TO_TCB";
static const char *_MSG_POOL = "MSG_POOL";
static struct rte_mempool *buffer_message_pool;


void InitSocketInterface()
{
//   socket_tcb_ring_recv = rte_ring_lookup(TCB_TO_SOCKET);
   buffer_message_pool = rte_mempool_lookup(_MSG_POOL);
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
   Socket_Send_Msg *Msg = NULL;
   struct tcb *ptcb = get_tcb_by_identifier(ser_id);
   if (rte_mempool_get(buffer_message_pool, &Msg) < 0) {
       printf ("Failed to get message buffer\n");
/// / put assert ;
   }
   Msg->m_Identifier = ser_id;
   Msg->m_Len = len;
   Msg->m_Msg_Type = SEND_DATA;
   memcpy(Msg->m_Data, message, len);
   if (rte_ring_enqueue(ptcb->tcb_socket_ring_send, Msg) < 0) {
      printf("Failed to send message - message discarded\n");
      rte_mempool_put(buffer_message_pool, Msg);
   }
   printf("****** Enqued for  %s and len %d and identifier %d\n",(char *)Msg->m_Data, Msg->m_Len, Msg->m_Identifier);
  // sendtcppacket(ptcb, mbuf, message, len);
  // ptcb->send_data(message, len); 
}

extern struct tcb *tcbs[];
extern int Ntcb;
extern pthread_mutex_t tcb_alloc_mutex;
int
check_socket_out_queue()
{
   Socket_Send_Msg *msg;
   int i;
   unsigned char message[1500];
   struct tcb *ptcb = NULL;
   struct tcb *temp = NULL;
   pthread_mutex_lock(&tcb_alloc_mutex);
   uint32_t TotalTcbs = Ntcb;
   pthread_mutex_unlock(&tcb_alloc_mutex);
    
   for(i=0; i<TotalTcbs; i++) {  // change it to hash type later
   //   if( i != 0)
      //printf("Checking tcb %d for sendingi %d %d\n", i, Ntcb, TotalTcbs);
      ptcb = tcbs[i];
      if(ptcb == NULL) {
         continue;
      }
      int num = rte_ring_dequeue(ptcb->tcb_socket_ring_recv, &msg);
      if(num < 0) {
         continue;
      }
      if(msg->m_Msg_Type == SOCKET_CLOSE) {
         temp = get_tcb_by_identifier(msg->m_Identifier);
         if(temp != ptcb) {
            printf ("Everything screewed at tcb'\n");
            exit(0);
      // put assert
         }
         sendfin(ptcb);
      }
      if(msg->m_Msg_Type == SEND_DATA) {
         printf("****** Received %s and len %d and identifier %d\n",(char *)msg->m_Data, msg->m_Len, msg->m_Identifier);
         memcpy(message, msg->m_Data, msg->m_Len);
     
         struct rte_mbuf *mbuf = get_mbuf();
         temp = get_tcb_by_identifier(msg->m_Identifier);
         if(temp != ptcb) {
            printf ("Everything screewed at tcb'\n");
            exit(0);
      // put assert
         }
         sendtcpdata(ptcb, mbuf, message, msg->m_Len);
      }
      rte_mempool_put(buffer_message_pool, msg);
   }
   return 0;
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
   struct tcb *ptcb = get_tcb_by_identifier(ser_id);
   while (rte_ring_dequeue(ptcb->socket_tcb_ring_recv, &msg) < 0){
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
   Socket_Send_Msg *Msg = NULL;
   struct tcb *ptcb = get_tcb_by_identifier(identifier);
   if (rte_mempool_get(buffer_message_pool, &Msg) < 0) {
       printf ("Failed to get message buffer\n");
/// / put assert ;
   }
   Msg->m_Identifier = identifier;
   Msg->m_Msg_Type = SOCKET_CLOSE;
   if (rte_ring_enqueue(ptcb->tcb_socket_ring_send, Msg) < 0) {
      printf("Failed to send message - message discarded\n");
      rte_mempool_put(buffer_message_pool, Msg);
   }
 //  remove_tcb(identifier);
   return 0;
}
