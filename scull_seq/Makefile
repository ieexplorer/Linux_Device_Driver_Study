MODULE_NAME = seq_scull_module
SRCS = scull.c scull_seq.c
OBJS = $(SRCS:.c=.o)

obj-m += $(MODULE_NAME).o

$(MODULE_NAME)-y = $(OBJS)

INCLUDE_DIR = -I $(src)/inc

ccflags-y := $(INCLUDE_DIR)

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
