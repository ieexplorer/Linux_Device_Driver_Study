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

#include <linux/seq_file.h>

#define SEQ_NUM 5

static void *scull_seq_start (struct seq_file *s, loff_t *pos)
{
	printk(KERN_ALERT "scull_seq: start!\n");

	return &dev;
}

static void *scull_seq_next (struct seq_file *s, void *y, loff_t *pos)
{
	
	printk(KERN_ALERT "scull_seq: start!\n");

	*pos += 1;
	if (*pos >= SEQ_NUM)
		return NULL;
	
	return &dev;
}

static void scull_seq_stop (struct seq_file *s ,void *v)
{
	return;
}

static int scull_seq_show (struct seq_file *s, void *v)
{
	seq_printf(s, "scull_seq: hello, show you the scull_seq\n");
	return 0;
}

static struct seq_operations scull_seq_ops = 
{
	.start 	= scull_seq_start,
	.next 	= scull_seq_next,
	.stop 	= scull_seq_stop,
	.show 	= scull_seq_show
};

static int scull_proc_open(struct inode *inode, struct file *file)
{
	return seq_open (file, &scull_seq_ops);	
}





struct file_operations scull_proc_ops =
{
	.owner 		= THIS_MODULE,
	.open  		= scull_proc_open,
	.read  		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release
};

