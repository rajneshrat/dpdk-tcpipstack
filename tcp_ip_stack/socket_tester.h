#ifndef __SOCKET_TESTER__
#define __SOCKET_TESTER__

void init_socket_example(int port, uint8_t *ip);
void init_socket_example_connect(int port, uint8_t *ip);

void *DoWork(void *test);
#endif
