#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "../../socket_interface.h"
#include "../../socket_tester.h"
#include <pthread.h>
#include "../../logger.h"
char ip[4] = {127,0,0,1};
int sample1(int, uint8_t*);
int sample1(int port, uint8_t *ip)
{
    int listenfd = 0;
    int i;

    char buf[1001];
    char file_name[1024];
    FILE *ptr_file;

    logger(LOG_SOCKET, NORMAL, " starting File to upload sample\n\n");
//    scanf("%s", file_name);
    strcpy(file_name, "/home/1MB");
    int fd = socket_open(TCP_STREAM);
    struct sock_addr addr;
    addr.port = port;
    addr.ip = 0;
    for(i=0; i<4; i++) {
       addr.ip |= ip[i] << i*8;
    }
//   printf("ip is %x\n", addr.ip);
   socket_bind(fd, &addr);
   socket_listen(fd, 5);
   struct sock_addr client;

    int connfd = socket_accept(fd, &client);
    printf("connection accepted\n");
    ptr_file =fopen(file_name, "r");
    if (!ptr_file){
        printf("file opening failed.\n");
        return 1;
    }
    while (fgets(buf,1000, ptr_file)!=NULL){
        printf("sending file\n");
        printf ("sending %s\n", buf);
        socket_send(connfd, (const unsigned char*)buf, strlen(buf));
    }
    fclose(ptr_file);
    close(connfd);
    close(listenfd);
    return 0;
}


