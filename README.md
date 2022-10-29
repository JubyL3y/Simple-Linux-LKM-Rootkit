# Simple-Linux-LKM-Rootkit
A simple implementation of a kernel-level rootkit. Includes the functionality to intercept some system calls, as well as the functionality to change the rights of a certain process.

# Features
1) Interception of systemcalls using Ftrace
2) Intercepted syscalls:
    - Getdents
  - Getdents64
  - Read
  - Mkdir
  - Readdir
3) Privilege Escalation for process by his pid
4) Communication with the driver is implemented through channels:
  - Procfs
  - Chardev
  - IOCTL code
5) Tested at kernel 3.x - 4.x, 5.x not tested
6) Functional:
  - Hide/Unhide Process
  - Hide/Unhide Network port
  - Hide/Unhide Kernel module
  - Hide/Unhide File
  - Change process credentials ( Privilege escalation to root )
  


# Project structure
## LKM
LKM located in LKM directory. This directory contains 2 folder: src and build. Build folder contains Makefile for building LKM module. Src directory contains full source code of rootkit with main file named as rk_main.c

## UM
UM located in LKM directory and has same folder structure as LKM folder.

# Build and Install
Use make utility for build LKM and UM. After install ko file using insmod utility.
