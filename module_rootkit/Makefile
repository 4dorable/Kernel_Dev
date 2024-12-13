obj-m += hello_mod.o
obj-m += reverse.o
obj-m += privesc.o

KDIR := /home/caca/linux-6.10.10  # Change ça pour le chemin correct
ccflags-y += -Wno-error

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean




