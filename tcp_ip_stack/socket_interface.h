#ifndef __SOCKET_INTERFACE_H__
#define __SOCKET_INTERFACE_H__
#include <sys/types.h>
#include <inttypes.h>

struct sock_addr {
   int port;
   uint32_t ip;

};

struct Sock_Bridge_Msg {
   uint16_t m_pTcb_Identifier;
   void *m_Message;
};

typedef void (BridgeDataCallBack)(struct Sock_Bridge_Msg);

struct sock_bridge_addr {
   int m_PortA;
   int m_PortB;
   BridgeDataCallBack *m_FuncA; 
   BridgeDataCallBack *m_FuncB; 
};

enum _STREAM_TYPE_{
   TCP_STREAM,
   UDP_STREAM,
   TCP_BRIDGE // special case. this will bridge two ports together. all packets coming from one port will come till socket layer and then send back from another port.
};

typedef enum _STREAM_TYPE_ STREAM_TYPE;

int socket_open(STREAM_TYPE stream);

int sock_bridge_bind(struct sock_bridge_addr *addr);

int socket_bind(int identifier, struct sock_addr *serv_addr);


int socket_listen(int identifier, int queue_len);

int socket_close(int identifier);

int check_socket_out_queue(void);

void InitSocketInterface(void);
int socket_connect(int identifier, struct sock_addr *client_addr);
int socket_accept(int ser_id, struct sock_addr *client_addr);
int socket_send(int ser_id, const unsigned char *message, int len);
int socket_read(int ser_id, char *buffer, int len);
int socket_read_nonblock(int ser_id, unsigned char *buffer);

#endif
