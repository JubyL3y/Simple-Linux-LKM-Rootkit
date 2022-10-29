#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <linux/namei.h>
#include <asm/ptrace.h>

#include "mkdir_hook.h"

static int hook_payload(const char __user *pathname, umode_t mode){
    char dir_name[NAME_MAX] = {0};
    long error = strncpy_from_user(dir_name, pathname, NAME_MAX);
    if (error > 0){
        printk(KERN_INFO "User trying to create directory with name %s\n", dir_name);
    }
    return 0;
}

#ifdef PTREGS_SYSCALL_STUBS
extern asmlinkage long (*orig_mkdir)(const struct pt_regs *);

asmlinkage int hook_mkdir(const struct pt_regs *regs){
    
    char __user *pathname = (char *)regs->di;
    umode_t mode = (umode_t)regs->si;
    
    hook_payload(pathname, mode);
    return orig_mkdir(regs);
}

#else
extern asmlinkage long (*orig_mkdir)(const char __user *pathname, umode_t mode);

asmlinkage int hook_mkdir(const char __user *pathname, umode_t mode){
   hook_payload(pathname, mode);
   return orig_mkdir(pathname, mode);
}

#endif
