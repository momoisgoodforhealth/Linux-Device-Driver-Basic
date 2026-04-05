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
*/ 

static struct proc_dir_entry *custom_proc_node;



static int shov_module_init(void) {
    printk("Entry:Shov_driver\n");
    custom_proc_node = proc_create("shov_driver",0,NULL,NULL);
    
    return 0;
}

static void shov_module_exit(void) {

    proc_remove(custom_proc_node);
          printk("Exit:Shov_driver!\n");
  
}

module_init(shov_module_init);
module_exit(shov_module_exit);