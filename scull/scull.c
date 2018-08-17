#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "scull.h"
MODULE_LICENSE("Dual BSD/GPL");


unsigned int scull_minor = 0;
unsigned int scull_majot = 0;
unsigned int scull_nr_dev = 1;
dev_t i_dev;

struct file_operations scull_fops = 
{
	.owner 		= THIS_MODULE,
	.llseek 	= scull_llseek,
	.read 		= scull_read,
	.write 		= scull_write,
	.ioctl 		= scull_ioctl,
	.open 		= scull_open,
	.release	= scull_release,
};

struct stcull_dev dev;

static void scull_setup_cdev (struct stcull_dev * dev, int index)
{
	int err = 0;
	int devno = 0;

	devno = MKDEV (scull_major, scull_minor + index);

	cdev_init (&(dev->cdev), &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops 	= &scull_fops; 

	err = cdev_add (&dev->cdev, devno, 1);

	if(err != 0)
		printf(KERN_NOTICE "Error %d adding scull%d\n", err, index);
	return ;
}

static int scull_init(void)
{
	int i = 0;
	int result = 0;


    printk(KERN_ALERT "scull start!\n");
    

    result = alloc_chrdev_region (&i_dev, scull_minor, scull_nr_dev, "scull_module");

    if(result != 0)
    {
    	printk (KERN_WARNING "scull: can't get major %d\n", scull_major)
    	return result;
    }

    scull_setup_cdev (&dev, 0);
    scull_major = MAJOR(i_dev);
    return 0;
}

int scull_open ( struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;
	dev = container_of (inode -> i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;

	if( (filp->f_flags & O_ACCMODE) == O_WRONLY)
	{
		scull_trim(dev);
	}

	return 0;
}

void scull_trim(struct_dev *dev)
{
	struct scull_qset *qset_ptr = NULL;

	if( (qset_ptr = dev -> data) == NULL)
		return ;

	for ( ; ;)
	{
		
		qtum_array_ptr* array_ptr = NULL;
		
		array_ptr = qset_ptr -> qtum_ptr; 
 		
 		for (i = 0; i < QTUM_PTR_ARRAY_SIZE; i++)
		{
			if (array_ptr != NULL)
				kfree(array_ptr)
			array_ptr++;
		}
		kree()

		if(qset_ptr -> qset_next == NULL)
			break;
		else
			qset_ptr =  qset_next;
	}


}

static void scull_exit(void)
{
    printk(KERN_ALERT "goodbye!\n");
}

module_init(scull_init);
module_exit(scull_exit);
