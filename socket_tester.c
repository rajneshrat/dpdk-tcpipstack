#include "socket_interface.h"
#include "logger.h"

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
   socket_bind(socket, &addr);
   socket_listen(socket, 5);
   struct sock_addr client;
//   printf("Waiting for accept\n");
   logger(SOCKET, NORMAL, "waiting on accept\n");
   int new_socket = socket_accept(socket, &client);
   logger(SOCKET, NORMAL, "coming off accept\n");
   socket_send(new_socket, "Helloindia", 5);
//   printf("accepted the connection\n");
}

