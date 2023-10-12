/*
 * procfs.c -  create a "file" in /proc
 */

#include <linux/kernel.h> /* We're doing kernel work */
#include <linux/module.h> /* Specifically, a module */
#include <linux/proc_fs.h> /* Necessary because we use the proc fs */
#include <linux/uaccess.h> /* For copy_from_user */
#include <linux/version.h>
#include <linux/string.h> /* For memset */

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "procbufrev"

/* This structure hold information about the /proc file */
static struct proc_dir_entry *our_proc_file;

/* The buffer used to store character for this module */
static char procfs_buffer[PROCFS_MAX_SIZE] = "HelloWorld!\n";

/* The size of the buffer */
static unsigned long procfs_buffer_size = 0;

/* This function is called then the /proc file is read */
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer,
                             size_t buffer_length, loff_t *offset)
{   
    int len = sizeof(procfs_buffer);
    ssize_t ret = len;

    if (*offset >= len) {
        return 0;
    }
    if (copy_to_user(buffer, procfs_buffer, len)) {
        pr_info("copy_to_user failed\n");
        ret = 0;
    } else {
        pr_info("procfile read /proc/%s\n", PROCFS_NAME);
        *offset += len;
    }

    return ret;
}

/* This function is called with the /proc file is written. */
static ssize_t procfile_write(struct file *file, const char __user *buff,
                              size_t len, loff_t *off)
{
    char temp;

	/* Clear internal buffer */
	memset(&procfs_buffer[0], 0, sizeof(procfs_buffer));
	
    procfs_buffer_size = len;
    if (procfs_buffer_size > PROCFS_MAX_SIZE)
        procfs_buffer_size = PROCFS_MAX_SIZE;

    if (copy_from_user(procfs_buffer, buff, procfs_buffer_size))
        return -EFAULT;

    procfs_buffer[procfs_buffer_size & (PROCFS_MAX_SIZE - 1)] = '\0';

    // reverse procfs_buffer
    for (int i = 0; i < len/2; i++) {
        temp = procfs_buffer[len - i - 2];  // offset of 2 skips newline and null chars
        procfs_buffer[len - i - 2] = procfs_buffer[i];
        procfs_buffer[i] = temp;
    }


    *off += procfs_buffer_size;
    pr_info("procfile write %d bytes %s\n", (int)len, procfs_buffer);

    return procfs_buffer_size;
}

static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};


static int __init procfs2_init(void)
{
    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops);
    if (NULL == our_proc_file) {
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit procfs2_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(procfs2_init);
module_exit(procfs2_exit);

MODULE_LICENSE("GPL");
