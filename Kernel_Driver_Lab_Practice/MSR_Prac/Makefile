obj-m := msrdrv.o

KERNELDIR = /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all :
	make -C $(KERNELDIR) M=$(PWD) modules
	sudo mknod /dev/msrdrv c 223 0
	sudo chmod 666 /dev/msrdrv
	sudo insmod -f msrdrv.ko
	gcc -o msrtest msrtest.c -lm

run :
	./msrtest

clean :
	make -C $(KERNELDIR) M=$(PWD) clean
	sudo rmmod msrdrv
	sudo rm /dev/msrdrv
	rm -rf *.o *.ko
