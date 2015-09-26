#include<stdio.h>
#include<string.h>  
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>  

int GetTotalInterfaces()
{
   return 1;
}
void showcommand(int socket_id)
{
   char *command_list = "help\n test";
   write(socket_id , command_list , strlen(command_list));
}  

void command_config(int socket_id)
{
   int TotalInterfaces = GetTotalInterfaces();
   int i;
   char buffer[1024];
   sprintf(buffer, "total interfaces available = %d\n", TotalInterfaces);
   write(socket_id, buffer, strlen(buffer));
   for(i=0;i<TotalInterfaces;i++) {
      sprintf(buffer, "configuring interface %d\n", i);
      write(socket_id, buffer, strlen(buffer));
   }
} 
 
int main(int argc , char *argv[])
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
      command_config(client_sock);
      //  write(client_sock , "client_message\n>" , strlen(client_message));
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
