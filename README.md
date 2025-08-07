# Mini-OS Toolkit

A lightweight C-based toolkit to explore core systems concepts in WSL/Linux.

> **Note:** All features are in early construction and subject to change.

**Key Features (early prototypes):**

* **Task Manager:** Spawn and track processes/threads
* **Scheduler:** Preemptive Round-Robin with timer interrupts
* **Allocator:** Simple `mmap`-based allocator (`kmalloc`/`kfree`)
* **IPC:** POSIX shared-memory ring buffer with semaphores
* **RPC:** Framed commands over TCP sockets
* **Cache:** In-memory page cache with callback invalidation
* **Shell:** CLI to tie it all together
