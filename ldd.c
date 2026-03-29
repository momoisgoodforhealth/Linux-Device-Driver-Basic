#include <linux/init.h>
#include <linux/module.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shovan");
MODULE_DESCRIPTION("First Kernel");


static int shov_module_init(void) {

    printk("Hello World!\n");
    return 0;
}

static void shov_module_exit(void) {
    printk("Bye!\n");
}

module_init(shov_module_init);
module_exit(shov_module_exit);