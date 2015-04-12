obj-m += chardev.o
obj-m += msrdrv.o

COMPILER = gcc
KERNELDIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)

all :
	make -C $(KERNELDIR) M=$(PWD) modules
	sudo mknod chardev c 100 0
	sudo insmod chardev.ko
	sudo mknod /dev/msrdrv c 223 0
	sudo chmod 666 /dev/msrdrv
	sudo insmod -f msrdrv.ko
	$(COMPILER) -o ioctl_test ioctl_test.c
	$(COMPILER) -o msrtest msrtest.c -lm

run1 :
	./ioctl_test

run2 :
	./msrtest

clean :
	make -C $(KERNELDIR) M=$(PWD) clean
	sudo rmmod chardev
	sudo rmmod msrdrv
	sudo rm /dev/msrdrv
	sudo rm -rf chardev
	rm -rf *.o *.ko
