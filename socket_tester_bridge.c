/* This is a sample code to show the use of socket apis. Right now this is part of overall code, later it will be moved away form main tcp code.
   This shows how to use the socket api to create socket, send and receive data.
*/

#include "socket_interface.h"
#include <pthread.h>
#include "logger.h"

void *DoWork2(void *test)
{
   int new_socket = *((int *) test);
   char buffer[1024];
   int len = 0;
   logger(SOCKET, NORMAL, "coming off accept\n");
   printf("Sending dat at socket %d\n", new_socket);
   socket_send(new_socket, "Hello World", 12);
   printf("waiting on socket read\n");
#if 0
   socket_read(new_socket, buffer, 10);
   printf("received from socket %s\n", buffer);
#endif
   len = 0;
   while(len < 22) {
      len += socket_read_nonblock(new_socket, buffer);
      if(len < 0) {
         printf("socket closed\n");
         break;
      }
      if(len==0) {
         printf("nothing to show\n");
      }
      else {
         printf("received from socket %s length %d\n", buffer, len);
      }
   }
   //printf("received from socket %s\n", buffer);
   printf("closing socket\n");
   socket_close(new_socket);
   free(test);
   return NULL;
}
void TcpDataA (struct Sock_Bridge_Msg *Msg)
{

}

void TcpDataB (struct Sock_Bridge_Msg *Msg)
{

}

void init_socket_bridge_example(int port, uint8_t *ip)
{
   int i = 0;
   int socket = socket_open(TCP_BRIDGE);
   struct sock_bridge_addr bridge_addr;
   bridge_addr.m_PortA = 0;
   bridge_addr.m_PortB = 1;
   bridge_addr.m_FuncA = TcpDataA;
   bridge_addr.m_FuncB = TcpDataB;
//   printf("ip is %x\n", addr.ip);
   sock_bridge_bind(bridge_addr);
   struct sock_addr client;
//   printf("Waiting for accept\n");
   logger(SOCKET, NORMAL, "waiting on accept\n");
   pthread_attr_t attr;
   int thread_id = 0;
   while(1) {
      int *socket_child = malloc(sizeof(int));
      *socket_child = socket_accept(socket, &client);
      pthread_create(&thread_id, NULL, DoWork2, socket_child); 
   }
//   printf("accepted the connection\n");
}

