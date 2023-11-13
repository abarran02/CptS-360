#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/string.h>

#include "kmlab_given.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Barran");
MODULE_DESCRIPTION("CPTS360 Lab 4");

#define DEBUG 1

#define PROCFS_MAX_SIZE 1024
#define PROCFS_DIR "kmlab"
#define PROCFS_FILE "status"

struct process_entry {
    pid_t pid;
    uint32_t cputime;
    struct process_entry* next;
};

/* Global variables */
// holds /proc directory
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_file;

// head for process linked list
struct process_entry *head = NULL;


/* This function is called then the /proc file is read */
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer,
                                size_t buffer_length, loff_t *offset) {
    char km_buffer[256];
    size_t total_bytes = 0;
    struct process_entry *pcurrent;

    pcurrent = head;

    while (pcurrent != NULL) {
        total_bytes += snprintf(km_buffer, sizeof(km_buffer), "PID%d: %d\n",
                                pcurrent->pid, pcurrent->cputime);

        // copy from kernel to user space
        if (copy_to_user(buffer + total_bytes - strnlen(km_buffer, 256), km_buffer, strnlen(km_buffer, 256))) {
            return -EFAULT;
        }

        pcurrent = pcurrent->next;
        pr_info("%p", pcurrent);
    }

    // increment file position and return total bytes written
    *offset += total_bytes;
    return total_bytes;
}

/* This function is called with the /proc file is written. */
static ssize_t procfile_write(struct file *file, const char __user *buff,
                                size_t len, loff_t *off) {
    int status;
    int pid;
    char km_buffer[8];
    struct process_entry *pcurrent;

    // copy new pid from user space
    if (copy_from_user(km_buffer, buff, len)) {
        return -EFAULT;
    }

    // convert new pid string to int
    if ((status = kstrtoint(km_buffer, 10, &pid))) {
        return status;
    }


    if (head == NULL) {
        // empty list, insert new process at beginning
        head = (struct process_entry *)kzalloc(sizeof(struct process_entry), GFP_KERNEL);
        head->pid = pid;
    } else {
        // traverse linked list and insert at end
        pcurrent = head;

        while (pcurrent->next != NULL) {
            pcurrent = pcurrent->next;
        }

        pcurrent->next = (struct process_entry *)kzalloc(sizeof(struct process_entry), GFP_KERNEL);
        pcurrent->next->pid = pid;
    }

    return len;
}


static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};

// kmlab_init - Called when module is loaded
int __init kmlab_init(void) {
    #ifdef DEBUG
    pr_info("KMLAB MODULE LOADING\n");
    #endif

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
    struct process_entry *pcurrent, *temp;
    
    #ifdef DEBUG
    pr_info("KMLAB MODULE UNLOADING\n");
    #endif

    // free memory allocated for linked list
    pcurrent = head;
    while (pcurrent != NULL) {
        temp = pcurrent->next;
        kfree(pcurrent);
        pcurrent = temp;
    }

    proc_remove(proc_dir);
    proc_remove(proc_file);

    pr_info("KMLAB MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(kmlab_init);
module_exit(kmlab_exit);
