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

	if( (filp->f_flags & O_ACCMODE) != O_RDONLY)
	{
		return -EINVAL;
	}

	dev -> array_wr_ptr = 0;
	dev -> array_rd_ptr = 0;

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


ssize_t scull_read (struct file *filp, char __user *buf, size_t count, loff_t *f_ops)
{
	char *dptr_kernel = kmalloc (sizeof(char) * count, GFP_KERNEL);

	if (dptr_kernel == NULL)
		return -ENOMEM;

	memset (dptr_kernel, 'e', sizeof(char) * count);

	if ( copy_to_user (buf, dptr_kernel, count))
	{
		return -EFAULT;
	}
	
	kfree(dptr_kernel);
	
	return count;
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

	if (copy_from_user (write_buf, buf ,count))
	{
		return -EFAULT;
	}

	if (dev -> data == NULL) //Initial state, no memory allocated
	{
		dev -> data = kmalloc (sizeof(struct scull_qset), GFP_KERNEL);
		dptr = dev -> data;
		dptr -> qset_next = NULL;
	}
	else
	{
		dptr = dev -> data;		
	}

	while (dptr -> qset_next != NULL)
		dptr = dptr -> qset_next; //Find the first potential qset in vacancy

	for (i_vacancy = 0; i_vacancy < QTUM_PTR_ARRAY_SIZE; i_vacancy++ ) //search the qtum array in vacancy
	{
		if ( (*(dptr -> qtum_ptr))[i_vacancy] == NULL)
			break;
	}

	i_vacancy %= QTUM_PTR_ARRAY_SIZE;

	if (i_vacancy == 0) // The 0th row is empty 
	{
		(*(dptr -> qtum_ptr))[i_vacancy] = kmalloc (sizeof(qtum_array), GFP_KERNEL);
		i_vacancy = 1;
	}	
	
		
	for ( wr_cnt = 0; wr_cnt < count; wr_cnt++)
	{
		qtum_array_ptr wr_array_ptr = NULL;

		if ( dev -> array_wr_ptr = 0) // The qtum to be written is in a new row
		{
			if (i_vacancy == 0) //Need a new qset 
			{
				dptr -> qset_next = kmalloc (sizeof(struct scull_qset), GFP_KERNEL);
				
				if (dptr -> qset_next = NULL)
					return -ENOMEM;
				
				dptr = dptr -> qset_next;
				
				(*(dptr -> qtum_ptr))[i_vacancy] = kmalloc (sizeof(qtum_array), GFP_KERNEL);
				
				if ((*(dptr -> qtum_ptr))[i_vacancy] = NULL)
					return -ENOMEM;

				i_vacancy ++;
			}
			else
			{
				(*(dptr -> qtum_ptr))[i_vacancy] = kmalloc (sizeof(qtum_array), GFP_KERNEL);
				
				if ((*(dptr -> qtum_ptr))[i_vacancy] = NULL)
					return -ENOMEM;
				
				i_vacancy ++;
				i_vacancy %= QTUM_PTR_ARRAY_SIZE;
			}
		}

		wr_array_ptr = (*(dptr -> qtum_ptr))[i_vacancy - 1]
		
		(*wr_array_ptr)[(dev -> array_wr_ptr) - 1] = *write_buf;		
		dev -> array_wr_ptr ++;
		dev -> array_wr_ptr %= QTUM_ARRAY_SIZE;
		write_buf ++;

	}

	return count;


}		


int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}
module_init(scull_init);
module_exit(scull_exit);
