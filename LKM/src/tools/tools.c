#include "tools.h"

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/limits.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

char* get_filename_by_pid(char* pid){
    char * filename;
    char * name;
    struct file* f;
    int sz, i;
    mm_segment_t fs;

    filename =  (char*)kzalloc(PATH_MAX+1, GFP_KERNEL);
    if (filename == NULL)
        return NULL;

    name = (char*)kzalloc(PATH_MAX+1, GFP_KERNEL);
    if ( name == NULL){
        kfree(filename);
        return NULL;
    }
    
    strcpy(filename, "/proc/");
    strcat(filename, pid);
    strcat(filename, "/cmdline");
    
    //printk(KERN_INFO"Try to open %s\n", filename);
    f = filp_open(filename, O_RDONLY, 0);
    if(IS_ERR(f)){
        kfree(filename);
        kfree(name);
        return NULL;
    }
    
    fs = get_fs();
    set_fs(KERNEL_DS);
    sz = f->f_op->read(f, name, PATH_MAX, &f->f_pos);
    set_fs(fs);
    
    if (sz ==0 ){
        kfree(filename);
        kfree(name);
        return NULL;
    }
    for (i=0; i<sz; ++i){
        if(name[i] ==' '){
            name[i]=0;
            break;
        }
    }
    filp_close(f, NULL);
    kfree(filename);
    return name;

}
