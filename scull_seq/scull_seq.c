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


static void *scull_seq_start(struct seq_file *s, loff_t *pos)
{}