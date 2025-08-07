#ifndef LIB_CLIENT_H
#define LIB_CLIENT_H
int send_command_to_linux(const char *command, char *response, size_t response_size);
int send_command_to_windows(const char *command, char *response, size_t response_size);
int send_command_to_daemons(const char *command, char *response, size_t response_size);
#endif // LIB_CLIENT_H