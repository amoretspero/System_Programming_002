obj-m += chardev.o
COMPILER = gcc

all :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	$(COMPILER) -o ioctl_test ioctl_test.c
	sudo mknod chardev c 100 0
	sudo insmod chardev.ko

run :
	./ioctl_test

clean :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod chardev
	rm -rf chardev *.o *.ko
