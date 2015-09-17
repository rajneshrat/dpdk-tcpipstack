#ifndef __SOCKET_INTERFACE_H__
#define __SOCKET_INTERFACE_H__
#include <sys/types.h>
#include <inttypes.h>

struct sock_addr {
   int port;
   uint32_t ip;

};

enum _STREAM_TYPE_{
   TCP_STREAM,
   UDP_STREAM
};

typedef enum _STREAM_TYPE_ STREAM_TYPE;

int socket_open(STREAM_TYPE stream);


int socket_bind(int identifier, struct sock_addr *serv_addr);


int socket_listen(int identifier, int queue_len);


int socket_accept(int ser_id, struct sock_addr *client_addr);


int socket_close(int identifier);

#endif
