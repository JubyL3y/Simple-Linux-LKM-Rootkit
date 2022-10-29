/*
 *  Header for device operations
 */

#ifndef _RK_DEVICE_H
#define _RK_DEVICE_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "ioctl.h"

//
// Device open handler
//
int device_open (struct inode *inode, struct file *file);

//
// Device close handler
//
int device_release (struct inode *inode, struct file *file);

//
// Device read handler
//
ssize_t device_read (struct file *file, char *buffer, size_t length, loff_t * offset);

//
// Device write handler
//
ssize_t device_write (struct file *file, const char *buffer, size_t length, loff_t * off);


//
// Device IOCTL handler
//
long int device_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif
