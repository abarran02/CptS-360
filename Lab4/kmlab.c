#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>

#include "kmlab_given.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Barran");
MODULE_DESCRIPTION("CPTS360 Lab 4");

#define DEBUG 1

#define PROCFS_MAX_SIZE 1024
#define PROCFS_DIR "kmlab"
#define PROCFS_FILE "status"
#define MAX_STAT_LEN 32

struct process_entry {
    pid_t pid;
    uint32_t cputime;
    struct list_head list;
};

/* Global variables */
static struct proc_dir_entry *proc_dir, *proc_file;

LIST_HEAD(process_list);
static struct timer_list proc_timer;
static struct work_struct work_list;
static DEFINE_SPINLOCK(spn_lock);


static ssize_t procfile_read(struct file *file, char __user *buffer,
                                size_t len, loff_t *offset) {
    struct process_entry *pos;
    char *km_buffer;
    unsigned long flags;
    size_t list_count = 0, size = 0, total_bytes = 0;

    // helps prevent infinite loops, check if data is already read
    if (*offset) {
        return 0;
    }

    // count items in list to allocate memory for printing
    spin_lock_irqsave(&spn_lock, flags);
    list_for_each_entry(pos, &process_list, list) {
        list_count++;
    }
    spin_unlock_irqrestore(&spn_lock, flags);

    size = list_count * MAX_STAT_LEN;
    km_buffer = kmalloc(size, GFP_KERNEL);
    
    // iterate over list and add each PID and CPU time to buffer
    // count total bytes to copy to user later
    spin_lock_irqsave(&spn_lock, flags);
    list_for_each_entry(pos, &process_list, list) {
        total_bytes += scnprintf(km_buffer + total_bytes, size - total_bytes, 
            "PID%d: %u\n", pos->pid, pos->cputime);
        
        if (total_bytes >= size) {
            break;
        } 
    }
    spin_unlock_irqrestore(&spn_lock, flags);

    // cannot copy more data than given buffer
    total_bytes = (total_bytes < len) ? total_bytes : len;
    
    // ensure data is successfully copied
    if (copy_to_user(buffer, km_buffer, total_bytes)) {
        return -EFAULT;
    }
    
    *offset += total_bytes;
    
    return total_bytes;
}

/* This function is called with the /proc file is written. */
static ssize_t procfile_write(struct file *file, const char __user *buffer,
                                size_t len, loff_t *offset) {
    int status;
    int pid;
    char km_buffer[8] = "";
    struct process_entry *pnew;

    // copy new pid from user space
    if (copy_from_user(km_buffer, buffer, len)) {
        return -EFAULT;
    }

    // convert new pid string to int
    if ((status = kstrtoint(km_buffer, 10, &pid))) {
        return status;
    }

    pnew = (struct process_entry *)kzalloc(sizeof(struct process_entry), GFP_KERNEL);
    pnew->pid = pid;
    INIT_LIST_HEAD(&pnew->list);
    list_add_tail(&pnew->list, &process_list);

    return len;
}

static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};

void timer_callback(struct timer_list *t) {
    // setup periodic timer, callback every 5 seconds
    mod_timer(&proc_timer, jiffies + msecs_to_jiffies(5000));
}

// kmlab_init - Called when module is loaded
int __init kmlab_init(void) {
    #ifdef DEBUG
    pr_info("KMLAB MODULE LOADING\n");
    #endif

    INIT_LIST_HEAD(&process_list);

    timer_setup(&proc_timer, timer_callback, 0);

    // create proc directory
    proc_dir = proc_mkdir(PROCFS_DIR, NULL);
    if (proc_dir == NULL) {
        pr_alert("Error: unable to initialize /proc/%s\n", PROCFS_DIR);
        return -ENOMEM;
    }

    // create status file in proc directory
    proc_file = proc_create(PROCFS_FILE, 0666, proc_dir, &proc_file_fops);
    if (proc_file == NULL) {
        pr_alert("Error: unable to create /proc/%s/%s\n", PROCFS_DIR, PROCFS_FILE);
        proc_remove(proc_dir);
        return -ENOMEM;
    }

    pr_info("KMLAB MODULE LOADED\n");
    return 0;
}

// kmlab_exit - Called when module is unloaded
void __exit kmlab_exit(void) {
    struct process_entry *pos, *tmp;

    #ifdef DEBUG
    pr_info("KMLAB MODULE UNLOADING\n");
    #endif

    list_for_each_entry_safe(pos, tmp, &process_list, list) {
        list_del(&pos->list);
        kfree(pos);
    }

    proc_remove(proc_dir);
    proc_remove(proc_file);

    pr_info("KMLAB MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(kmlab_init);
module_exit(kmlab_exit);
