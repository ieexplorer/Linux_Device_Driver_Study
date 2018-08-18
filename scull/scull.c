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
int scull_open (struct inode *inode, struct file *filp);
int scull_release(struct inode *inode, struct file *filp);

struct file_operations scull_fops = 
{
	.owner 		= THIS_MODULE,
	// .llseek 	= scull_llseek,
	.read 		= scull_read,
	// .write 		= scull_write,
	// .ioctl 		= scull_ioctl,
	.open 		= scull_open,
	.release	= scull_release,
};

struct scull_dev dev;
struct class *scull_class;

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

	return 0;
}

void scull_trim (struct scull_dev *dev)
{
	struct scull_qset *qset_ptr = NULL;

	if( (qset_ptr = dev -> data) == NULL)
		return ;

	for ( ; ;)
	{
		
		int i = 0;
		qptr_array_ptr* array_ptr = NULL;
		
		array_ptr = qset_ptr -> qtum_ptr; 
 		
 		for (i = 0; i < QTUM_PTR_ARRAY_SIZE; i++)
		{			
			kfree( (*array_ptr)[i] ); //kfree() accepts null pointer
		}
		
		kfree(array_ptr);

		if(qset_ptr -> qset_next == NULL)
			break;
		else
			qset_ptr = qset_ptr -> qset_next;
	}

	dev -> size = 0;
	dev -> data = NULL;
}

static void scull_exit(void)
{
	devno = MKDEV (scull_major, scull_minor);

    scull_trim (&dev);
    
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

int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}
module_init(scull_init);
module_exit(scull_exit);
