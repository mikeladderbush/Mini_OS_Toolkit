KERNEL_SRC = kernel_server/kernel_server.c
KERNEL_BIN = kernel_server_bin
LIBCLIENT_SRC = libclient/lib_client.c
LIBCLIENT_OBJ = libclient/lib_client.o
MT_SHELL_CORE_SRC = minitool_shell/mt_shell_core.c
MT_SHELL_CORE_BIN = mt_shell_core
MT_SHELL_LAUNCHER_SRC = minitool_shell/mt_shell.c
MT_SHELL_LAUNCHER_BIN = mt_shell

all: $(KERNEL_BIN) $(MT_SHELL_CORE_BIN) $(MT_SHELL_LAUNCHER_BIN)

$(KERNEL_BIN): $(KERNEL_SRC) $(LIBCLIENT_OBJ)
	cc -Wall -Wextra $(KERNEL_SRC) $(LIBCLIENT_OBJ) -o $(KERNEL_BIN)

$(LIBCLIENT_OBJ): $(LIBCLIENT_SRC)
	cc -Wall -Wextra -c $(LIBCLIENT_SRC) -o $(LIBCLIENT_OBJ)

$(MT_SHELL_CORE_BIN): $(MT_SHELL_CORE_SRC) $(LIBCLIENT_OBJ)
	cc -Wall -Wextra $(MT_SHELL_CORE_SRC) $(LIBCLIENT_OBJ) -o $(MT_SHELL_CORE_BIN)

$(MT_SHELL_LAUNCHER_BIN): $(MT_SHELL_LAUNCHER_SRC)
	cc -Wall -Wextra $(MT_SHELL_LAUNCHER_SRC) -o $(MT_SHELL_LAUNCHER_BIN)

.PHONY: clean
clean:
	rm -f $(KERNEL_BIN) $(LIBCLIENT_OBJ) $(MT_SHELL_CORE_BIN) $(MT_SHELL_LAUNCHER_BIN)
