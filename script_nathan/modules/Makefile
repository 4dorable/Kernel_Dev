obj-m += privesc.o
obj-m += hide_modules.o

KDIR := ../linux-6.10.10

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
