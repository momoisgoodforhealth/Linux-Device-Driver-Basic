#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shovan");
MODULE_DESCRIPTION("First Kernel");

/*
extern void proc_remove(struct proc_dir_entry *);
struct proc_dir_entry *proc_create(const char *name, umode_t mode, struct proc_dir_entry *parent, const struct proc_ops *proc_ops);
proc_ops

struct proc_ops {
	unsigned int proc_flags;
	int	(*proc_open)(struct inode *, struct file *);
	ssize_t	(*proc_read)(struct file *, char __user *, size_t, loff_t *);
	ssize_t (*proc_read_iter)(struct kiocb *, struct iov_iter *);
	ssize_t	(*proc_write)(struct file *, const char __user *, size_t, loff_t *);
	/* mandatory unless nonseekable_open() or equivalent is used 
	loff_t	(*proc_lseek)(struct file *, loff_t, int);
	int	(*proc_release)(struct inode *, struct file *);
	__poll_t (*proc_poll)(struct file *, struct poll_table_struct *);
	long	(*proc_ioctl)(struct file *, unsigned int, unsigned long);
#ifdef CONFIG_COMPAT
	long	(*proc_compat_ioctl)(struct file *, unsigned int, unsigned long);
#endif
	int	(*proc_mmap)(struct file *, struct vm_area_struct *);
	unsigned long (*proc_get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
} __randomize_layout;

*/ 

static struct proc_dir_entry *custom_proc_node;

ssize_t shov_read(struct file *, char __user *, size_t, loff_t *) {
    printk("shovan_read\n");
    return 0;
}

struct proc_ops driver_proc_ops = {
    .proc_read = shov_read
};

static int shov_module_init(void) {
    printk("Entry:Shov_driver\n");
    custom_proc_node = proc_create("shov_driver",0,NULL,&driver_proc_ops);
    if (custom_proc_node == NULL) {
        printk("shov_module_init:Error\n");
        return -1;
    }
    return 0;
}

static void shov_module_exit(void) {

    proc_remove(custom_proc_node);
          printk("Exit:Shov_driver!\n");
  
}

module_init(shov_module_init);
module_exit(shov_module_exit);