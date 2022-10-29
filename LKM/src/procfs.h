#ifndef _RK_PROCFS_H
#define _RK_PROCFS_H

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/uaccess.h>

ssize_t proc_node_read( struct file* file, char* buffer, size_t len, loff_t* off );
ssize_t proc_node_write (struct file *file, const char *buffer, size_t length, loff_t * off);

#endif
