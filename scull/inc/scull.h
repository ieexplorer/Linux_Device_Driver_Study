#ifndef SCULL_H_MODULE
#define SCULL_H_MODULE
#include <linux/cdev.h>
#include <linux/sem.h>

struct scull_dev {

	 struct 			scull_qset *data;
	 int 				quantum;
	 int 				qset;
	 unsigned long 		size;
	 unsigned int 		access_key;
	 stuct semaphore 	sem;
	 stuct cdev 		cdev;

};
#endif