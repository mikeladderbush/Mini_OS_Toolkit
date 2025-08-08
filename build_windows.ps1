# build_windows.ps1
Write-Host "Compiling windows_kernel_server.c with MinGW..."
gcc kernel_servers/windows_kernel_server.c -o windows_kernel_server_bin -lws2_32
Write-Host "Done!"