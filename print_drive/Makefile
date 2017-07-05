obj-m := printer.o 

LIB_DIR=/home/elinux1/linux-3.10.y-master/
PWD=$(shell pwd)

all:
	make -C $(LIB_DIR) M=$(PWD) modules

clean:

	rm -rf *.o
	rm -rf *.ko
distclean:

	rm -rf *.ko
