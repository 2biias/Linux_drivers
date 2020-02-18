


ifeq ($(KERNELRELEASE),)

#if KERNELDIR is not set by enviroment variable
ifeq ($(KERNELDIR),)
	KERNELDIR = ~/sources/rpi-4.19/
endif

#prefix for the cross compiler (compile to rpi)
ifeq ($(CCPREFIX),)
	CCPREFIX = arm-poky-linux-gnueabi-
endif

PWD := $(shell pwd) #current working directory

#fist target, no dependencies (always compile with make unless clean)
modules:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CCPREFIX) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers

.PHONY: modules clean

else
	ccflags-y := -DDEBUG -g -std=gnu99 -Wno-declaration-after-statement -Werror
	obj-m := hellomodule.o

endif
