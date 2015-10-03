#include<stdio.h>
#include<string.h>  
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>  
#include "ether.h"
#include "cli_server.h"

void showcommand(int socket_id)
{
   char *command_list = "help\n test";
   write(socket_id , command_list , strlen(command_list));
}  

void command_config(int socket_id)
{
   int TotalInterfaces = GetTotalInterfaces();
   int i;
   int index = 0;
   char buffer[1024];
   index += sprintf(buffer + index, "total interfaces available = %d\n", TotalInterfaces);
   for(i=0;i<TotalInterfaces;i++) {
      index += sprintf(buffer + index, "configuring interface %d\n", i);
   }
   write(socket_id, buffer, strlen(buffer));
} 

void show_interfaces(int socket_id)
{
   int TotalInterfaces = GetTotalInterfaces();
   int i, j;
   int index = 0;
   char buffer[1024];
   index += sprintf(buffer + index, "total interfaces available = %d\n", TotalInterfaces);
   for(i=0;i<TotalInterfaces;i++) {
      index += sprintf(buffer + index, "mac for interface %d = ", i);
      for(j=0;j<6;j++) {
         index += sprintf(buffer + index, "%02X", InterfaceHwAddr[i][j]);
      }
      index += sprintf(buffer + index, "\n");
   }
   write(socket_id, buffer, strlen(buffer));
}

void command_not_found(int socket_id)
{
   int TotalInterfaces = GetTotalInterfaces();
   int index = 0;
   char buffer[1024];
   index += sprintf(buffer + index, " ****Command not found. Enter help for command list.****\n");
   write(socket_id, buffer, strlen(buffer));
}

void set_interface(int socket_id, int id)
{
   int TotalInterfaces = GetTotalInterfaces();
   int i;
   int index = 0;
   char buffer[1024];
   index += sprintf(buffer + index, "total interfaces available = %d\n", TotalInterfaces);
   for(i=0;i<TotalInterfaces;i++) {
      index += sprintf(buffer + index, "configuring interface %d\n", i);
   }
   write(socket_id, buffer, strlen(buffer));
}
  
 
void command_showinterfacemac(int socket_id)
{
   int i, j;
   int index = 0;
   char buffer[1024];
   for(i=0;i<2;i++) {
      for(j=0;j<6;j++) {
         index += sprintf(buffer + index, "%02X", InterfaceHwAddr[i][j]);
      }
      index += sprintf(buffer + index, "\n");
   }
   write(socket_id, buffer, strlen(buffer));
}

int perform_command(int socket_id, char *command)
{
   if(!strcmp(command, "showinterface")) {
      show_interfaces(socket_id);
      return 0;
   }
   if(!strcmp(command, "help")) {
      show_help(socket_id);
      return 0;
   }
   command_not_found(socket_id);
   return -1;
}

int show_help(int socket_id)
{
   int i, j;
   int index = 0;
   char buffer[1024];
   index += sprintf(buffer + index, "showinterface\n");
   index += sprintf(buffer + index, "configinterface\n");
   index += sprintf(buffer + index, "help\n");
   write(socket_id, buffer, strlen(buffer));
}

int cli_server_init()
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
     
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    int yes = 1;
    if ( setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
    {
        perror("setsockopt failed\n");
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 7788 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
     
    //Receive a message from client
    while((read_size = recv(client_sock , client_message , 2000 , 0)) > 0)
    {
        //write(client_sock , client_message , strlen(client_message));
      printf("client sent command %s\n", client_message);
    //    showcommand(client_sock);
      perform_command(client_sock, client_message);
     // command_config(client_sock);
      //command_showinterfacemac(client_sock);
      //write(client_sock , "client_message\n>" , strlen(client_message));
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
     
    return 0;
}
