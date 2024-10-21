obj-m += hello_mod.o
#obj-m += nouv_mod.o // c'est ici qu'on va rajouter les nv modules 
KDIR := /home/caca/linux-6.10.10  
PWD := $(shell pwd)

all:
        $(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
        $(MAKE) -C $(KDIR) M=$(PWD) clean

