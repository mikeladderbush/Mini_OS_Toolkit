KERNEL_SRC = kernel_server/kernel_server.c
KERNEL_BIN = kernel_server_bin
LIBCLIENT_SRC = libclient/lib_client.c
LIBCLIENT_OBJ = libclient/lib_client.o

all: $(KERNEL_BIN) $(LIBCLIENT_OBJ)

$(KERNEL_BIN): $(KERNEL_SRC)
	cc -Wall -Wextra $(KERNEL_SRC) -o $(KERNEL_BIN)

$(LIBCLIENT_OBJ): $(LIBCLIENT_SRC)
	cc -Wall -Wextra -c $(LIBCLIENT_SRC) -o $(LIBCLIENT_OBJ)

.PHONY: clean
clean:
	rm -f $(KERNEL_BIN) $(LIBCLIENT_OBJ)
