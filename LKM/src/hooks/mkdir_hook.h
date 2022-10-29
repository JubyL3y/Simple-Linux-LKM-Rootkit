#ifndef _RK_MKDIR_HOOK_H
#define _RK_MKDIR_HOOK_H

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <asm/ptrace.h>


#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

#ifdef PTREGS_SYSCALL_STUBS

asmlinkage int hook_mkdir(const struct pt_regs *regs);

#else

asmlinkage int hook_mkdir(const char __user *pathname, umode_t mode);

#endif

#endif
