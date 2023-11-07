#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include "kmlab_given.h"
// Include headers as needed ...


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Barran");
MODULE_DESCRIPTION("CPTS360 Lab 4");

#define DEBUG 1

// Global variables as needed ...


// kmlab_init - Called when module is loaded
int __init kmlab_init(void)
{
   #ifdef DEBUG
   pr_info("KMLAB MODULE LOADING\n");
   #endif
   // Insert your code here ...

   pr_info("KMLAB MODULE LOADED\n");
   return 0;
}

// kmlab_exit - Called when module is unloaded
void __exit kmlab_exit(void)
{
   #ifdef DEBUG
   pr_info("KMLAB MODULE UNLOADING\n");
   #endif
   // Insert your code here ...

   pr_info("KMLAB MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(kmlab_init);
module_exit(kmlab_exit);
