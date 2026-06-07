
/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/of.h>
#include <linux/serdev.h>


MODULE_LICENSE("GPL");

/*  
 *  Prototypes - this would normally go in a .h file
 */


static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Major;		/* Major number assigned to our device driver */
static struct serdev_device *somo_serdev;  /* saved in probe, used in write */
static int Device_Open = 0;	/* Is device open?  
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};


static size_t somo_receive_buf(struct serdev_device *serdev,
                               const u8 *buf,
                               size_t count)
{
    // buf   = bytes just received from hardware
    // count = how many bytes arrived

    // for now just print them
    printk("somo: received %zu bytes: %*ph\n", count, (int)count, buf);

    return count;  // tell serdev how many bytes you consumed
}

static const struct serdev_device_ops somo_ops = {
    .receive_buf  = somo_receive_buf,    // called when UART receives bytes
    .write_wakeup = serdev_device_write_wakeup,  // called when TX buffer has space
};


static const struct of_device_id somo_of_match[] = {
    { .compatible = "sample,ldd" },   // matches your DTS
    {}
};


static int somo_probe(struct serdev_device *serdev)
{
    somo_serdev = serdev;  // save globally so device_write can use it
    serdev_device_set_drvdata(serdev, NULL);
    serdev_device_set_client_ops(serdev, &somo_ops);  // register receive_buf

    // open and configure the UART
    serdev_device_open(serdev);
    serdev_device_set_baudrate(serdev, 9600);
    serdev_device_set_flow_control(serdev, false);

	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
	  return Major;
	}

    printk("somo: probed\n");
    return 0;
}

static void somo_remove(struct serdev_device *serdev)
{
    unregister_chrdev(Major, DEVICE_NAME);
    serdev_device_close(serdev);
    printk("somo: removed\n");
}

static struct serdev_device_driver somo_driver = {
    .probe  = somo_probe,
    .remove = somo_remove,
    .driver = {
        .name           = "somo",
        .of_match_table = somo_of_match,
    },
};


module_serdev_device_driver(somo_driver);




/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/*
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;

	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (*msg_Ptr == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (length && *msg_Ptr) {

		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{  
	
	int to_copy, not_copied, delta;
char kbuf[256];
	to_copy = min(len, sizeof(kbuf));
	not_copied = copy_from_user(kbuf, buff, to_copy);
	if (not_copied)
		return -EFAULT;
	serdev_device_write(somo_serdev, kbuf, to_copy, HZ);
	delta = to_copy - not_copied;
	return delta;
}