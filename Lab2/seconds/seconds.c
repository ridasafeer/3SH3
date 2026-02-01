/**
 * seconds.c
 *
 * Kernel module that communicates with /proc file system.
 * 
 * Distribution: Ubuntu 25.10
 * Kernel version: 6.17.0-5-generic
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
//Include jiffies header: To receive the extern global variable jiffies
#include <linux/jiffies.h>

#define BUFFER_SIZE 128 //Kept as buffer size for simplicity

//Updated proc_name macro for use in init and exit functions
#define PROC_NAME "seconds"

//Global variable for jiffies when module is added: Updated only once when proc_init() is called
unsigned long jiffiesAdded;
/**
 * Function prototypes
 */
ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos);

static const struct proc_ops my_proc_ops = {
        .proc_read = proc_read,
};

/* This function is called when the module is loaded. */
int proc_init(void)
{

        // creates the /proc/seconds entry
        // the following function call is a wrapper forthe proc_create() function that initialized module load into kernel
        // proc_create_data() passing NULL as the last argument
        proc_create(PROC_NAME, 0, NULL, &my_proc_ops);
        jiffiesAdded = jiffies; //When loaded, global variable updated at time of function being called to save jiffies at time module added to kernel
        printk(KERN_INFO "/proc/%s created\n", PROC_NAME);

	return 0;
}

/* This function is called when the module is removed. */
void proc_exit(void) {

        // removes the /proc/seconds entry
        remove_proc_entry(PROC_NAME, NULL);

        printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

/**
 * This function is called each time the /proc/seconds is read.
 *
 *
 *
 * 
 * This function is called repeatedly until it returns 0, so
 * there must be logic that ensures it ultimately returns 0
 * once it has collected the data that is to go into the 
 * corresponding /proc file.
 *
 * params:
 *
 * file:
 * buf: buffer in user space
 * count:
 * pos:
 */
ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
        int rv = 0;
        char buffer[BUFFER_SIZE];
        static int completed = 0; //static completed - persists with each function call


        if (completed) { //if completed = 1, reset and return as 0
                completed = 0;
                return 0;
        }

        completed = 1; //if completed = 0, set and continue proc call

        //Calculation for time elapsed using jiffies and HZ rate the timer interrupt runs on
        long now = jiffies;
        long elapsedJiffies = now - timeAdded;

        //Conversion of elapsed jiffies into seconds via HZ machine variable
        long elapsedSeconds = elapsedJiffies/HZ;

        //Formats value into string for later printing to console user space
        rv = sprintf(buffer, "%lu\n", elapsedSeconds);

        //Copies the contents of buffer to userspace usr_buf
        copy_to_user(usr_buf, buffer, rv);

        return rv;
}


/* Macros for registering module entry and exit points. */
module_init( proc_init );
module_exit( proc_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("seconds");
MODULE_AUTHOR("SGG");

