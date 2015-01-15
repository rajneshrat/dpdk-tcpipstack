#include "socket_interface.h"

void init_socket_example(int port, char *ip)
{
   int i;
   int socket = socket_open(TCP);
   struct sock_addr addr;
   addr.port = port;
   for(i=0; i<4; i++) {
      addr.ip |= (ip[i] << i*8);
   }
   socket_bind(socket, &addr);
   socket_listen(socket, 5);
   struct sock_addr client;
   printf("Waiting for accept\n");
   socket_accept(socket, &client);
   printf("accepted the connection\n");
}

