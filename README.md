# WSU CptS 360 for Fall 2023

This repository includes all C programming assignments during my Fall 2023 semester in Systems Programming. The class focused on properties of the Linux operating system, including file systems, job control and scheduling, and kernel modules. I improved my understanding of low-level concepts like memory management, direct hardware interaction, and efficient, robust software development.

## Homework

### Unix Simulation

I developed a Unix-like file system simulator using pointers, linked lists, and trees. The project included operations like creating/removing directories, changing directories, and saving/reloading the file system tree, emphasizing error handling for invalid operations.

I next created a Unix-like shell with job control, exploring processes and signaling. The shell supports moving jobs between background and foreground by handling signals like `SIGINT`, `SIGSTOP`, and `SIGCONT`.

We expanded on our understanding of processes by implementing CPU scheduling algorithms (FCFS, RR, SJF). This revealed practical insights on their behavior and impact on system performance.

### Linux Kernel Modules

I delved into Linux Kernel fundamentals through creating modules, using timers, and working with kernel-side linked lists in an Ubuntu virtual machine. The provided user application writes a PID to the `/proc` filesystem, which is read by the module to start a timer which tracks the process running time. When the process exits, the timer callback removes it from the kernel-side process list.

### HTTP Proxy

I implemented a simple HTTP proxy for forwarding `HTTP/1.0 GET` requests to a destination server. Using an understanding of sockets, ports, and HTTP headers, the proxy acts as the middleman while handling errors against malformed or malicious input. We demonstrated the proxy using telnet, curl, netcat, and a web browser to show its abilities and limitations.
