#ifndef _RK_IOCTL_H
#define _RK_IOCTL_H    
#include <linux/ioctl.h>

#define IOC_MAGIC 'h'
#define IOCTL_TEST _IO ( IOC_MAGIC, 1 )
#define IOCTL_HIDE_FILE _IOW( IOC_MAGIC, 2, char *)
#define IOCTL_UNHIDE_FILE _IOW( IOC_MAGIC, 3, char *)

#define DEVPATH /dev/rootkitdev

#endif
