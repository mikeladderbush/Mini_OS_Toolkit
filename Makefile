KERNEL_SRC = kernel_server/kernel_server.c
KERNEL_BIN = kernel_server_bin

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_SRC)
	cc $(KERNEL_SRC) -o $(KERNEL_BIN)

.PHONY: clean
clean:
	rm -f *.o $(KERNEL_BIN)
