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
#include <pthread.h>

#include "../../socket_interface.h"
#include "../../socket_tester.h"
#include <pthread.h>
#include "../../logger.h"

#define BYTES 1024

void respond(int connfd);

void respond(int connfd)
{
    char mesg[99999], *reqline[3], path[99999];
    FILE *  fd;
    char buf[1101];
    char *ROOT;
    ROOT = getenv("PWD");
    printf("starting http responder\n");
    logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "starting http responder for connfd %d\n", connfd);

    memset( (void*)mesg, (int)'\0', 99999 );
    int lines_read = socket_read(connfd, mesg, 4);
    logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "%s %d", mesg, lines_read);
    sleep(2);
    reqline[0] = strtok (mesg, " \t\n");
    if ( strncmp(reqline[0], "GET\0", 4)==0 )
    {
        logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "found http get requesti for %d\n", connfd);
         reqline[1] = strtok (NULL, " \t");
         reqline[2] = strtok (NULL, " \t\n");
         if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
         {
            logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "http bad request for %d\n", connfd);
            socket_send(connfd, (const unsigned char*)"HTTP/1.0 400 Bad Request\n", 25);
         }
         else
         {
             if ( strncmp(reqline[1], "/\0", 2)==0 )
             {
                logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "No file");
             }

             strcpy(path, ROOT);
             strcpy(&path[strlen(ROOT)], reqline[1]);
             logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "file: %s\n", path);

//    strcpy(file_name, "/home/1MB");
            // if ( (fd=fopen(path, "w")) != NULL )    //FILE FOUND
             if ( (fd=fopen("/home/1MB", "r")) != NULL )    //FILE FOUND
             {
                    logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "file opened successfully for %d\n", connfd);
             //       socket_send(connfd, (const unsigned char*)"HTTP/1.0 200 OK\n\nContent-length: 100\n\n", 38);
                    socket_send(connfd, (const unsigned char*)"HTTP/1.0 200 OK\n\n", 17);
                    int done = 0;
                    while(1) {
                        int len = 0;
                        while (len <= 1000){
                           char *str = fgets(buf + len, 100, fd);
                           if(str == NULL) {
                                done = 1;
                                break;
                           }
                           len = len + strlen(str); 
                          // len = len -1;
                        }
                        logger(LOG_DATA, LOG_LEVEL_NORMAL, "sending %s of len %d\n", buf, len);
                        while(socket_send(connfd, (const unsigned char*)buf, len) < 0) {
                            printf("socket fail to send. waiting\n");
                            sleep(1);
                        }
                        if(done == 1) {
                           break;
                        }
                    }
             }
             else {   
                socket_send(connfd, (const unsigned char*) "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "no file found for %d\n", connfd);       
             }
             printf("Successfully send file\n");
         }
     }
    logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "closing http responder for %d\n", connfd);
     printf("closing socket\n");
      socket_close(connfd);

/*
    rcvd=recv(clients[n], mesg, 99999, 0);

    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
        fprintf(stderr,"Client disconnected upexpectedly.\n");
    else    // message received
    {
        printf("%s", mesg);
        reqline[0] = strtok (mesg, " \t\n");
        if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
            reqline[1] = strtok (NULL, " \t");
            reqline[2] = strtok (NULL, " \t\n");
            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
            {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
                if ( strncmp(reqline[1], "/\0", 2)==0 )
                    reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                printf("file: %s\n", path);

                if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
                {
                    send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
                    while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
                        write (clients[n], data_to_send, bytes_read);
                }
                else    write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
            }
        }
    }

    //Closing SOCKET
    shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n]=-1;
    */
}

char ip[4] = {127,0,0,1};
int sample1(int, uint8_t*);
int sample2(int, uint8_t*);
void *DoRespond(void *test);
void *DoRespond(void *test)
{
   int connfd = *((int *) test);
    respond(connfd);
    free(test);
    return NULL;

}
int sample1(int port, uint8_t *ip)
{
   // this provides apache server file download functionality.
    int i;
    port = 80;

 //   char buf[1001];
//    char file_name[1024];
//    FILE *ptr_file;

    logger(LOG_SOCKET, NORMAL, " starting File to upload sample\n\n");
//    scanf("%s", file_name);
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
   pthread_t thread_id = 0;

   while(1) {
      int *connfd = malloc(sizeof(int));
    *connfd = socket_accept(fd, &client);
      pthread_create(&thread_id, NULL, DoRespond, connfd); 
    logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "connection accepted\n");
   // respond(connfd);
   }
    return 0;
}
int sample2(int port, uint8_t *ip)
{
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
    logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "connection accepted\n");
    ptr_file =fopen(file_name, "r");
    if (!ptr_file){
        logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "file opening failed.\n");
        return 1;
    }
    while (fgets(buf,1000, ptr_file)!=NULL){
        logger(LOG_SOCKET, LOG_LEVEL_NORMAL, "sending file\n");
        logger(LOG_SOCKET, LOG_LEVEL_NORMAL,  "sending %s\n", buf);
        socket_send(connfd, (const unsigned char*)buf, strlen(buf));
    }
    fclose(ptr_file);
    close(connfd);
    return 0;
}


