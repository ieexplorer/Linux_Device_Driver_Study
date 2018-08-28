#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <linux/uaccess.h>	/* copy_*_user */
#include <linux/device.h>
#include <linux/kdev_t.h>

#include "scull.h"
#include "scull_qset.h"


MODULE_LICENSE("Dual BSD/GPL");


unsigned int scull_minor = 0;
unsigned int scull_major = 0;
unsigned int scull_nr_dev = 1;

dev_t i_dev;

int devno = 0;

ssize_t scull_read (struct file *filp, char __user *buf, size_t count, loff_t *f_ops);
ssize_t scull_write (struct file *filp, const char __user *buf, size_t count, loff_t *f_ops);

int scull_open (struct inode *inode, struct file *filp);
int scull_release(struct inode *inode, struct file *filp);

struct file_operations scull_fops = 
{
	.owner 		= THIS_MODULE,
	// .llseek 	= scull_llseek,
	.read 		= scull_read,
	.write 		= scull_write,
	// .ioctl 		= scull_ioctl,
	.open 		= scull_open,
	.release	= scull_release,
};

struct scull_dev dev;
struct class *scull_class;

static char *scull_devnode (struct device *dev, umode_t *mode)
{
	if (!mode) // Essential!!!
        return NULL;
	
	*mode = 0666;
	return NULL;
}

static void scull_setup_cdev (struct scull_dev * dev)
{
	int err = 0;

	devno = MKDEV (scull_major, scull_minor);
	
	cdev_init (&(dev -> cdev), &scull_fops);
	
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops 	= &scull_fops; 
	
	err = cdev_add (&dev->cdev, devno, 1);

	if(err != 0)
	{	
		cdev_del (&(dev -> cdev));
		printk (KERN_NOTICE "Error %d adding scull%d\n", err, devno);
	}
	
	scull_class = class_create(THIS_MODULE, "scull_class");

	if(IS_ERR(scull_class))
    {
        printk(KERN_ALERT "class_create error..\n");
        return ;
    }
    /*Set the permission of device */
	scull_class -> devnode = scull_devnode;

	if ( device_create(scull_class, NULL, devno, NULL, "scull_device") == NULL)
		    printk(KERN_ALERT "scull: device failed!\n");

	return ;
}

static int scull_init(void)
{
	// int i = 0;
	int result = 0;

    printk(KERN_ALERT "scull start!\n");
    
    result = alloc_chrdev_region (&i_dev, scull_minor, scull_nr_dev, "scull_module");

    if(result != 0)
    {
    	printk (KERN_WARNING "scull: can't get major %d\n", scull_major);
    	return result;
    }

    scull_major = MAJOR(i_dev);
    scull_setup_cdev (&dev);

    return 0;
}

int scull_open (struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;
	dev = container_of (inode -> i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;

	dev -> array_wr_ptr = 0;

	return 0;
}

static void scull_exit(void)
{
    
	device_destroy(scull_class, devno);
	class_destroy(scull_class);

	cdev_del(&(dev.cdev));
	unregister_chrdev_region(devno, scull_nr_dev);
    
    printk(KERN_ALERT "scull:goodbye!\n");
}

static int  scull_trim(struct scull_dev *dev)
{
	struct scull_qset *dptr, *next ;
	dptr = dev -> data;

	printk(KERN_ALERT "scull:trim invoked!\n");

	if (dptr == NULL)
		return 0;

	for (dptr = dev -> data; dptr != NULL; dptr = next)
	{
		if (dptr -> qtum_ptr != NULL)
		{
			int i = 0;
			qptr_array *array_ptr = dptr -> qtum_ptr;
			
			if (array_ptr != NULL)
			{
				for (i = 0; i < QTUM_PTR_ARRAY_SIZE; i++)
				{	
					kfree((*array_ptr)[i]);
					(*array_ptr)[i] = NULL;			
				}

				kfree(array_ptr);
			}
		}
		next = dptr -> qset_next;
		kfree(dptr);
		dptr = NULL;
		printk(KERN_ALERT "scull:qset trimed!\n");

	}
	printk(KERN_ALERT "scull:trim finished!\n");

	dev -> data = NULL;	
	return 0;
}

ssize_t scull_read (struct file *filp, char __user *buf, size_t count, loff_t *f_ops)
{
	struct scull_dev *dev 	= filp -> private_data;
	struct scull_qset *dptr = NULL;
	qptr_array *qtum_ptr = NULL;
	
	size_t i = 0;
	ssize_t retval = 0;

	char * read_buf = kmalloc (sizeof(char) * count, GFP_KERNEL);

	if (read_buf == NULL)
	{
		retval = -ENOMEM;
		goto READ_FAULT_EXIT;
	}

	if (count == 0)
	{	
		retval = 0;
		goto READ_FAULT_EXIT;
	}

	if ( !access_ok (VERIFIY_WRITE, (void __user*)buf, count))
	{
		retval = -EFAULT;
		goto READ_FAULT_EXIT;
	}

	if (dev == NULL)
	{
		retval = -EFAULT;
		goto READ_FAULT_EXIT;
	}
	
	dptr = dev -> data;

	if (dptr == NULL )
 	{
		retval = -EFAULT;
		goto READ_FAULT_EXIT;
 	}

	 qtum_ptr = dptr -> qtum_ptr;

	if (qtum_ptr == NULL)
	{
		retval = -EFAULT;
		goto READ_FAULT_EXIT;
	}

	 for (i = 0; i < count; i++)
	 {
	 	qtum_array* rd_array_ptr = NULL;
	 	size_t array_index = (i / QTUM_ARRAY_SIZE)%QTUM_PTR_ARRAY_SIZE;
	 	size_t qtum_index  = i % 4000;

	 	rd_array_ptr = (*qtum_ptr)[array_index];
	 	
	 	if (rd_array_ptr == NULL)
	 		break;

	 	if (array_index != QTUM_PTR_ARRAY_SIZE -1)
	 	{
	 		if (
	 			((*qtum_ptr)[array_index + 1] == NULL) 
	 			&& 
	 			(qtum_index >= dev -> array_wr_ptr)		
	 			)
	 		break;
	 	}
	 	else
	 	{
	 		if (dptr -> qset_next == NULL)
	 		{
	 			if(qtum_index >= dev -> array_wr_ptr)		
	 				break;
	 		}
	 		else
	 		{
	 			if (
	 					(*((dptr -> qset_next) -> qtum_ptr))[0] == NULL
	 					&&
	 					qtum_index >= dev -> array_wr_ptr
	 				)
		 			break;	
	 		}
	 		
	 	}

	 	
	 	read_buf[i] = (*rd_array_ptr)[i%QTUM_ARRAY_SIZE];
	 
	 	if (array_index ==  QTUM_PTR_ARRAY_SIZE -1
	 		&& 
	 		qtum_index == QTUM_ARRAY_SIZE -1
	 		)
	 	{
	 		dptr = dptr -> qset_next;
	 		if (dptr == NULL)
	 			break;

	 		qtum_ptr = dptr -> qtum_ptr;
	 		
	 		if(qtum_ptr == NULL)
	 			break;

	 	}
	 }

	if ( copy_to_user (buf, read_buf, i))
	{
		retval = -EFAULT;
		goto READ_FAULT_EXIT;
	}

	kfree(read_buf);

	return i;

READ_FAULT_EXIT:

	kfree(read_buf);
	return retval;

}

ssize_t scull_write (struct file *filp, const char __user *buf, size_t count, loff_t *f_ops)
{
	struct scull_dev *dev = filp -> private_data;
	struct scull_qset *dptr = NULL;

	int i_vacancy; //point to the next row empty
	int wr_cnt = 0;	
	
	char * write_buf = kmalloc (sizeof(char) * count, GFP_KERNEL);
	
	if (count == 0)
		return 0;
	
	printk(KERN_ALERT "scull:write initial!\n");

	if (write_buf == NULL)
	{
		return -ENOMEM;
	}

	if ( !access_ok (VERIFIY_READ, (void __user*)buf, count))
	{
		kfree(write_buf);
		return -EFAULT;
	}

	if (copy_from_user (write_buf, buf ,count))
	{
		kfree(write_buf);
		return -EFAULT;
	}

	if (dev -> data == NULL) //Initial state, no memory allocated
	{
		dev -> data = kmalloc (sizeof(struct scull_qset), GFP_KERNEL);
		dptr = dev -> data;
		if (dptr == NULL)
		{
			kfree(write_buf);
			return -ENOMEM;
		}	
		dptr -> qset_next = NULL;
		dptr -> qtum_ptr = NULL;
	}
	else
	{
		dptr = dev -> data;		
	}

	if(dptr -> qtum_ptr == NULL)//Initial state
	{
		int i = 0;

		qptr_array *ptr_tmp = NULL;

		dptr -> qtum_ptr = kmalloc (sizeof(qptr_array), GFP_KERNEL);

		if (dptr -> qtum_ptr == NULL)
		{
			kfree(write_buf);
			return -ENOMEM;
		}

		ptr_tmp =  dptr -> qtum_ptr;

		for (i = 0; i< QTUM_PTR_ARRAY_SIZE; i++)
			(*ptr_tmp)[i] = NULL;

	}

	printk(KERN_ALERT "scull:initial finished!\n");

	while (dptr -> qset_next != NULL)
	{
		dptr = dptr -> qset_next; //Find the first potential qset in vacancy
	}

	for (i_vacancy = 0; i_vacancy < QTUM_PTR_ARRAY_SIZE; i_vacancy++ ) //search the qtum array in vacancy
	{

		if ( (*(dptr -> qtum_ptr))[i_vacancy] == NULL)
			break;
	}

	for ( wr_cnt = 0; wr_cnt < count; wr_cnt++)
	{
		qtum_array* wr_array_ptr = NULL;

		if ( dev -> array_wr_ptr == 0) // The qtum to be written is in a new row
		{
			if (i_vacancy == QTUM_PTR_ARRAY_SIZE) //Need a new qset 
			{
				int i = 0;
				qptr_array *ptr_tmp = NULL;

				dptr -> qset_next = kmalloc (sizeof(struct scull_qset), GFP_KERNEL);
				
				if (dptr -> qset_next == NULL)
				{
					kfree(write_buf);
					return -ENOMEM;
				}
				
				
				(dptr -> qset_next)-> qtum_ptr = kmalloc (sizeof( qptr_array), GFP_KERNEL);

				if ((dptr -> qset_next) -> qtum_ptr == NULL)
				{
					kfree(dptr -> qset_next);
					dptr -> qset_next = NULL;//Vital, for next potential write
					kfree(write_buf);
					return -ENOMEM;
				}

				dptr = dptr -> qset_next;
				
				dptr -> qset_next = NULL;

				ptr_tmp = dptr -> qtum_ptr; 

				for (i = 0; i< QTUM_PTR_ARRAY_SIZE; i++)
					(*ptr_tmp)[i] = NULL;



				(*(dptr -> qtum_ptr))[i_vacancy%QTUM_PTR_ARRAY_SIZE] = kmalloc (sizeof(qtum_array), GFP_KERNEL);
				
				if ((*(dptr -> qtum_ptr))[i_vacancy%QTUM_PTR_ARRAY_SIZE] == NULL)
					{
						kfree(write_buf);
						return -ENOMEM;
					}
			}
			else
			{
				(*(dptr -> qtum_ptr))[i_vacancy] = kmalloc (sizeof(qtum_array), GFP_KERNEL);
				
				if ((*(dptr -> qtum_ptr))[i_vacancy] == NULL)
					{
						kfree(write_buf);
						return -ENOMEM;
					}
				
			}

			i_vacancy ++;
			i_vacancy %= QTUM_PTR_ARRAY_SIZE;
			
		}
		
		wr_array_ptr = (*(dptr -> qtum_ptr))[i_vacancy - 1];
		
		(*wr_array_ptr)[(dev -> array_wr_ptr)] = write_buf[wr_cnt];		
		dev -> array_wr_ptr ++;
		dev -> array_wr_ptr %= QTUM_ARRAY_SIZE;

	}

	kfree(write_buf);

	printk(KERN_ALERT "scull:write finished!\n");

	return count;

}		


int scull_release(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;
	dev = container_of (inode -> i_cdev, struct scull_dev, cdev);
	printk(KERN_ALERT "scull:release invoked!\n");

	scull_trim(dev);
	return 0;
}
module_init(scull_init);
module_exit(scull_exit);
