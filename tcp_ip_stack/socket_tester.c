/* This is a sample code to show the use of socket apis. Right now this is part of overall code, later it will be moved away form main tcp code.
   This shows how to use the socket api to create socket, send and receive data.
*/

#include "socket_interface.h"
#include "socket_tester.h"
#include <pthread.h>
#include "unistd.h"
#include "logger.h"

void *DoWork(void *test)
{
   int new_socket = *((int *) test);
   unsigned char buffer[1024];
   int len = 0;
   int total_send = 5;
   int count = 1;
   logger(LOG_SOCKET, NORMAL, "coming off accept\n");
   printf("Sending dat at socket %d\n", new_socket);
   const char *data = "Hello World";
   while (count <= total_send) {
      socket_send(new_socket, (const unsigned char *) data, 12);
      printf("waiting on socket read\n");
      sleep(1);
      count ++;
   }
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

void init_socket_example_connect(int port, uint8_t *ip)
{
   int i = 0;
   int socket = socket_open(TCP_STREAM);
   struct sock_addr addr;
   addr.port = port;
   addr.ip = 0;
   for(i=0; i<4; i++) {
      addr.ip |= ip[i] << i*8;
   }
//   printf("ip is %x\n", addr.ip);
printf("connecting call\n");
   socket_connect(socket, &addr);
printf("connected.\n");
#if 0
   socket_bind(socket, &addr);
   socket_listen(socket, 5);
   struct sock_addr client;
//   printf("Waiting for accept\n");
   logger(SOCKET, NORMAL, "waiting on accept\n");
   pthread_attr_t attr;
   int thread_id = 0;
   while(1) {
      int *socket_child = malloc(sizeof(int));
      *socket_child = socket_accept(socket, &client);
      pthread_create(&thread_id, NULL, DoWork, socket_child); 
   }
#endif
//   printf("accepted the connection\n");
}
void init_socket_example(int port, uint8_t *ip)
{
   int i = 0;
   int socket = socket_open(TCP_STREAM);
   struct sock_addr addr;
   addr.port = port;
   addr.ip = 0;
   for(i=0; i<4; i++) {
      addr.ip |= ip[i] << i*8;
   }
//   printf("ip is %x\n", addr.ip);
#if 1
   socket_bind(socket, &addr);
   socket_listen(socket, 5);
   struct sock_addr client;
//   printf("Waiting for accept\n");
   logger(LOG_SOCKET, NORMAL, "waiting on accept\n");
   pthread_t thread_id = 0;
   while(1) {
      int *socket_child = malloc(sizeof(int));
      *socket_child = socket_accept(socket, &client);
      pthread_create(&thread_id, NULL, DoWork, socket_child); 
   }
#endif
//   printf("accepted the connection\n");
}


