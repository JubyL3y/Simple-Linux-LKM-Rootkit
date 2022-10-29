#ifndef _RK_HOOKS_H
#define _RK_HOOKS_H

#include <linux/version.h>

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

int install_syscall_hooks(void);
void remove_syscall_hooks(void);

#endif
