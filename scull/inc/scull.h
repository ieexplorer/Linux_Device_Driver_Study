#ifndef SCULL_H_MODULE
#define SCULL_H_MODULE
#include <linux/cdev.h>
#include <linux/sem.h>

struct scull_dev {

	 struct 			scull_qset *data;
	 int 				array_wr_ptr;
	 struct semaphore 	sem;
	 struct cdev 		cdev;

};
#endif