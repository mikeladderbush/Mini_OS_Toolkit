#ifndef LIB_CLIENT_H
#define LIB_CLIENT_H
int send_command_to_daemon(const char *command, char *response, size_t response_size);
#endif // LIB_CLIENT_H