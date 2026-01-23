/**
 * simple.c
 *
 * A simple kernel module. 
 * 
 * To compile, run makefile by entering "make"
 *
 * Operating System Concepts - 10th Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>

/* NEW: print contents of init's PCB */
static void print_init_PCB(void)
{
       struct task_struct *init_pcb = &init_task;
       printk(KERN_INFO "init_task pid = %d\n", init_pcb->pid);
       printk(KERN_INFO "init_task state = %d\n", init_pcb->__state);
       printk(KERN_INFO "init_task flags = %d\n", init_pcb->flags);
       printk(KERN_INFO "init_task runtime prio = %d\n", init_pcb->rt_priority);
       printk(KERN_INFO "init_task process policy = %d\n", init_pcb->policy);
       printk(KERN_INFO "init_task task group id = %d\n", init_pcb->tgid);

}

/* This function is called when the module is loaded. */
static int simple_init(void)
{
       printk(KERN_INFO "Loading Module\n");
       print_init_PCB();

       return 0;
}

/* This function is called when the module is removed. */
static void simple_exit(void) {
	printk(KERN_INFO "Removing Module\n");
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");

