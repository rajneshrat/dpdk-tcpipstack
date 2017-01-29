#ifndef __TCP_COMMON_H__
#define __TCP_COMMON_H__

// data structure to send data from tcp to socket.
struct __attribute__((__packed__)) tcp_data
{
   int len;
   void *data;
};

#endif
