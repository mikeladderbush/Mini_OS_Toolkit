LINUX_KERNEL_SRC = kernel_servers/linux_kernel_server.c
LINUX_KERNEL_BIN = linux_kernel_server_bin
LIBCLIENT_SRC = libclient/lib_client.c
LIBCLIENT_OBJ = libclient/lib_client.o
MT_SHELL_CORE_SRC = minitool_shell/mt_shell_core.c
MT_SHELL_CORE_BIN = mt_shell_core
MT_SHELL_LAUNCHER_SRC = minitool_shell/mt_shell.c
MT_SHELL_LAUNCHER_BIN = mt_shell

# Only build Linux binaries in WSL
all: $(LINUX_KERNEL_BIN) $(MT_SHELL_CORE_BIN) $(MT_SHELL_LAUNCHER_BIN)

$(LINUX_KERNEL_BIN): $(LINUX_KERNEL_SRC) $(LIBCLIENT_OBJ)
	cc -Wall -Wextra $(LINUX_KERNEL_SRC) $(LIBCLIENT_OBJ) -o $(LINUX_KERNEL_BIN)

$(LIBCLIENT_OBJ): $(LIBCLIENT_SRC)
	cc -Wall -Wextra -c $(LIBCLIENT_SRC) -o $(LIBCLIENT_OBJ)

$(MT_SHELL_CORE_BIN): $(MT_SHELL_CORE_SRC) $(LIBCLIENT_OBJ)
	cc -Wall -Wextra $(MT_SHELL_CORE_SRC) $(LIBCLIENT_OBJ) -o $(MT_SHELL_CORE_BIN)

$(MT_SHELL_LAUNCHER_BIN): $(MT_SHELL_LAUNCHER_SRC)
	cc -Wall -Wextra $(MT_SHELL_LAUNCHER_SRC) -o $(MT_SHELL_LAUNCHER_BIN)

.PHONY: clean windows_build

clean:
	rm -f $(LINUX_KERNEL_BIN) $(LIBCLIENT_OBJ) $(MT_SHELL_CORE_BIN) $(MT_SHELL_LAUNCHER_BIN) windows_kernel_server_bin

# Windows build: launches a Windows PowerShell terminal to run build_windows.ps1
windows_build:
	@echo "Launching Windows terminal to build windows_kernel_server_bin..."
	@powershell.exe -Command "Start-Process powershell -ArgumentList '-NoExit','-Command cd $(wslpath $(pwd)); ./build_windows.ps1'"