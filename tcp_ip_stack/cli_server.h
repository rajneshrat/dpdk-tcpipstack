#ifndef __CLI_SERVER__
#define __CLI_SERVER__
void showcommand(int socket_id);
void command_config(int socket_id);
int cli_server_init(void);
int show_help(int socket_id);
int perform_command(int socket_id, char *input);
void add_ip(int socket_id, char *Ip);
void command_showinterfacemac(int socket_id);
void command_not_found(int socket_id);
void show_arps(int socket_id);
void show_interfaces(int socket_id);
void showcommand(int socket_id);
#endif
