#include<stdio.h>
#include<string.h>  
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>  
#include <assert.h>
#include "ether.h"
#include "cli_server.h"
#include "arp.h"
#include "logger.h"
#include "main.h"


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
   int index = 0;
   char buffer[1024];
   index += sprintf(buffer + index, " ****Command not found. Enter help for command list.****\n");
   write(socket_id, buffer, strlen(buffer));
}

void add_ip(int socket_id, char *Ip)
{
   int TotalInterfaces = GetTotalInterfaces();
   int i, j = 0;
   int num = 0;
   int index = 0;
   char buffer[1024];
   struct Interface temp;
   temp.InterfaceNumber = 0;
printf("IP is %s\n", Ip);
   for(i=0;i<4;i++) {
      num = 0;
      while((Ip[j] != '.' ) && (Ip[j] != '\0') && (Ip[j] != ' ')) {
         num = num * 10 + (Ip[j] - 48);
         j++;
      }
      printf("%d %d " ,i, num);
      temp.IP[i] = num;
      if(Ip[j] == '\0' || Ip[j] == ' ') {
         if(i != 3) {
            printf("Error : failed to read ip properly %d\n", i);
         }
         break;
      }
      j++;
   //   temp.HwAddress[i] = 0x01;
   }
   InitLogger();
   //InitInterface(IfList, 1);
printf("confiuring interface\n");
   AddInterface(&temp);
   index += sprintf(buffer + index, "total interfaces available = %d\n", TotalInterfaces);
   for(i=0;i<TotalInterfaces;i++) {
      index += sprintf(buffer + index, "configuring interface %d\n", i);
   }
   write(socket_id, buffer, strlen(buffer));
}
  
 
void show_arps(int socket_id)
{
   char buffer[4096];
   int len = get_arp_table(buffer, 4096);
   if(len > 4096) {
      assert(0);
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

int perform_command(int socket_id, char *input)
{
   char command[1024];
   int i = 0;
   printf("input is %s\n", input);
   while(input[i] != '\0' && input[i] != ' ') {
      command[i] = input[i];
      i++;
   }
   command[i] = '\0';
   if(!strcmp(command, "showinterface")) {
      show_interfaces(socket_id);
      return 0;
   }
   if(!strcmp(command, "showarp")) {
      show_arps(socket_id);
      return 0;
   }
   if(!strcmp(command, "addip")) {
      i++;
      printf("parameter is %s\n", input + i);
      add_ip(socket_id, input + i);
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
   int index = 0;
   char buffer[1024];
   index += sprintf(buffer + index, "showinterface\n");
   index += sprintf(buffer + index, "showarp\n");
   index += sprintf(buffer + index, "configinterface\n");
   index += sprintf(buffer + index, "help\n");
   write(socket_id, buffer, strlen(buffer));
   return 0;
}

int cli_server_init(void)
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
     
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("cli server created");
     
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
    puts("cli bind done");
     
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
