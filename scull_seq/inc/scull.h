#ifndef SCULL_H_MODULE
#define SCULL_H_MODULE
#include <linux/cdev.h>
#include <linux/sem.h>
#include <linux/seq_file.h>

struct scull_dev {

	 struct 			scull_qset *data;
	 int 				array_wr_ptr;
	 struct semaphore 	sem;
	 struct cdev 		cdev;

};

extern struct scull_dev dev;
extern struct file_operations scull_proc_ops;
#endif